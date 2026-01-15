# NodeToCode Architecture

Core plugin structure, editor integration, utilities, settings, and content organization.

## Plugin Module & Core Setup

*   **`NodeToCode.uplugin`**: Descriptor file (version, author, module definition: "NodeToCode" Editor module, PostEngineInit loading, Win64/Mac platforms).
*   **`Source/NodeToCode.Build.cs`**: Build script defining dependencies (Core, UnrealEd, BlueprintGraph, HTTP, Slate, UMG, etc.).
*   **`Source/Public/NodeToCode.h` & `Source/Private/NodeToCode.cpp`**:
    *   `FNodeToCodeModule`: Main plugin class.
        *   `StartupModule`: Initializes logging, HTTP timeouts (via `UHttpTimeoutConfig` for `DefaultEngine.ini`), user secrets, styles (`N2CStyle`, `FN2CCodeEditorStyle`), editor integration (`FN2CEditorIntegration`), and widget factories.
        *   `ShutdownModule`: Cleans up resources.
    *   `UHttpTimeoutConfig`: Helper to adjust global HTTP timeouts for LLM requests.

## Editor Integration & UI Shell

Connects plugin logic to the Unreal Editor UI.

*   **`Source/Core/N2CEditorIntegration.h/.cpp` (`FN2CEditorIntegration`)**: Singleton for Blueprint Editor integration. Registers toolbar extensions, maps UI commands (`FN2CToolbarCommand`) to handlers (e.g., `ExecuteCollectNodesForEditor`). Tracks `LastFocusedGraph`.
*   **`Source/Core/N2CToolbarCommand.h/.cpp` (`FN2CToolbarCommand`)**: `TCommands` subclass defining toolbar UI actions (OpenWindow, CollectNodes, CopyJson).
*   **`Source/Core/N2CEditorWindow.h/.cpp` (`SN2CEditorWindow`)**: Slate widget for the main plugin UI. Registers a nomad tab spawner, embedding `NodeToCodeUI.uasset`.
*   **`Content/UI/NodeToCodeUI.uasset`**: Main Editor Utility Widget Blueprint for the plugin's UI.
*   **`Content/UI/Components/`**: `.uasset` EUW components for building `NodeToCodeUI.uasset`.
*   **`Source/Models/N2CStyle.h/.cpp` (`N2CStyle`)**: Manages plugin's global Slate style (toolbar icon).
*   **`Source/Core/N2CWidgetContainer.h/.cpp` (`UN2CWidgetContainer`)**: Persistent UObject outer for `NodeToCodeUI` to maintain state.

## Utilities & Settings

General helpers and configuration.

*   **`Source/Core/N2CSettings.h/.cpp` (`UN2CSettings`)**: `UDeveloperSettings` class for all plugin configurations (LLM provider, API keys UI, model selections, Ollama config, LM Studio config, pricing, target language, logging severity, code editor themes `FN2CCodeEditorThemes`, reference source files `ReferenceSourceFilePaths`, custom translation output path `CustomTranslationOutputDirectory`).
*   **`Source/Core/N2CUserSecrets.h/.cpp` (`UN2CUserSecrets`)**: UObject for secure local storage of API keys in `secrets.json`.
*   **`Source/Utils/N2CLogger.h/.cpp` (`FN2CLogger`)**: Singleton for centralized logging, integrates with `UE_LOG`.
*   **`Source/Models/N2CLogging.h`**: Defines `EN2CLogSeverity`, `EN2CErrorCategory`, `FN2CError`, and `LogNodeToCode` category.
*   **`Source/Utils/N2CNodeTypeRegistry.h/.cpp` (`FN2CNodeTypeRegistry`)**: Singleton mapping `UK2Node` classes to `EN2CNodeType`.
*   **Validation Utilities (`Source/Utils/Validators/`)**: `FN2CBlueprintValidator`, `FN2CNodeValidator`, `FN2CPinValidator` for data integrity checks.
*   **`Source/Utils/N2CPinTypeCompatibility.h/.cpp` (`FN2CPinTypeCompatibility`)**: Static utility for checking pin connection compatibility.

## Content Files

*   **`.github/`**: Funding and issue templates.
*   **`.gitignore`**: Standard Git ignore.
*   **`Content/Prompting/`**: `.md` files for LLM system prompts (e.g., `CodeGen_CPP.md`).
*   **`Content/UI/`**:
    *   **`Components/`**: `.uasset` EUW components for UI building (chat, code editor, browsers, etc.). Excludes backer/auth components.
    *   **`NodeToCodeUI.uasset`**: Main plugin EUW.
    *   **`Textures/Icons/`**: UI icon textures.
    *   **`Theming/`**: General UI theming assets.
*   **`LICENSE`**, **`README.md`**.
*   **`Resources/`**: Plugin/toolbar icons (`Icon128.png`, `button_icon.png`).
*   **`assets/`**: Documentation/promotional images.
