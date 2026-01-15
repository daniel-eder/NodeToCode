# NodeToCode Python Scripting

MCP Bridge and Python script management system for LLM-powered UE automation.

## MCP Bridge (Python)

The NodeToCode MCP Bridge is a Python script that bridges MCP clients with the NodeToCode plugin's HTTP server:

*   **Location**: `Content/Python/mcp_bridge/nodetocode_bridge.py`
*   **Purpose**: Stdio-to-HTTP bridge for MCP protocol
*   **Key Features**:
    *   Reads JSON-RPC messages from stdin (from MCP clients like Claude)
    *   Forwards to NodeToCode's HTTP MCP server (localhost:27000)
    *   Relays responses back via stdout
    *   Handles session management and error conditions
*   **Usage**: `python nodetocode_bridge.py` or configure in MCP client settings

## Python Script Management System

Lean Python scripting framework for LLM-powered UE automation, designed to work with Context7 MCP for API documentation.

### Architecture

*   **Script Storage**: `Content/Python/scripts/<category>/` - Project-level, version-controlled
*   **Registry**: `Content/Python/scripts/script_registry.json` - Metadata index for fast search
*   **NodeToCode Module**: `Content/Python/nodetocode/` - Importable utilities

### Key Module Files

*   `scripts.py`: Script CRUD operations (list, search, get, save, delete, get_script_functions)
*   `bridge.py`: Python wrappers for C++ bridge (tagging, LLM provider info)
*   `blueprint.py`: Blueprint operations (get_focused, compile, save)
*   `utils.py`: Common utilities and result formatting

### Token-Efficient Function Discovery

*   `get_script_functions(name)`: Uses AST parsing to extract function signatures without loading full implementation
*   Returns: function names, parameters, type hints, docstrings, line numbers
*   ~80% token savings vs `get_script()` for discovery phase

### Python-Only Mode

*   Setting: `bEnablePythonScriptOnlyMode` in Plugin Settings
*   Registers only essential tools: run-python, script management, translation tools
*   Designed for LLMs to write/reuse Python scripts via Context7 API docs

### python-scripting Prompt

Enforces proper workflow:
1. Search existing scripts first (reuse over rewrite)
2. Use Context7 to lookup `radial-hks/unreal-python-stubhub` for UE Python API
3. Import and compose existing script functions
4. Structure new scripts as reusable modules
5. Save useful scripts to grow the library

## MCP Python Tools

Available via MCP when Python scripting is enabled:

*   `run-python` - Execute Python code in UE environment
*   `list-python-scripts` - List scripts by category
*   `search-python-scripts` - Search by name/description/tags
*   `get-python-script` - Get full script code and metadata
*   `get-script-functions` - Get function signatures via AST (token-efficient)
*   `save-python-script` - Save script to library
*   `delete-python-script` - Remove script from library
