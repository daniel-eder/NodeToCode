"""
Component management for Actor Blueprints.

Add, list, and manage components in Actor-based Blueprints.
Uses the SubobjectDataSubsystem for Blueprint component manipulation.

Usage:
    from scripts.core.component_adder import add_component, list_components

    # Add a static mesh component
    add_component('/Game/BP_Prop', 'StaticMeshComponent', 'MeshRoot')

    # List all components
    components = list_components('/Game/BP_Prop')

Functions return standardized dicts: {success: bool, data: {...}, error: str|None}
"""

import unreal
import re
from typing import List, Dict, Any, Optional


def add_component(
    blueprint_path: str,
    component_class: str,
    component_name: str,
    compile_after: bool = True
) -> Dict[str, Any]:
    """
    Add a component to an Actor Blueprint.

    Args:
        blueprint_path: Path to the Actor Blueprint
        component_class: Component class name or path (e.g., 'StaticMeshComponent',
                        '/Script/Engine.StaticMeshComponent')
        component_name: Name for the component instance
        compile_after: Compile the Blueprint after adding component

    Returns:
        {success, data: {component_name, component_class, blueprint_path}, error}
    """
    if not blueprint_path:
        return _make_error("Blueprint path cannot be empty")

    if not component_class:
        return _make_error("Component class cannot be empty")

    if not component_name:
        return _make_error("Component name cannot be empty")

    try:
        # Load the Blueprint
        if not unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
            return _make_error(f"Blueprint does not exist: {blueprint_path}")

        blueprint = unreal.EditorAssetLibrary.load_asset(blueprint_path)
        if blueprint is None or not isinstance(blueprint, unreal.Blueprint):
            return _make_error(f"Failed to load Blueprint or not a Blueprint: {blueprint_path}")

        # Resolve component class
        comp_class = _resolve_component_class(component_class)
        if comp_class is None:
            return _make_error(f"Could not find component class: {component_class}")

        # Get the SubobjectDataSubsystem
        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        if subsystem is None:
            return _make_error("Could not get SubobjectDataSubsystem")

        # Gather existing subobjects to get the root handle
        handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)

        if not handles or len(handles) == 0:
            return _make_error("Could not gather subobject data from Blueprint")

        # The first handle is typically the root
        root_handle = handles[0]

        # Create parameters for adding the component
        params = unreal.AddNewSubobjectParams(
            parent_handle=root_handle,
            new_class=comp_class,
            blueprint_context=blueprint
        )

        # Add the new component
        new_handle, failure_reason = subsystem.add_new_subobject(params)

        # Check for failure by examining failure_reason
        # In UE5, SubobjectDataHandle doesn't have is_valid() - check failure_reason instead
        failure_text = str(failure_reason) if failure_reason else ""
        if failure_text and failure_text not in ("", "None", "<None>"):
            return _make_error(f"Failed to add component: {failure_text}")

        # Rename the component using the subsystem's rename method
        try:
            subsystem.rename_subobject(new_handle, unreal.Text(component_name))
        except:
            pass  # Renaming failure is not critical

        # Compile if requested
        if compile_after:
            try:
                unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
            except:
                pass

        return _make_success({
            "component_name": component_name,
            "component_class": component_class,
            "resolved_class": str(comp_class.get_name()) if comp_class else "",
            "blueprint_path": blueprint_path,
            "compiled": compile_after
        })

    except Exception as e:
        return _make_error(f"Error adding component: {e}")


def add_components_batch(
    blueprint_path: str,
    components: List[Dict[str, Any]],
    compile_after: bool = True
) -> Dict[str, Any]:
    """
    Add multiple components to a Blueprint.

    Args:
        blueprint_path: Path to the Actor Blueprint
        components: List of component definitions:
            - class: Component class name (required)
            - name: Component instance name (required)
        compile_after: Compile after adding all components

    Returns:
        {success, data: {created_components, failed_components, ...}, error}
    """
    if not blueprint_path:
        return _make_error("Blueprint path cannot be empty")

    if not components:
        return _make_error("Components list cannot be empty")

    created = []
    failed = []

    # Add each component (don't compile until the end)
    for comp_def in components:
        comp_class = comp_def.get('class')
        comp_name = comp_def.get('name')

        if not comp_class:
            failed.append({
                "name": comp_name or "(missing)",
                "error": "Component class is required"
            })
            continue

        if not comp_name:
            failed.append({
                "class": comp_class,
                "error": "Component name is required"
            })
            continue

        result = add_component(blueprint_path, comp_class, comp_name, compile_after=False)

        if result["success"]:
            created.append(result["data"])
        else:
            failed.append({
                "name": comp_name,
                "class": comp_class,
                "error": result["error"]
            })

    # Compile once at the end
    if compile_after and created:
        try:
            blueprint = unreal.EditorAssetLibrary.load_asset(blueprint_path)
            if blueprint:
                unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
        except:
            pass

    return _make_success({
        "created_components": created,
        "created_count": len(created),
        "failed_components": failed,
        "failed_count": len(failed),
        "blueprint_path": blueprint_path,
        "compiled": compile_after
    })


