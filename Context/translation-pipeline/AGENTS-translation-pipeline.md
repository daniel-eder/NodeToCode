# NodeToCode Translation Pipeline

Blueprint node collection, translation to structured format, and node processing strategies.

## Directory Structure

```
Source/
├── Public/
│   ├── Core/
│   │   ├── N2CNodeCollector.h        → Collects nodes from Blueprint graphs
│   │   ├── N2CNodeTranslator.h       → Converts nodes to N2CBlueprint format
│   │   └── N2CSerializer.h           → JSON serialization/deserialization
│   │
│   ├── Models/
│   │   ├── N2CBlueprint.h            → FN2CBlueprint, FN2CGraph, FN2CFlows, etc.
│   │   ├── N2CNode.h                 → FN2CNodeDefinition, EN2CNodeType
│   │   ├── N2CPin.h                  → FN2CPinDefinition, EN2CPinType
│   │   ├── N2CTranslation.h          → LLM response structures
│   │   └── N2CBatchTranslationTypes.h → Batch translation types and delegates
│   │
│   ├── Utils/Processors/
│   │   ├── N2CNodeProcessor.h        → IN2CNodeProcessor interface
│   │   ├── N2CBaseNodeProcessor.h    → Base processor implementation
│   │   ├── N2CNodeProcessorFactory.h → Factory for node processors
│   │   └── N2C*Processor.h           → Specific processor implementations
│   │
│   ├── LLM/
│   │   └── N2CBatchTranslationOrchestrator.h → Batch translation management
│   │
│   └── BlueprintLibraries/
│       └── N2CBatchTranslationBlueprintLibrary.h → Blueprint API
│
└── Private/
    ├── Core/
    │   ├── N2CNodeCollector.cpp
    │   ├── N2CNodeTranslator.cpp
    │   └── N2CSerializer.cpp
    │
    ├── Models/
    │   ├── N2CBlueprint.cpp
    │   ├── N2CNode.cpp
    │   └── N2CPin.cpp
    │
    ├── Utils/Processors/
    │   └── N2C*Processor.cpp
    │
    ├── LLM/
    │   └── N2CBatchTranslationOrchestrator.cpp
    │
    └── BlueprintLibraries/
        └── N2CBatchTranslationBlueprintLibrary.cpp
```

## Core Translation Pipeline

Handles transforming Blueprint nodes into a structured, LLM-processable format.

### Node Collector

**`Source/Public/Core/N2CNodeCollector.h` (`FN2CNodeCollector`)**

Singleton that gathers `UK2Node` instances and pin details from `UEdGraph`.

| Method | Description |
|--------|-------------|
| `CollectNodesFromGraph()` | Collects all nodes from a specific graph |
| `CollectPinsFromNode()` | Collects input/output pins from a node |

Private helpers: `ValidatePin`, `GetK2PinMetadata`, `CollectPinDefaultValue`, `CollectPinConnections`

### Node Translator

**`Source/Public/Core/N2CNodeTranslator.h` (`FN2CNodeTranslator`)**

Singleton converting collected `UK2Node`s into `FN2CBlueprint`.

| Method | Description |
|--------|-------------|
| `GenerateN2CStruct()` | Process array of nodes into FN2CBlueprint |
| `GetN2CBlueprint()` | Get the translated Blueprint structure |
| `ProcessSingleNode()` | Convert a single UK2Node (useful for MCP tools) |
| `GetNodeIDMap()` / `GetPinIDMap()` | Get GUID-to-ID mappings |
| `PreserveIDMaps()` | Copy ID maps before they get cleared |

Key responsibilities:
- Manages ID generation (N1, N2, etc. for nodes; P1, P2, etc. for pins)
- Uses `FN2CNodeProcessorFactory` for node-specific property extraction
- Determines types via `FN2CNodeTypeRegistry`
- Handles nested graphs with depth tracking
- Processes related structs/enums used by nodes
- Traces connections through knot nodes

