# NodeToCode LLM Providers

LLM integration architecture and guide for adding/updating providers.

## LLM Integration & Providers

Manages interactions with LLMs.

*   **`Source/LLM/N2CLLMModule.h/.cpp` (`UN2CLLMModule`)**: Singleton orchestrating LLM requests. Initializes based on `UN2CSettings`, manages the active `IN2CLLMService`, processes JSON input, uses `UN2CSystemPromptManager` for prompts, and saves translations.
*   **`Source/LLM/IN2CLLMService.h`**: Interface for LLM service providers (init, send request, get config, etc.).
*   **`Source/LLM/N2CBaseLLMService.h/.cpp` (`UN2CBaseLLMService`)**: Abstract base class for LLM services, providing common init and request flow.
*   **Provider Implementations (`Source/LLM/Providers/`)**:
    *   Service classes (e.g., `UN2COpenAIService`, `UN2CAnthropicService`, `UN2CGeminiService`, `UN2CDeepSeekService`, `UN2COllamaService`, `UN2CLMStudioService`) inheriting `UN2CBaseLLMService`.
    *   Implement provider-specific API endpoint, auth, headers, and payload formatting (using `UN2CLLMPayloadBuilder`).
*   **`Source/LLM/N2CLLMProviderRegistry.h/.cpp` (`UN2CLLMProviderRegistry`)**: Singleton factory for creating LLM service instances based on `EN2CLLMProvider`.
*   **`Source/LLM/N2CLLMPayloadBuilder.h/.cpp` (`UN2CLLMPayloadBuilder`)**: Utility for building provider-specific JSON request payloads. Includes `GetN2CResponseSchema()` for the expected LLM output format.
*   **`Source/LLM/N2CResponseParserBase.h/.cpp` (`UN2CResponseParserBase`)**: Abstract base for LLM response parsers. Handles common N2C JSON structure.
*   **Parser Implementations (`Source/LLM/Providers/`)**:
    *   Parser classes (e.g., `UN2COpenAIResponseParser`, `UN2CLMStudioResponseParser`) inheriting `UN2CResponseParserBase`.
    *   Handle provider-specific JSON response structures, error formats, and extract core message content and token usage.
*   **`Source/LLM/N2CHttpHandlerBase.h/.cpp` & `Source/LLM/N2CHttpHandler.h`**: `UN2CHttpHandlerBase` (abstract) and `UN2CHttpHandler` (concrete) for sending HTTP POST requests to LLM APIs.
*   **`Source/LLM/N2CSystemPromptManager.h/.cpp` (`UN2CSystemPromptManager`)**: Loads and manages system prompts from `.md` files in `Content/Prompting/`. Handles language-specific prompts and prepends reference source files from settings.
*   **`Content/Prompting/CodeGen_[Language].md`**: Markdown files with detailed LLM instructions for each target language.
*   **`Source/LLM/N2CLLMTypes.h`**: Defines `EN2CLLMProvider`, `EN2CSystemStatus`, `FN2CLLMConfig`, and LLM-related delegates.
*   **`Source/LLM/N2CLLMModels.h/.cpp` & `Source/LLM/N2CLLMPricing.h`**: Enums for provider-specific models (`EN2COpenAIModel`, etc.), `FN2CLLMModelUtils` for model string IDs & pricing, and `FN2C[Provider]Pricing` structs.
*   **`Source/LLM/N2COllamaConfig.h`**: `FN2COllamaConfig` USTRUCT for Ollama-specific settings.

## Structured Output Support

Comprehensive structured output support across providers:

*   **OpenAI**: `json_schema` type with strict schema validation
*   **LM Studio**: OpenAI-compatible with `strict: true` parameter
*   **Gemini**: `responseMimeType` and `responseSchema` in generation config
*   **DeepSeek**: `json_object` type format
*   **Ollama**: Direct schema in `format` field
*   **Schema Definition**: `GetN2CResponseSchema()` provides consistent N2C translation format

## Prepended Model Commands

Support for model-specific command prefixes for local providers:

*   **Ollama Configuration** (`N2COllamaConfig.h`):
    *   `PrependedModelCommand`: Text prepended to user messages
    *   Use cases: `/no_think`, `/think`, reasoning control
