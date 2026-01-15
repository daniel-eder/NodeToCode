# NodeToCode Code Editor

In-plugin code viewing component with syntax highlighting and theme support.

## Code Editor UI and Syntax Highlighting

Provides the in-plugin code viewing component.

*   **`Source/Code Editor/Widgets/SN2CCodeEditor.h/.cpp` (`SN2CCodeEditor`)**: Core Slate widget for code display/editing, using `SMultiLineEditableText` and `FN2CRichTextSyntaxHighlighter`. Manages language, theme, font size.
*   **`Source/Code Editor/Widgets/N2CCodeEditorWidget.h/.cpp` (`UN2CCodeEditorWidget`)**: UMG wrapper for `SN2CCodeEditor`, for use in EUWs.
*   **`Source/Code Editor/Syntax/N2CSyntaxDefinition.h`**: Abstract base for language-specific syntax rules (keywords, operators, comments).
*   **Language-Specific Syntax Definitions (`Source/Code Editor/Syntax/`)**: Implementations like `FN2CCPPSyntaxDefinition` for C++, Python, JS, C#, Swift, Pseudocode.
*   **`Source/Code Editor/Syntax/N2CSyntaxDefinitionFactory.h/.cpp` (`FN2CSyntaxDefinitionFactory`)**: Singleton factory for `FN2CSyntaxDefinition` instances based on `EN2CCodeLanguage`.
*   **`Source/Code Editor/Syntax/N2CRichTextSyntaxHighlighter.h/.cpp` (`FN2CRichTextSyntaxHighlighter`)**: Performs syntax highlighting using a `FN2CSyntaxDefinition` and `FSyntaxTokenizer`. Applies `FTextBlockStyle` based on tokens.
*   **`Source/Code Editor/Syntax/N2CWhiteSpaceRun.h/.cpp` (`FN2CWhiteSpaceRun`)**: Custom `FSlateTextRun` for handling tab rendering.
*   **`Source/Code Editor/Models/N2CCodeEditorStyle.h/.cpp` (`FN2CCodeEditorStyle`)**: Manages global `FSlateStyleSet` for code editor syntax styles, sourced from `UN2CSettings`.
*   **`Source/Code Editor/Models/N2CCodeLanguage.h`**: `EN2CCodeLanguage` enum.

## Supported Languages

*   C++
*   Python
*   JavaScript
*   C#
*   Swift
*   Pseudocode

## Theme Configuration

Code editor themes are configured via `FN2CCodeEditorThemes` in `UN2CSettings`. Each theme defines colors for:

*   Keywords
*   Operators
*   Comments
*   Strings
*   Numbers
*   Types
*   Functions
*   Background
*   Default text
