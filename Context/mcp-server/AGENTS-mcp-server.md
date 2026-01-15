# NodeToCode MCP Server

Model Context Protocol server implementation for exposing Blueprint functionality to AI assistants.

## MCP Tools Development Guide

For creating new MCP tools, see: `@Source/Private/MCP/Tools/AGENTS.md`

## MCP Server Architecture

The MCP server is built on HTTP transport with full JSON-RPC 2.0 compliance:

*   **Transport Layer**: HTTP server on port 27000 (configurable via settings)
*   **Protocol**: JSON-RPC 2.0 with proper error handling and batch support
*   **Lifecycle**: Auto-starts with plugin, graceful shutdown on exit
*   **Session Management**: Protocol version negotiation and session tracking

## Core Server Components (`Source/MCP/Server/`)

*   **`N2CMcpHttpServerManager.h/.cpp`**: Central server management
    *   Singleton pattern for server lifecycle control
    *   HTTP routing: `/mcp` (main), `/mcp/health` (health check)
    *   CORS headers for local development
    *   Client notification channel management
    *   Session protocol version tracking

*   **`N2CMcpHttpRequestHandler.h/.cpp`**: Request processing engine
    *   Full JSON-RPC 2.0 message parsing and validation
    *   Method routing with extensible dispatch system
    *   Batch request processing support
    *   Protocol version negotiation (supports "2025-03-26", "2024-11-05")
    *   Comprehensive error responses with proper codes

*   **`N2CMcpJsonRpcTypes.h/.cpp`**: JSON-RPC type system
    *   `FJsonRpcRequest/Response/Error/Notification` structures
    *   `FJsonRpcUtils` for serialization/deserialization
    *   Standard error codes (-32700 to -32603)
    *   Message type detection and validation

*   **`IN2CMcpNotificationChannel.h`**: Notification transport abstraction
    *   Interface for future WebSocket/SSE support
    *   Enables server-initiated notifications
    *   Session-based channel management

## Validation Framework (`Source/MCP/Validation/`)

*   **`N2CMcpRequestValidator.h/.cpp`**: Centralized request validation
    *   Method-specific validators (tools/call, resources/read, etc.)
    *   Generic validation helpers for common patterns
    *   Required/optional field validation
    *   Type checking with detailed error messages

## Progress Tracking System (`Source/MCP/Progress/`)

*   **`N2CMcpProgressTracker.h/.cpp`**: Real-time operation tracking
    *   Thread-safe progress management
    *   Progress notifications via MCP protocol
    *   Percentage-based progress reporting
    *   Integration with notification broadcasting

## Tool Subsystem (`Source/MCP/Tools/`)

The tool system provides a scalable framework for exposing Blueprint operations:

*   **Core Infrastructure**:
    *   `IN2CMcpTool.h`: Tool interface contract
    *   `N2CMcpToolBase.h/.cpp`: Base class with common functionality
    *   `N2CMcpToolTypes.h/.cpp`: Tool definition and result structures
    *   `N2CMcpToolManager.h/.cpp`: Thread-safe tool registry
    *   `N2CMcpToolRegistry.h/.cpp`: Automatic registration system

*   **Tool Registration**:
    *   `REGISTER_MCP_TOOL` macro for static initialization
    *   Self-contained tools auto-register on plugin load
    *   Game Thread execution support with timeouts
    *   Input schema validation framework

