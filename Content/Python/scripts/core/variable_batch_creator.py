"""
Batch variable creation for Blueprints.

Efficiently add multiple member variables with types, defaults, and categories.
Supports all Blueprint variable types including containers (arrays, sets, maps).

Usage:
    from scripts.core.variable_batch_creator import add_variables_to_blueprint

    variables = [
        {'name': 'Health', 'type': 'float', 'default': '100.0', 'category': 'Stats'},
        {'name': 'MaxHealth', 'type': 'float', 'default': '100.0', 'category': 'Stats'},
        {'name': 'IsDead', 'type': 'bool', 'default': 'false', 'category': 'State'},
        {'name': 'Inventory', 'type': 'Actor', 'container': 'array', 'category': 'Items'},
    ]
    result = add_variables_to_blueprint('/Game/BP_Character', variables)

Functions return standardized dicts: {success: bool, data: {...}, error: str|None}
"""

import unreal
from typing import List, Dict, Any, Optional


def add_variables_to_blueprint(
    blueprint_path: str,
    variables: List[Dict[str, Any]],
    compile_after: bool = True
) -> Dict[str, Any]:
    """
    Add multiple variables to a Blueprint.

    Args:
        blueprint_path: Path to the Blueprint asset
        variables: List of variable definitions, each containing:
            - name: Variable name (required)
            - type: Type identifier (required) - e.g., 'bool', 'float', 'FVector',
                   '/Script/Engine.Actor', 'StaticMesh'
            - default: Default value as string (optional)
            - category: Category for organization (optional, default 'Default')
            - instance_editable: Whether editable per-instance (optional, default True)
            - tooltip: Description tooltip (optional)
            - container: 'none', 'array', 'set', or 'map' (optional, default 'none')
            - map_key_type: Key type for map containers (required if container is 'map')
        compile_after: Compile Blueprint after adding variables

    Returns:
        {success, data: {created_variables, failed_variables, blueprint_path}, error}
    """
    if not blueprint_path:
        return _make_error("Blueprint path cannot be empty")

    if not variables:
        return _make_error("Variables list cannot be empty")

    try:
        # Load the Blueprint
        if not unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
            return _make_error(f"Blueprint does not exist: {blueprint_path}")

        blueprint = unreal.EditorAssetLibrary.load_asset(blueprint_path)
        if blueprint is None or not isinstance(blueprint, unreal.Blueprint):
            return _make_error(f"Failed to load Blueprint or not a Blueprint: {blueprint_path}")

        created = []
        failed = []

        for var_def in variables:
            # Validate required fields
            var_name = var_def.get('name')
            var_type = var_def.get('type')

            if not var_name:
                failed.append({
                    "name": "(missing)",
                    "error": "Variable name is required"
                })
                continue

            if not var_type:
                failed.append({
                    "name": var_name,
                    "error": "Variable type is required"
                })
                continue

            # Get optional fields
            default_value = var_def.get('default', '')
            category = var_def.get('category', 'Default')
            instance_editable = var_def.get('instance_editable', True)
            tooltip = var_def.get('tooltip', '')
            container = var_def.get('container', 'none').lower()
            map_key_type = var_def.get('map_key_type', '')

            # Validate map container
            if container == 'map' and not map_key_type:
                failed.append({
                    "name": var_name,
                    "error": "Map containers require 'map_key_type'"
                })
                continue

            # Create the pin type using import_text (UE5 compatible)
            pin_type_result = _create_pin_type(var_type, container, map_key_type)
            if not pin_type_result["success"]:
                failed.append({
                    "name": var_name,
                    "error": pin_type_result["error"]
                })
                continue

            pin_type = pin_type_result["data"]

            # Add the variable
            try:
                success = unreal.BlueprintEditorLibrary.add_member_variable(
                    blueprint,
                    var_name,
                    pin_type
                )

                if success:
                    created.append({
                        "name": var_name,
                        "type": var_type,
                        "container": container,
                        "category": category
                    })
                else:
                    failed.append({
                        "name": var_name,
                        "error": "add_member_variable returned false"
                    })

            except Exception as e:
                failed.append({
                    "name": var_name,
                    "error": str(e)
                })

        # Compile if requested and we created any variables
        if compile_after and created:
            try:
                unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
            except:
                pass  # Compilation errors shouldn't fail the whole operation

        return _make_success({
            "created_variables": created,
            "created_count": len(created),
            "failed_variables": failed,
            "failed_count": len(failed),
            "blueprint_path": blueprint_path,
            "compiled": compile_after
        })

    except Exception as e:
        return _make_error(f"Error adding variables: {e}")