*   **LM Studio Configuration** (`N2CSettings.h`):
    *   `LMStudioPrependedModelCommand`: Similar functionality for LM Studio
*   **Implementation**: Commands are prepended with `\n\n` separator before N2C JSON payload in both services.

---

## Adding/Updating LLM Providers/Models (Guide)

1.  **Define Core Types (`N2CLLMTypes.h`, `N2CLLMModels.h/.cpp`, `N2CLLMPricing.h`):**
    *   Add to `EN2CLLMProvider` enum (if new provider). Current providers: OpenAI, Anthropic, Gemini, DeepSeek, Ollama, LMStudio.
    *   Define/update model enum (e.g., `EN2CNewProviderModel`). Note: Ollama and LMStudio use dynamic string-based model names instead of enums.
    *   Update `FN2CLLMModelUtils`: Add `Get[ProviderName]ModelValue()`, `Get[ProviderName]Pricing()`, and static pricing map entry.
    *   If new pricing structure, define `FN2CNewProviderPricing`.

2.  **Plugin Settings (`N2CSettings.h/.cpp`):**
    *   Add UPROPERTY for new provider's model enum, API Key UI string, and pricing map (if new).
    *   Update `GetActiveApiKey()`, `GetActiveModel()`, and pricing getters.
    *   In constructor/`InitializePricing()`, set default model and pricing.
    *   In `PostEditChangeProperty()`, handle saving the new API key UI string to `UserSecrets`.

3.  **User Secrets (`N2CUserSecrets.h/.cpp`):**
    *   Add `FString` UPROPERTY for the new provider's API key.
    *   Update `LoadSecrets()` and `SaveSecrets()` to handle the new key.

4.  **Service Implementation (`Source/LLM/Providers/`):**
    *   Create `N2CNewProviderService.h/.cpp` inheriting `UN2CBaseLLMService`.
    *   Implement:
        *   `CreateResponseParser()`: Return new `UN2CNewProviderResponseParser`.
        *   `GetConfiguration()`: Provide endpoint, auth token, system prompt support.
        *   `GetProviderHeaders()`: Define HTTP headers.
        *   `FormatRequestPayload()`: Use `UN2CLLMPayloadBuilder` with `ConfigureForNewProvider()`.
        *   `GetDefaultEndpoint()`.

5.  **Response Parser (`Source/LLM/Providers/`):**
    *   Create `N2CNewProviderResponseParser.h/.cpp` inheriting `UN2CResponseParserBase`.
    *   Implement `ParseLLMResponse()`: Deserialize provider JSON, handle errors, extract core N2C JSON content and token usage, then call `Super::ParseLLMResponse()`.

6.  **Payload Builder (`N2CLLMPayloadBuilder.h/.cpp`):**
    *   Add `ConfigureForNewProvider()` method.
    *   Update setters (`SetTemperature`, `SetMaxTokens`, etc.) with a `case EN2CLLMProvider::NewProvider:` to handle provider-specific payload structures.

7.  **LLM Module Registration (`N2CLLMModule.cpp`):**
    *   In `InitializeProviderRegistry()`, call `Registry->RegisterProvider(EN2CLLMProvider::NewProvider, UN2CNewProviderService::StaticClass());`.

8.  **System Prompts (`Content/Prompting/`, `N2CSystemPromptManager.cpp` - Optional):**
    *   If needed, add new `.md` prompt files and update `LoadPrompts()` in `UN2CSystemPromptManager`.

9.  **Local Provider Configuration (for local LLMs like Ollama/LMStudio):**
    *   For local providers, consider configuration patterns:
        *   **Ollama Pattern**: Separate config struct (`FN2COllamaConfig`) with extensive parameters
        *   **LM Studio Pattern**: Simple settings directly in `UN2CSettings` with minimal configuration
    *   Key considerations for local providers:
        *   No API key authentication (or simple dummy keys)
        *   Custom endpoint configuration
        *   Free pricing (local models)
        *   Dynamic model names (strings vs enums)
        *   Temperature parameter handling (skip for LM Studio to allow UI control)
        *   Support for prepended model commands for reasoning control

10. **Pricing**
    *   Always assume that you are unsure about the pricing of models when implementing a new one unless explicitly told the prices. If you are not told what the prices are, then search for them online via official documentation or other reputable resources.
