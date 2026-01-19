// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "Core/Widgets/SN2CContextWindowVisualizer.h"
#include "Core/Services/N2CTokenEstimationService.h"
#include "Core/N2CSettings.h"
#include "LLM/N2CLLMModelRegistry.h"
#include "LLM/N2CLLMModels.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Styling/AppStyle.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "SN2CContextWindowVisualizer"

// Color scheme (reusing from SN2CBatchProgressModal)
namespace N2CVisualizerColors
{
	const FLinearColor BgPanel = FLinearColor::FromSRGBColor(FColor(37, 37, 38));
	const FLinearColor TextPrimary = FLinearColor::FromSRGBColor(FColor(204, 204, 204));
	const FLinearColor TextSecondary = FLinearColor::FromSRGBColor(FColor(157, 157, 157));
	const FLinearColor TextMuted = FLinearColor::FromSRGBColor(FColor(107, 107, 107));
	const FLinearColor AccentOrange = FLinearColor::FromSRGBColor(FColor(212, 160, 74));
	const FLinearColor AccentGreen = FLinearColor::FromSRGBColor(FColor(78, 201, 176));
}

void SN2CContextWindowVisualizer::Construct(const FArguments& InArgs)
{
	// Initialize provider and model options
	PopulateProviderOptions();
	PopulateModelOptions();

	// Cache initial provider state
	const UN2CSettings* Settings = GetDefault<UN2CSettings>();
	CachedProvider = Settings->Provider;

	// Subscribe to model changes from the token estimation service
	FN2CTokenEstimationService& TokenService = FN2CTokenEstimationService::Get();
	ModelChangedHandle = TokenService.OnModelChanged.AddSP(this, &SN2CContextWindowVisualizer::OnModelChanged);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)

			// Provider/Model selection row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(SHorizontalBox)

				// Provider label and dropdown
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ProviderLabel", "Provider: "))
					.ColorAndOpacity(N2CVisualizerColors::TextSecondary)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 12.0f, 0.0f)
				[
					SAssignNew(ProviderComboBox, SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&ProviderOptions)
					.OnSelectionChanged(this, &SN2CContextWindowVisualizer::OnProviderSelectionChanged)
					.OnGenerateWidget(this, &SN2CContextWindowVisualizer::GenerateProviderWidget)
					.Content()
					[
						SNew(STextBlock)
						.Text(this, &SN2CContextWindowVisualizer::GetCurrentProviderText)
						.ColorAndOpacity(N2CVisualizerColors::AccentOrange)
					]
				]

				// Model label
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ModelLabel", "Model: "))
					.ColorAndOpacity(N2CVisualizerColors::TextSecondary)
				]

				// Model selection container (will hold either dropdown or text input)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SAssignNew(ModelSelectionContainer, SBox)
				]

				// Settings button
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked(this, &SN2CContextWindowVisualizer::OnSettingsClicked)
					.ToolTipText(LOCTEXT("SettingsTooltip", "Open NodeToCode Settings"))
					.ContentPadding(FMargin(2.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("\u2699"))) // Gear icon unicode
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
						.ColorAndOpacity(N2CVisualizerColors::TextSecondary)
					]
				]
			]

			// Total token count row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 2.0f)
			[
				SAssignNew(TokenCountTextBlock, STextBlock)
				.Text(this, &SN2CContextWindowVisualizer::GetTokenCountText)
				.ColorAndOpacity(N2CVisualizerColors::TextPrimary)
			]

			// Estimated batch cost row
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(CostTextBlock, STextBlock)
				.Text(this, &SN2CContextWindowVisualizer::GetEstimatedCostText)
				.ColorAndOpacity(N2CVisualizerColors::AccentGreen)
			]
		]
	];

	// Build the initial model selection widget
	RebuildModelSelectionWidget();
}

SN2CContextWindowVisualizer::~SN2CContextWindowVisualizer()
{
	// Unsubscribe from model changes
	if (ModelChangedHandle.IsValid())
	{
		FN2CTokenEstimationService::Get().OnModelChanged.Remove(ModelChangedHandle);
	}
}