### Serializer

**`Source/Public/Core/N2CSerializer.h` (`FN2CSerializer`)**

Static utility for serializing `FN2CBlueprint` to JSON and deserializing.

| Method | Description |
|--------|-------------|
| `ToJson()` | Convert FN2CBlueprint to JSON string |
| `FromJson()` | Parse JSON string to FN2CBlueprint |
| `NodeToJsonObject()` | Convert single node to JSON (for MCP tools) |
| `SetPrettyPrint()` / `SetIndentLevel()` | Configure formatting |

## Data Models

### Blueprint Structure

**`Source/Public/Models/N2CBlueprint.h`**

| Struct | Description |
|--------|-------------|
| `FN2CBlueprint` | Top-level container (version, metadata, graphs, structs, enums) |
| `FN2CMetadata` | Blueprint name, type (`EN2CBlueprintType`), class |
| `FN2CGraph` | Single graph with nodes and `FN2CFlows` |
| `FN2CFlows` | Execution chains and data pin connections |
| `FN2CVersion` | Version info (always "1.0.0") |

**User-Defined Types:**

| Struct | Description |
|--------|-------------|
| `FN2CStruct` | Blueprint-defined struct with members |
| `FN2CStructMember` | Member with type, container flags, key types |
| `FN2CEnum` | Blueprint-defined enum |
| `FN2CEnumValue` | Single enum value with optional comment |

**Enums:**

| Enum | Values |
|------|--------|
| `EN2CBlueprintType` | Normal, Const, MacroLibrary, Interface, LevelScript, FunctionLibrary |
| `EN2CGraphType` | EventGraph, Function, Composite, Macro, Construction, Animation, Struct, Enum |
| `EN2CStructMemberType` | Bool, Byte, Int, Float, String, Name, Text, Vector, Vector2D, Rotator, Transform, Class, Object, Struct, Enum, Array, Set, Map, Custom |

### Node Definition

**`Source/Public/Models/N2CNode.h`**

`FN2CNodeDefinition` contains:
- `ID` - Short identifier (e.g., "N1")
- `NodeType` - `EN2CNodeType` enum value
- `Name` - Display name or function name
- `MemberParent` - Class/scope containing the function/variable
- `MemberName` - Function/variable name being accessed
- `Comment` - Node comment if any
- `bPure`, `bLatent` - Node behavior flags
- `InputPins`, `OutputPins` - Arrays of `FN2CPinDefinition`

**`EN2CNodeType`** categories (~100+ node types):

| Category | Examples |
|----------|----------|
| Function Calls | `CallFunction`, `CallArrayFunction`, `CallDelegate`, `CallParentFunction`, `FunctionEntry`, `FunctionResult` |
| Variables | `VariableSet`, `VariableGet`, `LocalVariable`, `FunctionParameter`, `TemporaryVariable` |
| Events | `Event`, `CustomEvent`, `ActorBoundEvent`, `ComponentBoundEvent`, `InputAction`, `InputKey` |
| Flow Control | `ForEachLoop`, `WhileLoop`, `ForLoop`, `Sequence`, `Branch`, `Select`, `Gate`, `MultiGate`, `DoOnce`, `Tunnel` |
| Switches | `Switch`, `SwitchInt`, `SwitchString`, `SwitchEnum`, `SwitchName` |
| Structs/Objects | `MakeStruct`, `BreakStruct`, `SetFieldsInStruct`, `StructMemberGet`, `StructMemberSet` |
| Containers | `MakeArray`, `MakeMap`, `MakeSet`, `GetArrayItem` |
| Casting | `DynamicCast`, `ClassDynamicCast`, `CastByteToEnum`, `ConvertAsset` |
| Delegates | `AddDelegate`, `CreateDelegate`, `ClearDelegate`, `RemoveDelegate`, `CallDelegate` |
| Async/Latent | `AsyncAction`, `BaseAsyncTask` |
| Components | `AddComponent`, `AddComponentByClass` |
| Utility | `ConstructObjectFromClass`, `Timeline`, `SpawnActor`, `FormatText`, `GetSubsystem` |
| Math/Logic | `MathExpression`, `EaseFunction`, `EnumEquality`, `CommutativeAssociativeBinaryOperator` |
| Special | `Self`, `Composite`, `MacroInstance`, `Message` |

