// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "Core/Widgets/SN2CTranslationViewer.h"
#include "Code Editor/Widgets/SN2CCodeEditor.h"
#include "Core/N2CGraphStateManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateIconFinder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Core/N2CSettings.h"

#define LOCTEXT_NAMESPACE "SN2CTranslationViewer"

void SN2CTranslationViewer::Construct(const FArguments& InArgs)
{
	OnCloseRequestedDelegate = InArgs._OnCloseRequested;
	ActiveFileType = TEXT("h"); // Default to header file
	bHasTranslation = false;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(UIL(N2CUI().BgPanel))
		.Padding(0)
		[
			SNew(SVerticalBox)

			// Header with graph name and close button
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(UIL(N2CUI().BgPanelDarker))
				.Padding(FMargin(12.0f, 10.0f))
				[
					SNew(SHorizontalBox)
					// Graph name
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(GraphNameText, STextBlock)
						.Text(LOCTEXT("NoTranslation", "No Translation Loaded"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(UIL(N2CUI().TextPrimary))
					]
					// Close button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "NoBorder")
						.ContentPadding(FMargin(4.0f))
						.OnClicked(this, &SN2CTranslationViewer::HandleCloseClicked)
						.ToolTipText(LOCTEXT("CloseTooltip", "Close"))
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("\u2715"))) // ✕
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
							.ColorAndOpacity(UIL(N2CUI().TextMuted))
						]
					]
				]
			]

			// Toolbar with file tabs and copy button
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(UIL(N2CUI().BgPanel))
				.Padding(FMargin(12.0f, 8.0f))
				[
					SNew(SHorizontalBox)
					// File tabs
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 4.0f, 0.0f)
						[
							CreateFileTab(TEXT(".cpp"), TEXT("cpp"), CppTabButton)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.0f, 0.0f, 4.0f, 0.0f)
						[
							CreateFileTab(TEXT(".h"), TEXT("h"), HeaderTabButton)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							CreateFileTab(TEXT("JSON"), TEXT("json"), JsonTabButton)
						]
					]
					// Spacer
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNullWidget::NullWidget
					]
					// Copy button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "Button")
						.ContentPadding(FMargin(6.0f, 6.0f))
						.OnClicked(this, &SN2CTranslationViewer::HandleCopyCodeClicked)
						.ToolTipText(LOCTEXT("CopyCodeTooltip", "Copy code to clipboard"))
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							[
								SNew(SBox)
								.WidthOverride(14.0f)
								.HeightOverride(14.0f)
								[
									SNew(SImage)
									.Image(FAppStyle::GetBrush("Icons.Clipboard"))
								]
							]
						]
					]
				]
			]

			// Code viewer
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.BorderBackgroundColor(UIL(N2CUI().BgPanelDarker))
				.Padding(0)
				[
					SAssignNew(CodeEditor, SN2CCodeEditor)
					.Language(EN2CCodeLanguage::Cpp)
					.ThemeName(TEXT("Unreal Engine"))
				]
			]

			// Notes section (collapsible)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(NotesSection, SExpandableArea)
				.AreaTitle(LOCTEXT("NotesHeader", "Translation Notes"))
				.InitiallyCollapsed(false)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(UIL(N2CUI().BgPanelDarker))
				.HeaderPadding(FMargin(12.0f, 8.0f))
				.Padding(FMargin(0.0f))
				.BodyContent()
				[
					SNew(SBox)
					.MaxDesiredHeight(150.0f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
						.BorderBackgroundColor(UIL(N2CUI().BgPanelDarker))
						.Padding(FMargin(16.0f, 12.0f))
						[
							SNew(SHorizontalBox)
							// Notes text
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									SAssignNew(NotesText, STextBlock)
									.Text(LOCTEXT("NoNotes", "No implementation notes available."))
									.ColorAndOpacity(UIL(N2CUI().TextSecondary))
									.AutoWrapText(true)
								]
							]
							// Copy notes button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Top)
							.Padding(8.0f, 0.0f, 0.0f, 0.0f)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "Button")
								.ContentPadding(FMargin(6.0f, 6.0f))
								.OnClicked(this, &SN2CTranslationViewer::HandleCopyNotesClicked)
								.ToolTipText(LOCTEXT("CopyNotesTooltip", "Copy notes to clipboard"))
								[
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									[
										SNew(SBox)
										.WidthOverride(14.0f)
										.HeightOverride(14.0f)
										[
											SNew(SImage)
											.Image(FAppStyle::GetBrush("Icons.Clipboard"))
										]
									]
								]
							]
						]
					]
				]
			]
		]
	];

	UpdateTabStyles();
}