void SN2CContextWindowVisualizer::UpdateForSelection(const TArray<FN2CTagInfo>& SelectedGraphs)
{
	TotalTokens = 0;
	TotalCost = 0.0f;
	SelectedGraphCount = SelectedGraphs.Num();

	if (SelectedGraphs.IsEmpty())
	{
		return;
	}

	FN2CTokenEstimationService& TokenService = FN2CTokenEstimationService::Get();

	// Accumulate token estimates for all selected graphs
	for (const FN2CTagInfo& GraphInfo : SelectedGraphs)
	{
		int32 GraphTokens = TokenService.GetTokenEstimate(GraphInfo);
		TotalTokens += GraphTokens;
	}

	// Calculate total cost
	TotalCost = TokenService.CalculateCost(TotalTokens);
}

void SN2CContextWindowVisualizer::OnModelChanged()
{
	// Model changed, force repaint to update displayed values
	Invalidate(EInvalidateWidgetReason::Paint);
}

// ==================== Provider/Model Selection ====================

void SN2CContextWindowVisualizer::PopulateProviderOptions()
{
	ProviderOptions.Empty();

	// Add all providers
	ProviderOptions.Add(MakeShared<FString>(TEXT("OpenAI")));
	ProviderOptions.Add(MakeShared<FString>(TEXT("Anthropic")));
	ProviderOptions.Add(MakeShared<FString>(TEXT("Gemini")));
	ProviderOptions.Add(MakeShared<FString>(TEXT("DeepSeek")));
	ProviderOptions.Add(MakeShared<FString>(TEXT("Ollama")));
	ProviderOptions.Add(MakeShared<FString>(TEXT("LM Studio")));
}

void SN2CContextWindowVisualizer::PopulateModelOptions()
{
	ModelOptions.Empty();

	const UN2CSettings* Settings = GetDefault<UN2CSettings>();
	FN2CLLMModelRegistry& Registry = FN2CLLMModelRegistry::Get();

	switch (Settings->Provider)
	{
	case EN2CLLMProvider::OpenAI:
		for (const auto& Pair : Registry.GetAllOpenAIModels())
		{
			ModelOptions.Add(MakeShared<FString>(Pair.Value.DisplayName));
		}
		break;
	case EN2CLLMProvider::Anthropic:
		for (const auto& Pair : Registry.GetAllAnthropicModels())
		{
			ModelOptions.Add(MakeShared<FString>(Pair.Value.DisplayName));
		}
		break;
	case EN2CLLMProvider::Gemini:
		for (const auto& Pair : Registry.GetAllGeminiModels())
		{
			ModelOptions.Add(MakeShared<FString>(Pair.Value.DisplayName));
		}
		break;
	case EN2CLLMProvider::DeepSeek:
		for (const auto& Pair : Registry.GetAllDeepSeekModels())
		{
			ModelOptions.Add(MakeShared<FString>(Pair.Value.DisplayName));
		}
		break;
	// Ollama and LM Studio use text input, not dropdown
	default:
		break;
	}

	// Refresh combo box if it exists
	if (ModelComboBox.IsValid())
	{
		ModelComboBox->RefreshOptions();
	}
}

void SN2CContextWindowVisualizer::RebuildModelSelectionWidget()
{
	if (!ModelSelectionContainer.IsValid())
	{
		return;
	}

	const UN2CSettings* Settings = GetDefault<UN2CSettings>();
	const bool bIsLocal = IsLocalProvider();

	if (bIsLocal)
	{
		// For local providers, use text input
		ModelComboBox.Reset();
		ModelSelectionContainer->SetContent(
			SAssignNew(LocalModelTextBox, SEditableTextBox)
			.Text(this, &SN2CContextWindowVisualizer::GetLocalModelText)
			.OnTextCommitted(this, &SN2CContextWindowVisualizer::OnLocalModelTextCommitted)
			.MinDesiredWidth(150.0f)
			.SelectAllTextWhenFocused(true)
		);
	}
	else
	{
		// For cloud providers, use dropdown
		LocalModelTextBox.Reset();
		PopulateModelOptions();
		ModelSelectionContainer->SetContent(
			SAssignNew(ModelComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&ModelOptions)
			.OnSelectionChanged(this, &SN2CContextWindowVisualizer::OnModelSelectionChanged)
			.OnGenerateWidget(this, &SN2CContextWindowVisualizer::GenerateModelWidget)
			.Content()
			[
				SNew(STextBlock)
				.Text(this, &SN2CContextWindowVisualizer::GetModelNameText)
				.ColorAndOpacity(N2CVisualizerColors::AccentOrange)
			]
		);
	}
}