### Pin Definition

**`Source/Public/Models/N2CPin.h`**

`FN2CPinDefinition` contains:
- `ID` - Short local ID (e.g., "P1")
- `Name` - Display name
- `Type` - `EN2CPinType` enum value
- `SubType` - Optional subtype (e.g., struct name)
- `DefaultValue` - Default value if any
- `bConnected` - Whether pin is connected
- `bIsReference`, `bIsConst` - Modifier flags
- `bIsArray`, `bIsMap`, `bIsSet` - Container flags

**`EN2CPinType`** values:

| Category | Types |
|----------|-------|
| Basic | Exec, Boolean, Byte, Integer, Integer64, Float, Double, Real, String, Name, Text |
| Math | Vector, Vector2D, Vector4D, Rotator, Transform, Quat |
| Object | Object, Class, Interface, SoftObject, SoftClass, AssetId |
| Complex | Struct, Enum, Delegate, MulticastDelegate |
| Containers | Array, Set, Map |
| Assets | Material, Texture, StaticMesh, SkeletalMesh |
| Animation | Pose, Animation, BlendSpace |
| Special | FieldPath, Bitmask, Self, Index, Wildcard |

### Translation Response

**`Source/Public/Models/N2CTranslation.h`**

| Struct | Description |
|--------|-------------|
| `FN2CTranslationResponse` | Contains `FN2CGraphTranslation[]` and `FN2CTranslationUsage` |
| `FN2CGraphTranslation` | Translated code for one graph (name, type, class, code) |
| `FN2CGeneratedCode` | Declaration, implementation, and implementation notes |
| `FN2CTranslationUsage` | Input/output token counts |

## Node Processing Strategy

Handles diverse Blueprint node types using a strategy pattern.

### Framework Components

| Class | Description |
|-------|-------------|
| `IN2CNodeProcessor` | Interface with `Process()` method |
| `FN2CBaseNodeProcessor` | Base implementation with `ExtractNodeProperties()` |
| `FN2CNodeProcessorFactory` | Singleton factory mapping `EN2CNodeType` to processors |

### Specific Node Processors

**`Source/Public/Utils/Processors/` and `Source/Private/Utils/Processors/`**

| Processor | Handles |
|-----------|---------|
| `FN2CArrayProcessor` | `MakeArray`, `MakeMap`, `MakeSet`, `MakeContainer`, `GetArrayItem` |
| `FN2CCastProcessor` | `DynamicCast`, `ClassDynamicCast`, `CastByteToEnum` |
| `FN2CDelegateProcessor` | `CreateDelegate`, `AddDelegate`, `RemoveDelegate`, `ClearDelegate`, `CallDelegate` |
| `FN2CEventProcessor` | `Event`, `CustomEvent`, `ActorBoundEvent`, `ComponentBoundEvent` |
| `FN2CFlowControlProcessor` | `ExecutionSequence`, `IfThenElse`, `Select`, `Switch`, `MultiGate`, `DoOnceMultiInput` |
| `FN2CFunctionCallProcessor` | `CallFunction` and variations |
| `FN2CFunctionEntryProcessor` | `FunctionEntry`, `FunctionResult`, `MacroInstance` |
| `FN2CStructProcessor` | `MakeStruct`, `BreakStruct`, `SetFieldsInStruct`, `StructOperation` |
| `FN2CVariableProcessor` | `Variable`, `VariableGet`, `VariableSet` |

## Batch Translation System

Batch translation of multiple Blueprint graphs from `FN2CTagInfo` arrays.

### Types and Delegates

