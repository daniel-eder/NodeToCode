# AGENTS.md

This file provides guidance to AI coding agents when working with code in this repository.

# NodeToCode Unreal Engine 5 Plugin

# CRITICAL - START

## Understanding NodeToCode's MCP Server Tools

@Source/Private/MCP/Tools/AGENTS.md

## Unreal Engine Source Path

Run `Content/Scripts/detect-ue-path.sh` (Mac/Linux) or `Content\Scripts\detect-ue-path.bat` (Windows) to detect your UE installation and update this file.

UE_SOURCE_PATH: <!-- RUN DETECT SCRIPT TO POPULATE -->

This is where source files for Unreal Engine 5 can be found and searched through for research purposes. NEVER try to modify files here.

## TESTING COMPILATION

When testing code changes, please run build.sh (if on Mac) or build.ps1 (if on Windows) found in the root of the NodeToCode plugin folder (./build.sh or .\build.ps1). This will build the project and output timestamped build logs in the BuildLogs folder.

# CRITICAL - END

## Overview

NodeToCode is an Unreal Engine 5 plugin for translating Blueprints into various programming languages (C++, Python, JavaScript, C#, Swift, Pseudocode) using Large Language Models (LLMs). It integrates into the Unreal Editor, providing tools to collect Blueprint nodes, manage LLM interactions, and display generated code with syntax highlighting.

**Key Features:**
*   **Blueprint-to-Code Translation:** Converts selected Blueprint graphs/entire Blueprints.
*   **Multi-LLM Support:** Integrates with OpenAI, Anthropic, Google Gemini, DeepSeek, local Ollama instances, and LM Studio.
*   **Editor Integration:** Adds a "Node to Code" toolbar button in the Blueprint Editor for translation, JSON export, and a dedicated plugin window.
*   **Interactive Code Editor:** Displays generated code with syntax highlighting and theme customization.
*   **Configuration:** Plugin settings for LLM provider selection, API keys (stored securely), target language, and logging.
*   **Reasoning Model Support:** Prepended model commands for controlling reasoning models and feature toggles.

## Context Navigation

For detailed information on specific areas of the codebase, navigate to the relevant context file:

```
Context/
├── architecture/
│   └── AGENTS-architecture.md          → Core plugin setup, editor integration, settings
│
├── translation-pipeline/
│   └── AGENTS-translation-pipeline.md  → Blueprint node → code conversion, batch translation
│
├── llm-providers/
│   └── AGENTS-llm-providers.md         → LLM services, adding/updating providers
│
├── mcp-server/
│   └── AGENTS-mcp-server.md            → MCP server implementation, tools, resources
│
├── python-scripting/
│   └── AGENTS-python-scripting.md      → Python bridge, script management system
│
├── code-editor/
│   └── AGENTS-code-editor.md           → Syntax highlighting, themes, code display
│
├── development-workflow/
│   └── AGENTS-development-workflow.md  → Git submodules, IDE setup, build process
```

### Quick Reference

| Task | Context File |
|------|--------------|
| Plugin initialization, settings | `Context/architecture/` |
| Node collection, translation | `Context/translation-pipeline/` |
| Adding LLM providers/models | `Context/llm-providers/` |
| MCP tools development | `Context/mcp-server/` + `@Source/Private/MCP/Tools/AGENTS.md` |
| Python automation | `Context/python-scripting/` |
| Code editor styling | `Context/code-editor/` |
| Git workflow, IDE config | `Context/development-workflow/` |