void SN2CContextWindowVisualizer::OnProviderSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	UN2CSettings* Settings = GetMutableDefault<UN2CSettings>();

	// Map string to enum
	EN2CLLMProvider NewProvider = EN2CLLMProvider::Anthropic;
	if (*NewValue == TEXT("OpenAI"))
	{
		NewProvider = EN2CLLMProvider::OpenAI;
	}
	else if (*NewValue == TEXT("Anthropic"))
	{
		NewProvider = EN2CLLMProvider::Anthropic;
	}
	else if (*NewValue == TEXT("Gemini"))
	{
		NewProvider = EN2CLLMProvider::Gemini;
	}
	else if (*NewValue == TEXT("DeepSeek"))
	{
		NewProvider = EN2CLLMProvider::DeepSeek;
	}
	else if (*NewValue == TEXT("Ollama"))
	{
		NewProvider = EN2CLLMProvider::Ollama;
	}
	else if (*NewValue == TEXT("LM Studio"))
	{
		NewProvider = EN2CLLMProvider::LMStudio;
	}

	// Check if provider actually changed
	if (Settings->Provider == NewProvider)
	{
		return;
	}

	Settings->Provider = NewProvider;
	CachedProvider = NewProvider;

	// Rebuild model selection widget (dropdown vs text input)
	RebuildModelSelectionWidget();

	// Save and notify
	SaveSettings();
	Settings->NotifyModelSettingsChanged();
}

void SN2CContextWindowVisualizer::OnModelSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	UN2CSettings* Settings = GetMutableDefault<UN2CSettings>();
	FN2CLLMModelRegistry& Registry = FN2CLLMModelRegistry::Get();
	const FString& SelectedDisplayName = *NewValue;

	// Find the model enum value by display name
	switch (Settings->Provider)
	{
	case EN2CLLMProvider::OpenAI:
		for (const auto& Pair : Registry.GetAllOpenAIModels())
		{
			if (Pair.Value.DisplayName == SelectedDisplayName)
			{
				Settings->OpenAI_Model = Pair.Key;
				break;
			}
		}
		break;
	case EN2CLLMProvider::Anthropic:
		for (const auto& Pair : Registry.GetAllAnthropicModels())
		{
			if (Pair.Value.DisplayName == SelectedDisplayName)
			{
				Settings->AnthropicModel = Pair.Key;
				break;
			}
		}
		break;
	case EN2CLLMProvider::Gemini:
		for (const auto& Pair : Registry.GetAllGeminiModels())
		{
			if (Pair.Value.DisplayName == SelectedDisplayName)
			{
				Settings->Gemini_Model = Pair.Key;
				break;
			}
		}
		break;
	case EN2CLLMProvider::DeepSeek:
		for (const auto& Pair : Registry.GetAllDeepSeekModels())
		{
			if (Pair.Value.DisplayName == SelectedDisplayName)
			{
				Settings->DeepSeekModel = Pair.Key;
				break;
			}
		}
		break;
	default:
		break;
	}

	// Save and notify
	SaveSettings();
	Settings->NotifyModelSettingsChanged();
}

void SN2CContextWindowVisualizer::OnLocalModelTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnCleared)
	{
		return;
	}

	UN2CSettings* Settings = GetMutableDefault<UN2CSettings>();
	const FString ModelName = NewText.ToString();

	switch (Settings->Provider)
	{
	case EN2CLLMProvider::Ollama:
		Settings->OllamaModel = ModelName;
		break;
	case EN2CLLMProvider::LMStudio:
		Settings->LMStudioModel = ModelName;
		break;
	default:
		break;
	}

	// Save and notify
	SaveSettings();
	Settings->NotifyModelSettingsChanged();
}

TSharedRef<SWidget> SN2CContextWindowVisualizer::GenerateProviderWidget(TSharedPtr<FString> Item)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*Item))
		.Margin(FMargin(4.0f, 2.0f));
}

TSharedRef<SWidget> SN2CContextWindowVisualizer::GenerateModelWidget(TSharedPtr<FString> Item)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*Item))
		.Margin(FMargin(4.0f, 2.0f));
}

