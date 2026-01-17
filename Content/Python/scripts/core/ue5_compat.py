"""
UE5 Python API Compatibility Utilities.

This module provides workarounds for common UE5 Python API limitations discovered
during development. Use these utilities when the standard Python API doesn't work
as expected.

Key Patterns:
1. EdGraphPinType - Use import_text() instead of set_editor_property()
2. SubobjectDataHandle - Check failure_reason string instead of is_valid()
3. Protected properties - Use export_text() and parse the output
4. FunctionGraphs - Not enumerable; use find_graph() with known names

Usage:
    from scripts.core.ue5_compat import (
        create_pin_type,
        check_subobject_success,
        parse_weak_object_ptr,
    )
"""

import unreal
import re
from typing import Dict, Any, Optional, Tuple


# ============================================================================
# EdGraphPinType Utilities
# ============================================================================

def create_pin_type(
    category: str,
    subcategory: str = "",
    subcategory_object_path: str = "",
    container_type: str = "none",
    is_reference: bool = False
) -> unreal.EdGraphPinType:
    """
    Create an EdGraphPinType using import_text() for UE5 compatibility.

    The standard set_editor_property('pin_category', ...) does NOT work in UE5.
    This function uses import_text() which properly sets all properties.

    Args:
        category: Pin category ('bool', 'byte', 'int', 'int64', 'real', 'string',
                  'name', 'text', 'struct', 'object', 'class', etc.)
        subcategory: Pin subcategory (e.g., 'double' for real numbers)
        subcategory_object_path: Full path to struct/class for object types
        container_type: 'none', 'array', 'set', or 'map'
        is_reference: Whether this is a reference type

    Returns:
        Configured EdGraphPinType

    Example:
        # Float variable
        pin = create_pin_type('real', 'double')

        # Actor reference
        pin = create_pin_type('object', '', '/Script/Engine.Actor')

        # Array of vectors
        pin = create_pin_type('struct', '', '/Script/CoreUObject.Vector', 'array')
    """
    parts = [f'PinCategory="{category}"']

    if subcategory:
        parts.append(f'PinSubCategory="{subcategory}"')

    if subcategory_object_path:
        parts.append(f'PinSubCategoryObject={subcategory_object_path}')

    container_map = {
        'none': 'None',
        'array': 'Array',
        'set': 'Set',
        'map': 'Map'
    }
    container_value = container_map.get(container_type.lower(), 'None')
    parts.append(f'ContainerType={container_value}')

    if is_reference:
        parts.append('bIsReference=True')

    import_string = f"({','.join(parts)})"

    pin_type = unreal.EdGraphPinType()
    pin_type.import_text(import_string)

    return pin_type


def create_float_pin_type() -> unreal.EdGraphPinType:
    """Create a pin type for float/double values (UE5 uses real/double)."""
    return create_pin_type('real', 'double')


def create_int_pin_type() -> unreal.EdGraphPinType:
    """Create a pin type for integer values."""
    return create_pin_type('int')


def create_bool_pin_type() -> unreal.EdGraphPinType:
    """Create a pin type for boolean values."""
    return create_pin_type('bool')


def create_string_pin_type() -> unreal.EdGraphPinType:
    """Create a pin type for string values."""
    return create_pin_type('string')


def create_object_pin_type(class_path: str) -> unreal.EdGraphPinType:
    """
    Create a pin type for object references.

    Args:
        class_path: Full class path (e.g., '/Script/Engine.Actor')
    """
    return create_pin_type('object', '', class_path)


def create_struct_pin_type(struct_path: str) -> unreal.EdGraphPinType:
    """
    Create a pin type for struct values.

    Args:
        struct_path: Full struct path (e.g., '/Script/CoreUObject.Vector')
    """
    return create_pin_type('struct', '', struct_path)


def create_array_pin_type(element_category: str, element_subcategory: str = "",
                          element_object_path: str = "") -> unreal.EdGraphPinType:
    """
    Create a pin type for array containers.

    Args:
        element_category: Category of array elements
        element_subcategory: Subcategory of array elements
        element_object_path: Object path for struct/object elements
    """
    return create_pin_type(element_category, element_subcategory,
                           element_object_path, 'array')