*   **Tool Organization** (`Implementations/`):
    *   **Blueprint/** - Blueprint-related tools
        *   **Analysis/** - Blueprint inspection (get-focused-blueprint, list-blueprint-functions, list-overridable-functions)
        *   **Functions/** - Function management (create/open/delete-blueprint-function)
        *   **Variables/** - Variable creation (create-variable, create-local-variable, search-variable-types)
        *   **Graph/** - Node operations (search-blueprint-nodes, add-bp-node-to-active-graph, connect-pins)
        *   **Organization/** - Tagging system (tag-blueprint-graph, list-blueprint-tags, remove-tag-from-graph)
    *   **Translation/** - Code translation tools (translate-focused-blueprint, get-available-translation-targets, get-available-llm-providers, get-translation-output-directory)
    *   **FileSystem/** - File operations (read-path, read-file)
    *   **ContentBrowser/** - Content browser operations (open-content-browser-path, read-content-browser-path)
    *   **Python/** - Python script management tools
        *   `run-python` - Execute Python code in UE environment
        *   `list-python-scripts` - List scripts by category
        *   `search-python-scripts` - Search by name/description/tags
        *   `get-python-script` - Get full script code and metadata
        *   `get-script-functions` - Get function signatures via AST (token-efficient)
        *   `save-python-script` - Save script to library
        *   `delete-python-script` - Remove script from library

## Resource System (`Source/MCP/Resources/`)

Resources expose Blueprint data in a structured, queryable format:

*   **Core Infrastructure**:
    *   `N2CMcpResourceTypes.h/.cpp`: Resource definitions and contents
    *   `N2CMcpResourceManager.h/.cpp`: Resource registry with subscription support
    *   `IN2CMcpResource.h`: Resource interface for implementations

*   **Resource Types**:
    *   Static resources: Fixed URIs with direct handlers
    *   Dynamic resources: Template-based URI patterns
    *   Subscription support for real-time updates

*   **Implemented Resources** (`Implementations/`):
    *   `N2CMcpBlueprintResource.h/.cpp`:
        *   `nodetocode://blueprint/current`: Current focused Blueprint
        *   `nodetocode://blueprints/all`: List of open Blueprints
        *   `nodetocode://blueprint/{name}`: Blueprint by name (template)

## Prompt System (`Source/MCP/Prompts/`)

Prompts provide guided interactions for common Blueprint tasks:

*   **Core Infrastructure**:
    *   `N2CMcpPromptTypes.h`: Prompt structures with type alias for arguments
    *   `N2CMcpPromptManager.h/.cpp`: Prompt registry and execution
    *   `IN2CMcpPrompt.h`: Interface for prompt implementations

*   **Type Safety**:
    *   `FMcpPromptArguments` type alias for `TMap<FString, FString>`
    *   Avoids macro expansion issues with template parameters
    *   Consistent argument passing across the system

*   **Implemented Prompts** (`Implementations/`):
    *   `N2CMcpCodeGenerationPrompt`: Generate code with language/style options
    *   `N2CMcpBlueprintAnalysisPrompt`: Analyze Blueprint structure
    *   `N2CMcpRefactorPrompt`: Suggest refactoring improvements
    *   `N2CMcpPythonScriptingPrompt`: UE Python scripting with Context7 + script reuse enforcement

## MCP Protocol Implementation

The server implements these MCP methods:

*   **Core Methods**:
    *   `initialize`: Connection setup with capability negotiation
    *   `notifications/initialized`: Connection confirmation
    *   `ping`: Keep-alive testing

*   **Tool Methods**:
    *   `tools/list`: Enumerate available tools with schemas
    *   `tools/call`: Execute tools with arguments

*   **Resource Methods**:
    *   `resources/list`: List available resources and templates
    *   `resources/read`: Read resource contents

*   **Prompt Methods**:
    *   `prompts/list`: List available prompts
    *   `prompts/get`: Get prompt with argument substitution

*   **Advanced Features**:
    *   Batch request processing
    *   Progress notifications
    *   Protocol version negotiation
    *   Session management

## Editor Integration Updates

*   **`FN2CEditorIntegration` Refactoring**:
    *   Modularized Blueprint collection and translation
    *   Helper methods for MCP tool usage:
        *   `GetFocusedBlueprintAsJson()`: High-level API
        *   `CollectNodesFromGraph()`: Node collection
        *   `TranslateNodesToN2CBlueprint()`: Translation
        *   `SerializeN2CBlueprintToJson()`: Serialization

*   **Plugin Settings** (`N2CSettings.h/.cpp`):
    *   `McpServerPort`: Configurable server port
    *   Exposed in Editor UI for runtime configuration

## Testing and Development

Example test commands for the MCP server:

```bash
# Basic connectivity
curl -X GET http://localhost:27000/mcp/health

# Protocol initialization
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc": "2.0", "method": "initialize", "params": {"protocolVersion": "2025-03-26", "clientInfo": {"name": "test", "version": "1.0"}}, "id": 1}'

# List tools with schemas
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc": "2.0", "method": "tools/list", "id": 2}'

# Execute get-focused-blueprint tool
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc": "2.0", "method": "tools/call", "params": {"name": "get-focused-blueprint", "arguments": {}}, "id": 3}'

# List resources
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc": "2.0", "method": "resources/list", "id": 4}'

# Read current blueprint resource
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc": "2.0", "method": "resources/read", "params": {"uri": "nodetocode://blueprint/current"}, "id": 5}'

# Get code generation prompt
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc": "2.0", "method": "prompts/get", "params": {"name": "generate-code", "arguments": {"language": "python", "style": "concise"}}, "id": 6}'

# Batch request example
curl -X POST http://localhost:27000/mcp \
  -H "Content-Type: application/json" \
  -d '[{"jsonrpc": "2.0", "method": "ping", "id": 1}, {"jsonrpc": "2.0", "method": "tools/list", "id": 2}]'
```

## Related Projects

*   **NodeToCode MCP Bridge**: Python stdio-to-HTTP bridge for MCP clients
    *   Location: `Content/Python/mcp_bridge/nodetocode_bridge.py`
    *   Enables Claude for Desktop and other MCP client integration
    *   Handles protocol translation and session management