FText SN2CContextWindowVisualizer::GetCurrentProviderText() const
{
	const UN2CSettings* Settings = GetDefault<UN2CSettings>();

	switch (Settings->Provider)
	{
	case EN2CLLMProvider::OpenAI:
		return FText::FromString(TEXT("OpenAI"));
	case EN2CLLMProvider::Anthropic:
		return FText::FromString(TEXT("Anthropic"));
	case EN2CLLMProvider::Gemini:
		return FText::FromString(TEXT("Gemini"));
	case EN2CLLMProvider::DeepSeek:
		return FText::FromString(TEXT("DeepSeek"));
	case EN2CLLMProvider::Ollama:
		return FText::FromString(TEXT("Ollama"));
	case EN2CLLMProvider::LMStudio:
		return FText::FromString(TEXT("LM Studio"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FText SN2CContextWindowVisualizer::GetLocalModelText() const
{
	const UN2CSettings* Settings = GetDefault<UN2CSettings>();

	switch (Settings->Provider)
	{
	case EN2CLLMProvider::Ollama:
		return FText::FromString(Settings->OllamaModel);
	case EN2CLLMProvider::LMStudio:
		return FText::FromString(Settings->LMStudioModel);
	default:
		return FText::GetEmpty();
	}
}

bool SN2CContextWindowVisualizer::IsLocalProvider() const
{
	const UN2CSettings* Settings = GetDefault<UN2CSettings>();
	return Settings->Provider == EN2CLLMProvider::Ollama ||
		   Settings->Provider == EN2CLLMProvider::LMStudio;
}

FReply SN2CContextWindowVisualizer::OnSettingsClicked()
{
	// Open Project Settings to NodeToCode section
	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.ShowViewer("Project", "Plugins", "Node to Code");

	return FReply::Handled();
}

void SN2CContextWindowVisualizer::SaveSettings()
{
	UN2CSettings* Settings = GetMutableDefault<UN2CSettings>();
	const FString ConfigPath = Settings->GetDefaultConfigFilename();
	Settings->SaveConfig();
}

// ==================== Display Text Getters ====================

FText SN2CContextWindowVisualizer::GetTokenCountText() const
{
	if (SelectedGraphCount == 0)
	{
		return LOCTEXT("SelectGraphsHint", "Select graphs to estimate tokens");
	}

	// Format token count with commas
	FString TokensStr;
	{
		FString Result = FString::Printf(TEXT("%d"), TotalTokens);
		int32 InsertPosition = Result.Len() - 3;
		while (InsertPosition > 0)
		{
			Result.InsertAt(InsertPosition, TEXT(","));
			InsertPosition -= 3;
		}
		TokensStr = Result;
	}

	if (SelectedGraphCount == 1)
	{
		return FText::Format(
			LOCTEXT("TotalTokensSingle", "Total for selection: {0} tokens"),
			FText::FromString(TokensStr)
		);
	}

	return FText::Format(
		LOCTEXT("TotalTokensMultiple", "Total for {0} graphs: {1} tokens"),
		FText::AsNumber(SelectedGraphCount),
		FText::FromString(TokensStr)
	);
}

FText SN2CContextWindowVisualizer::GetEstimatedCostText() const
{
	FN2CTokenEstimationService& TokenService = FN2CTokenEstimationService::Get();

	if (TokenService.IsLocalProvider())
	{
		return LOCTEXT("LocalModelCost", "Estimated batch cost: Free (local model)");
	}

	if (TotalTokens == 0)
	{
		return LOCTEXT("NoCost", "Estimated batch cost: $0.00");
	}

	// Create formatting options for currency display
	FNumberFormattingOptions CostFormatOptions;
	CostFormatOptions.SetUseGrouping(true);
	CostFormatOptions.SetMinimumFractionalDigits(2);
	CostFormatOptions.SetMaximumFractionalDigits(4);

	return FText::Format(
		LOCTEXT("EstimatedCostFormat", "Estimated batch cost: ${0}"),
		FText::AsNumber(TotalCost, &CostFormatOptions)
	);
}

FText SN2CContextWindowVisualizer::GetModelNameText() const
{
	return FText::FromString(FN2CTokenEstimationService::Get().GetCurrentModelName());
}

#undef LOCTEXT_NAMESPACE