TSharedRef<SWidget> SN2CTranslationViewer::CreateFileTab(const FString& Label, const FString& FileType, TSharedPtr<SButton>& OutButton)
{
	return SAssignNew(OutButton, SButton)
		.ButtonStyle(FAppStyle::Get(), "Button")
		.ContentPadding(FMargin(12.0f, 4.0f))
		.OnClicked(this, &SN2CTranslationViewer::HandleFileTabClicked, FileType)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Label))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(this, &SN2CTranslationViewer::GetTabTextColor, FileType)
		];
}

bool SN2CTranslationViewer::LoadTranslation(const FN2CTagInfo& GraphInfo)
{
	// Try to load translation from state manager
	FN2CGraphTranslation Translation;
	FGuid GraphGuid;
	if (!FGuid::Parse(GraphInfo.GraphGuid, GraphGuid))
	{
		return false;
	}

	if (UN2CGraphStateManager::Get().LoadTranslation(GraphGuid, Translation))
	{
		// Try to get JSON content as well
		FString JsonContent;
		UN2CGraphStateManager::Get().LoadN2CJson(GraphGuid, JsonContent);

		LoadTranslation(Translation, GraphInfo.GraphName, JsonContent);
		return true;
	}

	return false;
}

void SN2CTranslationViewer::LoadTranslation(const FN2CGraphTranslation& Translation, const FString& GraphName, const FString& JsonContent)
{
	CurrentTranslation = Translation;
	CurrentGraphName = GraphName;
	CurrentJsonContent = JsonContent;
	bHasTranslation = true;

	// Update graph name display
	if (GraphNameText.IsValid())
	{
		GraphNameText->SetText(FText::FromString(GraphName));
	}

	// Update notes
	if (NotesText.IsValid())
	{
		FString Notes = Translation.Code.ImplementationNotes;
		if (Notes.IsEmpty())
		{
			Notes = TEXT("No implementation notes available.");
		}
		NotesText->SetText(FText::FromString(Notes));
	}

	// Update code display
	UpdateCodeDisplay();
}

void SN2CTranslationViewer::SetJsonContent(const FString& JsonContent, const FString& GraphName)
{
	CurrentJsonContent = JsonContent;
	CurrentGraphName = GraphName;

	// Clear translation data
	CurrentTranslation = FN2CGraphTranslation();
	bHasTranslation = false;

	// Update graph name display
	if (GraphNameText.IsValid())
	{
		GraphNameText->SetText(FText::FromString(GraphName));
	}

	// Clear notes
	if (NotesText.IsValid())
	{
		NotesText->SetText(LOCTEXT("NoNotes", "No implementation notes available."));
	}

	// Switch to JSON tab
	ActiveFileType = TEXT("json");
	UpdateTabStyles();
	UpdateCodeDisplay();
}

void SN2CTranslationViewer::Clear()
{
	CurrentTranslation = FN2CGraphTranslation();
	CurrentGraphName = FString();
	CurrentJsonContent = FString();
	bHasTranslation = false;

	if (GraphNameText.IsValid())
	{
		GraphNameText->SetText(LOCTEXT("NoTranslation", "No Translation Loaded"));
	}

	if (NotesText.IsValid())
	{
		NotesText->SetText(LOCTEXT("NoNotes", "No implementation notes available."));
	}

	if (CodeEditor.IsValid())
	{
		CodeEditor->SetText(FText::GetEmpty());
	}
}