# ============================================================================
# SubobjectData Utilities
# ============================================================================

def check_subobject_success(failure_reason: Any) -> Tuple[bool, str]:
    """
    Check if a SubobjectDataSubsystem operation succeeded.

    SubobjectDataHandle doesn't have is_valid() in UE5 Python.
    Instead, check the failure_reason return value.

    Args:
        failure_reason: The second return value from add_new_subobject(), etc.

    Returns:
        Tuple of (success: bool, error_message: str)

    Example:
        new_handle, failure_reason = subsystem.add_new_subobject(params)
        success, error = check_subobject_success(failure_reason)
        if not success:
            print(f"Failed: {error}")
    """
    if failure_reason is None:
        return (True, "")

    failure_str = str(failure_reason)

    # Empty or None-like strings indicate success
    if failure_str in ("", "None", "<None>", "null"):
        return (True, "")

    return (False, failure_str)


def parse_weak_object_ptr(exported: str) -> Optional[Dict[str, str]]:
    """
    Parse component/object info from SubobjectData.export_text() output.

    UE5 doesn't expose get_object() or get_display_name() on SubobjectData.
    Use export_text() and parse the WeakObjectPtr instead.

    Args:
        exported: The export_text() output string

    Returns:
        Dict with 'name', 'class', 'class_path', 'object_path' or None

    Example:
        data = subsystem.k2_find_subobject_data_from_handle(handle)
        info = parse_weak_object_ptr(data.export_text())
        if info:
            print(f"Component: {info['name']} ({info['class']})")
    """
    try:
        # Extract WeakObjectPtr value
        # Format: WeakObjectPtr="/Script/Engine.Class'/Path/To/Object.Object:Name'"
        match = re.search(r'WeakObjectPtr="([^"]+)"', exported)
        if not match:
            return None

        weak_ptr = match.group(1)

        # Parse: /Script/Engine.ClassName'/Game/Path.Object:ComponentName'
        class_match = re.match(r"(/[^']+)'([^']+)'", weak_ptr)
        if not class_match:
            return None

        class_path = class_match.group(1)
        object_path = class_match.group(2)

        # Extract class name
        class_name = class_path.split('.')[-1] if '.' in class_path else class_path.split('/')[-1]

        # Extract component/object name (after last colon)
        if ':' in object_path:
            name = object_path.split(':')[-1]
        else:
            name = object_path.split('.')[-1]

        return {
            "name": name,
            "class": class_name,
            "class_path": class_path,
            "object_path": object_path
        }
    except:
        return None


# ============================================================================
# Blueprint Property Access
# ============================================================================

def get_protected_property_via_export(obj: Any, property_name: str) -> Optional[str]:
    """
    Attempt to get a protected property value via export_text().

    Some Blueprint properties (like FunctionGraphs) are protected and
    can't be accessed via get_editor_property(). This function tries
    to extract the value from the export_text() output.

    Args:
        obj: The Unreal object
        property_name: The property to look for

    Returns:
        The property value as string, or None if not found

    Note: This is a best-effort workaround and may not work for all properties.
    """
    try:
        exported = obj.export_text()
        # Look for PropertyName=(...) or PropertyName="..."
        pattern = rf'{property_name}=([^\s,\)]+|\([^\)]*\)|"[^"]*")'
        match = re.search(pattern, exported)
        if match:
            return match.group(1)
    except:
        pass
    return None


# ============================================================================
# Type Resolution Helpers
# ============================================================================

# Common type mappings for Blueprint variables
PRIMITIVE_TYPES = {
    'bool': ('bool', ''),
    'boolean': ('bool', ''),
    'byte': ('byte', ''),
    'uint8': ('byte', ''),
    'int': ('int', ''),
    'int32': ('int', ''),
    'integer': ('int', ''),
    'int64': ('int64', ''),
    'float': ('real', 'double'),  # UE5 uses real/double
    'double': ('real', 'double'),
    'real': ('real', 'double'),
    'string': ('string', ''),
    'fstring': ('string', ''),
    'name': ('name', ''),
    'fname': ('name', ''),
    'text': ('text', ''),
    'ftext': ('text', ''),
}