def list_components(blueprint_path: str) -> Dict[str, Any]:
    """
    List all components in a Blueprint.

    Args:
        blueprint_path: Path to the Blueprint

    Returns:
        {success, data: {components: [...], count: int}, error}
    """
    if not blueprint_path:
        return _make_error("Blueprint path cannot be empty")

    try:
        if not unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
            return _make_error(f"Blueprint does not exist: {blueprint_path}")

        blueprint = unreal.EditorAssetLibrary.load_asset(blueprint_path)
        if blueprint is None:
            return _make_error(f"Failed to load Blueprint: {blueprint_path}")

        # Get the SubobjectDataSubsystem
        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        if subsystem is None:
            return _make_error("Could not get SubobjectDataSubsystem")

        # Gather subobjects
        handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)

        components = []
        for handle in handles:
            try:
                data = subsystem.k2_find_subobject_data_from_handle(handle)
                if data:
                    # Parse component info from export_text since get_object() isn't available
                    exported = data.export_text()
                    comp_info = _parse_subobject_data(exported)
                    if comp_info:
                        components.append(comp_info)
            except:
                continue  # Skip any handles that fail to parse

        return _make_success({
            "components": components,
            "count": len(components),
            "blueprint_path": blueprint_path
        })

    except Exception as e:
        return _make_error(f"Error listing components: {e}")


def get_common_component_classes() -> Dict[str, str]:
    """
    Get commonly used component classes.

    Returns:
        Dict mapping simple names to full class paths
    """
    return {
        # Scene components
        'SceneComponent': '/Script/Engine.SceneComponent',
        'BillboardComponent': '/Script/Engine.BillboardComponent',
        'ArrowComponent': '/Script/Engine.ArrowComponent',

        # Mesh components
        'StaticMeshComponent': '/Script/Engine.StaticMeshComponent',
        'SkeletalMeshComponent': '/Script/Engine.SkeletalMeshComponent',
        'InstancedStaticMeshComponent': '/Script/Engine.InstancedStaticMeshComponent',
        'HierarchicalInstancedStaticMeshComponent': '/Script/Engine.HierarchicalInstancedStaticMeshComponent',
        'SplineMeshComponent': '/Script/Engine.SplineMeshComponent',

        # Collision components
        'CapsuleComponent': '/Script/Engine.CapsuleComponent',
        'BoxComponent': '/Script/Engine.BoxComponent',
        'SphereComponent': '/Script/Engine.SphereComponent',

        # Camera and rendering
        'CameraComponent': '/Script/Engine.CameraComponent',
        'SpringArmComponent': '/Script/Engine.SpringArmComponent',
        'SceneCaptureComponent2D': '/Script/Engine.SceneCaptureComponent2D',

        # Lights
        'PointLightComponent': '/Script/Engine.PointLightComponent',
        'SpotLightComponent': '/Script/Engine.SpotLightComponent',
        'DirectionalLightComponent': '/Script/Engine.DirectionalLightComponent',
        'RectLightComponent': '/Script/Engine.RectLightComponent',

        # Audio
        'AudioComponent': '/Script/Engine.AudioComponent',

        # Particles
        'ParticleSystemComponent': '/Script/Engine.ParticleSystemComponent',
        'NiagaraComponent': '/Script/Niagara.NiagaraComponent',

        # Movement
        'CharacterMovementComponent': '/Script/Engine.CharacterMovementComponent',
        'FloatingPawnMovement': '/Script/Engine.FloatingPawnMovement',
        'ProjectileMovementComponent': '/Script/Engine.ProjectileMovementComponent',
        'RotatingMovementComponent': '/Script/Engine.RotatingMovementComponent',

        # UI
        'WidgetComponent': '/Script/UMG.WidgetComponent',

        # Physics
        'PhysicsConstraintComponent': '/Script/Engine.PhysicsConstraintComponent',
        'PhysicsHandleComponent': '/Script/Engine.PhysicsHandleComponent',
        'PhysicsThrusterComponent': '/Script/Engine.PhysicsThrusterComponent',

        # Splines
        'SplineComponent': '/Script/Engine.SplineComponent',

        # Decals
        'DecalComponent': '/Script/Engine.DecalComponent',

        # Text
        'TextRenderComponent': '/Script/Engine.TextRenderComponent',
    }


# ============================================================================
# Private Helper Functions
# ============================================================================

def _parse_subobject_data(exported: str) -> Optional[Dict[str, Any]]:
    """
    Parse component info from SubobjectData.export_text() output.

    UE5 doesn't expose get_object() on SubobjectData, so we parse the
    WeakObjectPtr from export_text() instead.

    Format example:
    (WeakObjectPtr="/Script/Engine.StaticMeshComponent'/Game/BP.BP_C:MeshName'",...)

    Args:
        exported: The export_text() output string

    Returns:
        Dict with name, class, class_path or None if parsing fails
    """
    try:
        # Extract WeakObjectPtr value
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

        # Extract class name from path
        class_name = class_path.split('.')[-1] if '.' in class_path else class_path.split('/')[-1]

        # Extract component name (after last colon)
        if ':' in object_path:
            name = object_path.split(':')[-1]
        else:
            name = object_path.split('.')[-1]

        return {
            "name": name,
            "class": class_name,
            "class_path": class_path
        }
    except:
        return None


def _resolve_component_class(class_identifier: str) -> Optional[unreal.Class]:
    """
    Resolve a component class identifier to a UClass object.

    Args:
        class_identifier: Simple name or full path

    Returns:
        UClass object or None if not found
    """
    # If it's already a full path, try to load it
    if class_identifier.startswith('/'):
        try:
            return unreal.load_object(None, class_identifier)
        except:
            return None

    # Check common class mappings
    common_classes = get_common_component_classes()

    # Try exact match first
    if class_identifier in common_classes:
        class_path = common_classes[class_identifier]
        try:
            return unreal.load_object(None, class_path)
        except:
            pass

    # Try case-insensitive match
    for name, path in common_classes.items():
        if name.lower() == class_identifier.lower():
            try:
                return unreal.load_object(None, path)
            except:
                pass

    # Try as a generic Engine class
    try:
        guess_path = f'/Script/Engine.{class_identifier}'
        return unreal.load_object(None, guess_path)
    except:
        pass

    return None


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