FReply SN2CTranslationViewer::HandleFileTabClicked(FString FileType)
{
	if (ActiveFileType != FileType)
	{
		ActiveFileType = FileType;
		UpdateTabStyles();
		UpdateCodeDisplay();
	}
	return FReply::Handled();
}

FReply SN2CTranslationViewer::HandleCloseClicked()
{
	OnCloseRequestedDelegate.ExecuteIfBound();
	return FReply::Handled();
}

FReply SN2CTranslationViewer::HandleCopyCodeClicked()
{
	FString Content = GetContentForActiveTab();
	if (!Content.IsEmpty())
	{
		FPlatformApplicationMisc::ClipboardCopy(*Content);
	}
	return FReply::Handled();
}

FReply SN2CTranslationViewer::HandleCopyNotesClicked()
{
	if (!CurrentTranslation.Code.ImplementationNotes.IsEmpty())
	{
		FPlatformApplicationMisc::ClipboardCopy(*CurrentTranslation.Code.ImplementationNotes);
	}
	return FReply::Handled();
}

void SN2CTranslationViewer::UpdateCodeDisplay()
{
	if (!CodeEditor.IsValid())
	{
		return;
	}

	FString Content = GetContentForActiveTab();

	// Set appropriate language for syntax highlighting
	if (ActiveFileType == TEXT("json"))
	{
		// Use Pseudocode for JSON since we don't have dedicated JSON highlighting
		// This provides reasonable highlighting for structured text
		CodeEditor->SetLanguage(EN2CCodeLanguage::Pseudocode);
	}
	else
	{
		CodeEditor->SetLanguage(EN2CCodeLanguage::Cpp);
	}

	CodeEditor->SetText(FText::FromString(Content));
}

void SN2CTranslationViewer::UpdateTabStyles()
{
	// Tab styles are handled via GetTabTextColor binding
	// Force a refresh by invalidating the widget
	if (CppTabButton.IsValid())
	{
		CppTabButton->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (HeaderTabButton.IsValid())
	{
		HeaderTabButton->Invalidate(EInvalidateWidgetReason::Paint);
	}
	if (JsonTabButton.IsValid())
	{
		JsonTabButton->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

FString SN2CTranslationViewer::GetContentForActiveTab() const
{
	if (ActiveFileType == TEXT("cpp"))
	{
		return CurrentTranslation.Code.GraphImplementation;
	}
	else if (ActiveFileType == TEXT("h"))
	{
		return CurrentTranslation.Code.GraphDeclaration;
	}
	else if (ActiveFileType == TEXT("json"))
	{
		// Prettify the JSON for display (don't modify the stored content)
		if (!CurrentJsonContent.IsEmpty())
		{
			TSharedPtr<FJsonValue> JsonValue;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CurrentJsonContent);
			if (FJsonSerializer::Deserialize(Reader, JsonValue) && JsonValue.IsValid())
			{
				FString PrettyJson;
				TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
					TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&PrettyJson);
				if (FJsonSerializer::Serialize(JsonValue, TEXT(""), Writer))
				{
					return PrettyJson;
				}
			}
		}
		// Fall back to raw content if prettifying fails
		return CurrentJsonContent;
	}
	return FString();
}

const FButtonStyle* SN2CTranslationViewer::GetTabButtonStyle(bool bIsActive) const
{
	return bIsActive
		? &FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton")
		: &FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
}

FSlateColor SN2CTranslationViewer::GetTabTextColor(FString FileType) const
{
	if (FileType == ActiveFileType)
	{
		return UIL(N2CUI().AccentOrange);
	}
	return UIL(N2CUI().TextSecondary);
}

FSlateColor SN2CTranslationViewer::GetTabBorderColor(FString FileType) const
{
	if (FileType == ActiveFileType)
	{
		return UIL(N2CUI().AccentOrange);
	}
	return UIL(N2CUI().BorderColor);
}

#undef LOCTEXT_NAMESPACE