STRUCT_TYPES = {
    'vector': '/Script/CoreUObject.Vector',
    'fvector': '/Script/CoreUObject.Vector',
    'vector2d': '/Script/CoreUObject.Vector2D',
    'vector4': '/Script/CoreUObject.Vector4',
    'rotator': '/Script/CoreUObject.Rotator',
    'frotator': '/Script/CoreUObject.Rotator',
    'transform': '/Script/CoreUObject.Transform',
    'ftransform': '/Script/CoreUObject.Transform',
    'quat': '/Script/CoreUObject.Quat',
    'color': '/Script/CoreUObject.Color',
    'fcolor': '/Script/CoreUObject.Color',
    'linearcolor': '/Script/CoreUObject.LinearColor',
    'timespan': '/Script/CoreUObject.Timespan',
    'datetime': '/Script/CoreUObject.DateTime',
    'gameplaytag': '/Script/GameplayTags.GameplayTag',
    'gameplaytagcontainer': '/Script/GameplayTags.GameplayTagContainer',
}

OBJECT_TYPES = {
    'object': '/Script/CoreUObject.Object',
    'actor': '/Script/Engine.Actor',
    'pawn': '/Script/Engine.Pawn',
    'character': '/Script/Engine.Character',
    'actorcomponent': '/Script/Engine.ActorComponent',
    'scenecomponent': '/Script/Engine.SceneComponent',
    'staticmesh': '/Script/Engine.StaticMesh',
    'skeletalmesh': '/Script/Engine.SkeletalMesh',
    'material': '/Script/Engine.MaterialInterface',
    'texture': '/Script/Engine.Texture2D',
    'soundcue': '/Script/Engine.SoundCue',
    'soundwave': '/Script/Engine.SoundWave',
}


def resolve_type_to_pin_info(type_identifier: str) -> Optional[Dict[str, str]]:
    """
    Resolve a type name to pin category/subcategory info.

    Args:
        type_identifier: Simple type name or full path

    Returns:
        Dict with 'category', 'subcategory', 'object_path' or None

    Example:
        info = resolve_type_to_pin_info('float')
        # Returns: {'category': 'real', 'subcategory': 'double', 'object_path': ''}

        info = resolve_type_to_pin_info('vector')
        # Returns: {'category': 'struct', 'subcategory': '', 'object_path': '/Script/CoreUObject.Vector'}
    """
    type_lower = type_identifier.lower().strip()

    # Check primitives
    if type_lower in PRIMITIVE_TYPES:
        cat, subcat = PRIMITIVE_TYPES[type_lower]
        return {'category': cat, 'subcategory': subcat, 'object_path': ''}

    # Check structs
    if type_lower in STRUCT_TYPES:
        return {'category': 'struct', 'subcategory': '', 'object_path': STRUCT_TYPES[type_lower]}

    # Check objects
    if type_lower in OBJECT_TYPES:
        return {'category': 'object', 'subcategory': '', 'object_path': OBJECT_TYPES[type_lower]}

    # Try as full path
    if type_identifier.startswith('/'):
        try:
            obj = unreal.load_object(None, type_identifier)
            if obj:
                if isinstance(obj, unreal.ScriptStruct):
                    return {'category': 'struct', 'subcategory': '', 'object_path': type_identifier}
                else:
                    return {'category': 'object', 'subcategory': '', 'object_path': type_identifier}
        except:
            pass

    return None


def create_pin_type_from_name(type_name: str, container: str = 'none') -> Optional[unreal.EdGraphPinType]:
    """
    Create an EdGraphPinType from a simple type name.

    Convenience function that combines resolve_type_to_pin_info and create_pin_type.

    Args:
        type_name: Simple type name (e.g., 'float', 'vector', 'actor')
        container: Container type ('none', 'array', 'set', 'map')

    Returns:
        EdGraphPinType or None if type not recognized

    Example:
        pin = create_pin_type_from_name('float')
        pin = create_pin_type_from_name('actor', 'array')  # Array of actors
    """
    info = resolve_type_to_pin_info(type_name)
    if info:
        return create_pin_type(
            info['category'],
            info['subcategory'],
            info['object_path'],
            container
        )
    return None
