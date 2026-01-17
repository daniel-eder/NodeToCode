"""
Asset editor opener for Unreal Engine.

Open assets in their appropriate editors (Blueprint Editor, UMG Designer, etc.)
programmatically. Supports batch opening and various asset types.

Usage:
    from scripts.ui.asset_editor_opener import open_asset, open_assets_batch

    # Open a single asset
    result = open_asset('/Game/UI/Menus/WBP_MainMenu')

    # Open multiple assets
    result = open_assets_batch([
        '/Game/UI/Menus/WBP_MainMenu',
        '/Game/UI/Menus/WBP_PauseMenu'
    ])

Functions return standardized dicts: {success: bool, data: {...}, error: str|None}
"""

import unreal
from typing import List, Dict, Any, Optional


def open_asset(asset_path: str) -> Dict[str, Any]:
    """
    Open an asset in its appropriate editor.

    Automatically detects the asset type and opens it in the correct editor
    (Blueprint Editor for Blueprints, UMG Designer for Widgets, etc.)

    Args:
        asset_path: Path to the asset (e.g., '/Game/UI/Menus/WBP_MainMenu')

    Returns:
        {success, data: {asset_path, asset_name, asset_type, opened}, error}

    Example:
        result = open_asset('/Game/Blueprints/BP_Player')
    """
    if not asset_path:
        return _make_error("Asset path cannot be empty")

    try:
        # Check if asset exists
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            return _make_error(f"Asset does not exist: {asset_path}")

        # Load the asset
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset is None:
            return _make_error(f"Failed to load asset: {asset_path}")

        # Get the asset editor subsystem
        editor_subsystem = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
        if editor_subsystem is None:
            return _make_error("Could not get AssetEditorSubsystem")

        # Open the asset in its editor
        opened = editor_subsystem.open_editor_for_assets([asset])

        asset_name = asset_path.split('/')[-1]
        asset_type = str(asset.get_class().get_name())

        return _make_success({
            "asset_path": asset_path,
            "asset_name": asset_name,
            "asset_type": asset_type,
            "opened": opened
        })

    except Exception as e:
        return _make_error(f"Error opening asset: {e}")


def open_assets_batch(asset_paths: List[str]) -> Dict[str, Any]:
    """
    Open multiple assets in their appropriate editors.

    Opens all assets at once, which is more efficient than opening them
    one by one. Each asset opens in its appropriate editor type.

    Args:
        asset_paths: List of asset paths to open

    Returns:
        {success, data: {opened: [...], failed: [...], total_opened, total_failed}, error}

    Example:
        result = open_assets_batch([
            '/Game/UI/Menus/WBP_MainMenu',
            '/Game/UI/Menus/WBP_PauseMenu',
            '/Game/Blueprints/BP_Player'
        ])
    """
    if not asset_paths:
        return _make_error("Asset paths list cannot be empty")

    if not isinstance(asset_paths, list):
        return _make_error("Asset paths must be a list")

    try:
        editor_subsystem = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
        if editor_subsystem is None:
            return _make_error("Could not get AssetEditorSubsystem")

        opened = []
        failed = []
        assets_to_open = []

        # Load all assets first
        for asset_path in asset_paths:
            if not asset_path:
                failed.append({
                    "path": asset_path,
                    "error": "Empty path"
                })
                continue

            if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                failed.append({
                    "path": asset_path,
                    "error": "Asset does not exist"
                })
                continue

            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if asset is None:
                failed.append({
                    "path": asset_path,
                    "error": "Failed to load asset"
                })
                continue

            assets_to_open.append(asset)
            opened.append({
                "path": asset_path,
                "name": asset_path.split('/')[-1],
                "type": str(asset.get_class().get_name())
            })

        # Open all valid assets at once
        if assets_to_open:
            editor_subsystem.open_editor_for_assets(assets_to_open)

        return _make_success({
            "opened": opened,
            "failed": failed,
            "total_opened": len(opened),
            "total_failed": len(failed)
        })

    except Exception as e:
        return _make_error(f"Error opening assets: {e}")