**`Source/Public/Models/N2CBatchTranslationTypes.h`**

| Type | Description |
|------|-------------|
| `EN2CBatchItemStatus` | Pending, Processing, Completed, Failed, Skipped |
| `FN2CBatchTranslationItem` | Internal tracking struct per graph (with cached JSON payload) |
| `FN2CBatchTranslationResult` | Summary with counts, timing, token usage, output path |
| `FN2CBatchJsonExportResult` | Result for JSON-only export (no LLM translation) |

**Delegates (Blueprint-compatible):**
- `FOnBatchItemTranslationComplete(TagInfo, Response, bSuccess, ItemIndex, TotalCount)`
- `FOnBatchTranslationComplete(Result)`
- `FOnBatchTranslationProgress(CurrentIndex, TotalCount, CurrentGraphName)`
- `FOnBatchJsonExportComplete(Result)`

**Native Delegates (C++ only, for Slate widgets):**
- `FOnBatchItemTranslationCompleteNative`
- `FOnBatchTranslationCompleteNative`
- `FOnBatchTranslationProgressNative`

### Orchestrator

**`Source/Public/LLM/N2CBatchTranslationOrchestrator.h` (`UN2CBatchTranslationOrchestrator`)**

Singleton orchestrator managing batch operations:

| Method | Description |
|--------|-------------|
| `StartBatchTranslation()` | Begin batch translation from TagInfo array |
| `CancelBatch()` | Cancel current batch (marks remaining as Skipped) |
| `IsBatchInProgress()` | Check if batch is running |
| `GetBatchProgress()` | Get progress percentage (0.0 - 1.0) |
| `GetCurrentItemIndex()` / `GetTotalItemCount()` | Progress tracking |
| `GetCurrentResult()` | Get current/partial result |
| `BatchExportJson()` | Export to JSON without LLM translation |

Key features:
- Loads blueprints via `FSoftObjectPath::TryLoad()` without editor focus
- Finds graphs by GUID in `UbergraphPages`, `FunctionGraphs`, `MacroGraphs`
- Sequential translation reusing `UN2CLLMModule::ProcessN2CJson()`
- Outputs to `BatchTranslation_{timestamp}/` with per-graph subfolders and `BatchSummary.json`
- Generates combined markdown file for easy LLM chat upload
- Blueprint caching to avoid reloading

### Blueprint Library

**`Source/Public/BlueprintLibraries/N2CBatchTranslationBlueprintLibrary.h`**

| Function | Description |
|----------|-------------|
| `StartBatchTranslation(TaggedGraphs)` | Direct array input |
| `TranslateGraphsWithTag(Tag, Category)` | All graphs with specific tag |
| `TranslateGraphsInCategory(Category)` | All graphs in category |
| `CancelBatchTranslation()` | Cancel current batch |
| `GetBatchProgress()` | Get progress percentage |
| `GetBatchOrchestrator()` | Get orchestrator for delegate binding |
| `BatchExportJson(TaggedGraphs, bMinifyJson)` | Export JSON without LLM |
| `ExportGraphsWithTagToJson(Tag, Category, bMinifyJson)` | Export tagged graphs to JSON |
| `ExportGraphsInCategoryToJson(Category, bMinifyJson)` | Export category to JSON |

## Translation Workflow

1. **Collection**: `FN2CNodeCollector` gathers nodes from the selected graph(s)
2. **Translation**: `FN2CNodeTranslator` converts nodes to `FN2CBlueprint` format
   - Assigns unique IDs to nodes (N1, N2...) and pins (P1, P2...)
   - Uses `FN2CNodeProcessorFactory` to get specialized processors
   - Builds execution and data flow connections
   - Processes related structs/enums
3. **Serialization**: `FN2CSerializer` converts to JSON string
4. **LLM Processing**: JSON is sent to the configured LLM provider
5. **Response Parsing**: LLM response is parsed into `FN2CTranslationResponse`
6. **Display**: Generated code is shown in the code editor widget
