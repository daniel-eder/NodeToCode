# NodeToCode Translation Pipeline

Blueprint node collection, translation to structured format, and node processing strategies.

## Core Translation Pipeline

Handles transforming Blueprint nodes into a structured, LLM-processable format.

*   **`Source/Core/N2CNodeCollector.h/.cpp` (`FN2CNodeCollector`)**: Singleton that gathers `UK2Node` instances and pin details from `UEdGraph`.
*   **`Source/Core/N2CNodeTranslator.h/.cpp` (`FN2CNodeTranslator`)**: Singleton converting collected `UK2Node`s into `FN2CBlueprint`. Manages ID generation, uses `FN2CNodeProcessorFactory` for node-specific property extraction, determines types via `FN2CNodeTypeRegistry`, handles nested graphs, and processes related structs/enums.
*   **`Source/Models/N2CBlueprint.h/.cpp`**: Defines primary data structures:
    *   `FN2CBlueprint`: Top-level container (version, metadata, graphs, structs, enums).
    *   `FN2CMetadata`: Blueprint name, type (`EN2CBlueprintType`), class.
    *   `FN2CGraph`: Single graph with nodes and `FN2CFlows`.
    *   `FN2CFlows`: Execution and data pin connections.
    *   `FN2CStruct`, `FN2CStructMember`, `FN2CEnum`, `FN2CEnumValue`: User-defined types.
*   **`Source/Models/N2CNode.h/.cpp`**: Defines `FN2CNodeDefinition` (node ID, `EN2CNodeType`, name, pins, flags) and `EN2CNodeType` (enum for various `UK2Node` kinds).
*   **`Source/Models/N2CPin.h/.cpp`**: Defines `FN2CPinDefinition` (pin ID, name, `EN2CPinType`, subtype, default value, flags) and `EN2CPinType` (enum for pin kinds). Includes compatibility checks.
*   **`Source/Core/N2CSerializer.h/.cpp` (`FN2CSerializer`)**: Static utility for serializing `FN2CBlueprint` to JSON and deserializing. Supports pretty-printing.
*   **`Source/Models/N2CTranslation.h`**: Defines LLM output structures:
    *   `FN2CTranslationResponse`: Contains `FN2CGraphTranslation[]` and `FN2CTranslationUsage`.
    *   `FN2CGraphTranslation`: Translated code for one graph.
    *   `FN2CGeneratedCode`: Declaration, implementation, and notes.
    *   `FN2CTranslationUsage`: Input/output token counts.

## Node Processing Strategy

Handles diverse Blueprint node types using a strategy pattern.

*   **`Source/Utils/Processors/N2CNodeProcessor.h` (`IN2CNodeProcessor`)**: Interface for node processors.
*   **`Source/Utils/Processors/N2CBaseNodeProcessor.h/.cpp` (`FN2CBaseNodeProcessor`)**: Base implementation, handles common properties.
*   **`Source/Utils/Processors/N2CNodeProcessorFactory.h/.cpp` (`FN2CNodeProcessorFactory`)**: Singleton factory mapping `EN2CNodeType` to specific processor implementations.
*   **Specific Node Processors (`Source/Utils/Processors/`)**: Derived classes like `FN2CFunctionCallProcessor`, `FN2CEventProcessor`, `FN2CVariableProcessor`, etc., overriding `ExtractNodeProperties` for specific `UK2Node` types.

## Batch Translation System

Batch translation of multiple Blueprint graphs from `FN2CTagInfo` arrays:

*   **`Source/Public/Models/N2CBatchTranslationTypes.h`**: Defines batch types and delegates:
    *   `EN2CBatchItemStatus`: Pending, Processing, Completed, Failed, Skipped
    *   `FN2CBatchTranslationItem`: Internal tracking struct per graph
    *   `FN2CBatchTranslationResult`: Summary with counts, timing, token usage
    *   `FOnBatchItemTranslationComplete(TagInfo, Response, bSuccess, ItemIndex, TotalCount)`
    *   `FOnBatchTranslationComplete(Result)`, `FOnBatchTranslationProgress`

*   **`Source/LLM/N2CBatchTranslationOrchestrator.h/.cpp`**: Singleton orchestrator:
    *   Loads blueprints via `FSoftObjectPath::TryLoad()` without editor focus
    *   Finds graphs by GUID in `UbergraphPages`, `FunctionGraphs`, `MacroGraphs`
    *   Sequential translation reusing `UN2CLLMModule::ProcessN2CJson()`
    *   Outputs to `BatchTranslation_{timestamp}/` with per-graph subfolders and `BatchSummary.json`

*   **`Source/BlueprintLibraries/N2CBatchTranslationBlueprintLibrary.h/.cpp`**: Blueprint API:
    *   `StartBatchTranslation(TArray<FN2CTagInfo>)` - Direct array input
    *   `TranslateGraphsWithTag(Tag, Category)` - All graphs with specific tag
    *   `TranslateGraphsInCategory(Category)` - All graphs in category
    *   `CancelBatchTranslation()`, `GetBatchProgress()`, `GetBatchOrchestrator()`