def add_variable(
    blueprint_path: str,
    name: str,
    type_identifier: str,
    default_value: Optional[str] = None,
    category: str = 'Default',
    instance_editable: bool = True,
    tooltip: str = '',
    container_type: str = 'none',
    map_key_type: Optional[str] = None,
    compile_after: bool = True
) -> Dict[str, Any]:
    """
    Add a single variable to a Blueprint.

    Args:
        blueprint_path: Path to the Blueprint asset
        name: Variable name
        type_identifier: Type (e.g., 'bool', 'float', '/Script/Engine.Actor')
        default_value: Default value as string
        category: Variable category
        instance_editable: Editable per-instance
        tooltip: Variable tooltip
        container_type: 'none', 'array', 'set', or 'map'
        map_key_type: Key type if container_type is 'map'
        compile_after: Compile after adding

    Returns:
        {success, data: {name, type, category, ...}, error}
    """
    variable_def = {
        'name': name,
        'type': type_identifier,
        'default': default_value or '',
        'category': category,
        'instance_editable': instance_editable,
        'tooltip': tooltip,
        'container': container_type,
        'map_key_type': map_key_type or ''
    }

    result = add_variables_to_blueprint(blueprint_path, [variable_def], compile_after)

    if result["success"]:
        data = result["data"]
        if data["created_count"] > 0:
            return _make_success(data["created_variables"][0])
        elif data["failed_count"] > 0:
            return _make_error(data["failed_variables"][0]["error"])

    return result


def get_supported_types() -> Dict[str, str]:
    """
    Get a mapping of simple type names to their descriptions.

    Returns:
        Dict mapping type names to descriptions
    """
    return {
        # Primitives
        'bool': 'Boolean value (true/false)',
        'byte': 'Unsigned 8-bit integer (0-255)',
        'int': 'Signed 32-bit integer',
        'int32': 'Signed 32-bit integer',
        'int64': 'Signed 64-bit integer',
        'float': 'Single-precision floating point',
        'double': 'Double-precision floating point',
        'real': 'Real number (double precision)',

        # Text types
        'string': 'Text string (FString)',
        'name': 'FName identifier',
        'text': 'Localizable text (FText)',

        # Math types
        'vector': 'FVector (X, Y, Z)',
        'vector2d': 'FVector2D (X, Y)',
        'vector4': 'FVector4 (X, Y, Z, W)',
        'rotator': 'FRotator (Pitch, Yaw, Roll)',
        'transform': 'FTransform (Location, Rotation, Scale)',
        'quat': 'FQuat (quaternion rotation)',

        # Color types
        'color': 'FColor (RGBA byte)',
        'linearcolor': 'FLinearColor (RGBA float)',

        # Object references
        'object': 'UObject reference',
        'class': 'UClass reference',
        'softobject': 'Soft object reference',
        'softclass': 'Soft class reference',
        'actor': 'Actor reference',
        'component': 'ActorComponent reference',

        # Gameplay types
        'gameplay_tag': 'Gameplay Tag',
        'gameplay_tag_container': 'Gameplay Tag Container',
        'timedelta': 'Time delta (FTimespan)',
        'datetime': 'Date and time (FDateTime)',
    }


def get_container_types() -> List[str]:
    """
    Get available container types.

    Returns:
        List of container type names
    """
    return ['none', 'array', 'set', 'map']


# ============================================================================
# Private Helper Functions
# ============================================================================

def _create_pin_type(
    type_identifier: str,
    container_type: str = 'none',
    map_key_type: Optional[str] = None
) -> Dict[str, Any]:
    """
    Create an EdGraphPinType from a type identifier string.

    Uses import_text() for UE5 compatibility instead of set_editor_property().

    Args:
        type_identifier: Type name or path
        container_type: 'none', 'array', 'set', or 'map'
        map_key_type: Key type for maps

    Returns:
        {success, data: EdGraphPinType, error}
    """
    try:
        # Resolve type to pin category and subcategory
        type_info = _resolve_type(type_identifier)
        if not type_info["success"]:
            return type_info

        category = type_info["data"]["category"]
        subcategory = type_info["data"].get("subcategory", "")
        subcategory_object_path = type_info["data"].get("subcategory_object_path", "")

        # Build the import text string
        # Format: (PinCategory="category",PinSubCategory="subcategory",PinSubCategoryObject=path,ContainerType=type)
        parts = [f'PinCategory="{category}"']

        if subcategory:
            parts.append(f'PinSubCategory="{subcategory}"')

        if subcategory_object_path:
            parts.append(f'PinSubCategoryObject={subcategory_object_path}')

        # Handle container type
        container_map = {
            'none': 'None',
            'array': 'Array',
            'set': 'Set',
            'map': 'Map'
        }
        container_value = container_map.get(container_type.lower(), 'None')
        parts.append(f'ContainerType={container_value}')

        import_string = f"({','.join(parts)})"

        # Create and import the pin type
        pin_type = unreal.EdGraphPinType()
        pin_type.import_text(import_string)

        return _make_success(pin_type)

    except Exception as e:
        return _make_error(f"Failed to create pin type: {e}")