def close_asset_editors(asset_path: str) -> Dict[str, Any]:
    """
    Close all editors for a specific asset.

    Args:
        asset_path: Path to the asset whose editors should be closed

    Returns:
        {success, data: {asset_path, closed}, error}

    Example:
        result = close_asset_editors('/Game/Blueprints/BP_Player')
    """
    if not asset_path:
        return _make_error("Asset path cannot be empty")

    try:
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            return _make_error(f"Asset does not exist: {asset_path}")

        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset is None:
            return _make_error(f"Failed to load asset: {asset_path}")

        editor_subsystem = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
        if editor_subsystem is None:
            return _make_error("Could not get AssetEditorSubsystem")

        closed = editor_subsystem.close_all_editors_for_asset(asset)

        return _make_success({
            "asset_path": asset_path,
            "closed": closed
        })

    except Exception as e:
        return _make_error(f"Error closing asset editors: {e}")


def close_all_asset_editors() -> Dict[str, Any]:
    """
    Close all open asset editors.

    Note: This function is not fully supported in UE5 Python API.
    The AssetEditorSubsystem.get_all_edited_assets() method is not exposed.
    Use close_asset_editors(path) to close specific asset editors instead.

    Returns:
        {success, data: {message}, error}

    Example:
        result = close_all_asset_editors()
    """
    # UE5 Python API does not expose get_all_edited_assets() or close_all_asset_editors()
    # This is a known limitation of the UE5 Python bindings
    return _make_success({
        "message": "close_all_asset_editors is not supported in UE5 Python API. Use close_asset_editors(path) for specific assets.",
        "supported": False
    })


def is_asset_editor_open(asset_path: str) -> Dict[str, Any]:
    """
    Check if an asset has any open editors.

    Note: This function is not fully supported in UE5 Python API.
    Neither find_editors_for_asset() nor get_all_edited_assets() are exposed.

    Args:
        asset_path: Path to the asset to check

    Returns:
        {success, data: {asset_path, is_open, supported}, error}

    Example:
        result = is_asset_editor_open('/Game/Blueprints/BP_Player')
    """
    if not asset_path:
        return _make_error("Asset path cannot be empty")

    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return _make_error(f"Asset does not exist: {asset_path}")

    # UE5 Python API does not expose find_editors_for_asset() or get_all_edited_assets()
    # We cannot reliably determine if an asset editor is open
    return _make_success({
        "asset_path": asset_path,
        "is_open": None,
        "supported": False,
        "message": "is_asset_editor_open is not supported in UE5 Python API"
    })


def open_and_focus_asset(asset_path: str) -> Dict[str, Any]:
    """
    Open an asset editor and bring it to focus.

    If the editor is already open, it will be focused.
    If not, it will be opened and focused.

    Note: Due to UE5 Python API limitations, we cannot detect if the editor
    was already open. The action will always be 'opened_or_focused'.

    Args:
        asset_path: Path to the asset

    Returns:
        {success, data: {asset_path, asset_name, action}, error}

    Example:
        result = open_and_focus_asset('/Game/UI/Menus/WBP_MainMenu')
    """
    if not asset_path:
        return _make_error("Asset path cannot be empty")

    try:
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            return _make_error(f"Asset does not exist: {asset_path}")

        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset is None:
            return _make_error(f"Failed to load asset: {asset_path}")

        editor_subsystem = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
        if editor_subsystem is None:
            return _make_error("Could not get AssetEditorSubsystem")

        # Open the editor - UE will focus it if already open
        # Note: We cannot detect if it was already open due to API limitations
        editor_subsystem.open_editor_for_assets([asset])

        return _make_success({
            "asset_path": asset_path,
            "asset_name": asset_path.split('/')[-1],
            "action": "opened_or_focused"
        })

    except Exception as e:
        return _make_error(f"Error opening/focusing asset: {e}")


# ============================================================================
# Private Helper Functions
# ============================================================================

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