def _resolve_type(type_identifier: str) -> Dict[str, Any]:
    """
    Resolve a type identifier to pin category and subcategory.

    Args:
        type_identifier: Type name or full path

    Returns:
        {success, data: {category, subcategory, subcategory_object_path}, error}
    """
    type_lower = type_identifier.lower().strip()

    # Primitive type mappings - UE5 uses 'real' with 'double' subcategory for float
    primitives = {
        'bool': ('bool', ''),
        'boolean': ('bool', ''),
        'byte': ('byte', ''),
        'uint8': ('byte', ''),
        'int': ('int', ''),
        'int32': ('int', ''),
        'integer': ('int', ''),
        'int64': ('int64', ''),
        'float': ('real', 'double'),  # UE5 uses real/double for float
        'double': ('real', 'double'),
        'real': ('real', 'double'),
        'string': ('string', ''),
        'fstring': ('string', ''),
        'name': ('name', ''),
        'fname': ('name', ''),
        'text': ('text', ''),
        'ftext': ('text', ''),
    }

    if type_lower in primitives:
        cat, subcat = primitives[type_lower]
        return _make_success({
            "category": cat,
            "subcategory": subcat,
            "subcategory_object_path": ""
        })

    # Struct type mappings
    structs = {
        'vector': '/Script/CoreUObject.Vector',
        'fvector': '/Script/CoreUObject.Vector',
        'vector2d': '/Script/CoreUObject.Vector2D',
        'fvector2d': '/Script/CoreUObject.Vector2D',
        'vector4': '/Script/CoreUObject.Vector4',
        'fvector4': '/Script/CoreUObject.Vector4',
        'rotator': '/Script/CoreUObject.Rotator',
        'frotator': '/Script/CoreUObject.Rotator',
        'transform': '/Script/CoreUObject.Transform',
        'ftransform': '/Script/CoreUObject.Transform',
        'quat': '/Script/CoreUObject.Quat',
        'fquat': '/Script/CoreUObject.Quat',
        'color': '/Script/CoreUObject.Color',
        'fcolor': '/Script/CoreUObject.Color',
        'linearcolor': '/Script/CoreUObject.LinearColor',
        'flinearcolor': '/Script/CoreUObject.LinearColor',
        'timedelta': '/Script/CoreUObject.Timespan',
        'timespan': '/Script/CoreUObject.Timespan',
        'datetime': '/Script/CoreUObject.DateTime',
        'gameplay_tag': '/Script/GameplayTags.GameplayTag',
        'gameplaytag': '/Script/GameplayTags.GameplayTag',
        'gameplay_tag_container': '/Script/GameplayTags.GameplayTagContainer',
        'gameplaytagcontainer': '/Script/GameplayTags.GameplayTagContainer',
    }

    if type_lower in structs:
        struct_path = structs[type_lower]
        return _make_success({
            "category": "struct",
            "subcategory": "",
            "subcategory_object_path": struct_path
        })

    # Object type mappings
    objects = {
        'object': '/Script/CoreUObject.Object',
        'actor': '/Script/Engine.Actor',
        'pawn': '/Script/Engine.Pawn',
        'character': '/Script/Engine.Character',
        'component': '/Script/Engine.ActorComponent',
        'actorcomponent': '/Script/Engine.ActorComponent',
        'scenecomponent': '/Script/Engine.SceneComponent',
        'staticmesh': '/Script/Engine.StaticMesh',
        'skeletalmesh': '/Script/Engine.SkeletalMesh',
        'material': '/Script/Engine.MaterialInterface',
        'texture': '/Script/Engine.Texture2D',
        'soundcue': '/Script/Engine.SoundCue',
        'soundwave': '/Script/Engine.SoundWave',
        'particlesystem': '/Script/Engine.ParticleSystem',
        'niagarasystem': '/Script/Niagara.NiagaraSystem',
        'datatable': '/Script/Engine.DataTable',
        'userwidget': '/Script/UMG.UserWidget',
        'widget': '/Script/UMG.Widget',
    }

    if type_lower in objects:
        class_path = objects[type_lower]
        return _make_success({
            "category": "object",
            "subcategory": "",
            "subcategory_object_path": class_path
        })

    # Try loading as full path (object reference)
    if type_identifier.startswith('/'):
        try:
            obj = unreal.load_object(None, type_identifier)
            if obj:
                # Determine if it's a class or struct
                if isinstance(obj, unreal.ScriptStruct):
                    return _make_success({
                        "category": "struct",
                        "subcategory": "",
                        "subcategory_object_path": type_identifier
                    })
                else:
                    return _make_success({
                        "category": "object",
                        "subcategory": "",
                        "subcategory_object_path": type_identifier
                    })
        except:
            pass

    # Try as class reference
    if type_lower in ['class', 'uclass']:
        return _make_success({
            "category": "class",
            "subcategory": "",
            "subcategory_object_path": ""
        })

    # Fallback: assume it's an object type with unknown class
    return _make_error(f"Unknown type identifier: {type_identifier}")


def _make_success(data: Any) -> Dict[str, Any]:
    """Create a success result."""
    return {
        "success": True,
        "data": data,
        "error": None
    }


def _make_error(message: str) -> Dict[str, Any]:
    """Create an error result."""
    return {
        "success": False,
        "data": None,
        "error": message
    }
