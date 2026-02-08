// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "N2CStyle.h"
#include "Core/N2CSettings.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Framework/Application/SlateApplication.h"

TSharedPtr<FSlateStyleSet> N2CStyle::StyleInstance = nullptr;

// Static FButtonStyle instances — stable addresses allow SButton to see live updates
FButtonStyle N2CStyle::N2CButtonStyle;
FButtonStyle N2CStyle::N2CSimpleButtonStyle;
FButtonStyle N2CStyle::N2CNoBorderStyle;

// Static FSlateBrush instances — stable addresses allow SBorder to see live updates
FSlateBrush N2CStyle::N2CPanelBorderBrush;
FSlateBrush N2CStyle::N2CDarkPanelBorderBrush;

// Static combo box styles — stable addresses allow SComboBox to see live updates
FComboBoxStyle N2CStyle::N2CComboBoxStyle;
FTableRowStyle N2CStyle::N2CComboRowStyle;

void N2CStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance.Get());
	}
}

void N2CStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance.Get());
		StyleInstance.Reset();
	}
}

void N2CStyle::Reinitialize()
{
	Shutdown();
	Initialize();
}

const ISlateStyle& N2CStyle::Get()
{
	if (!StyleInstance.IsValid())
	{
		Initialize();
	}
	return *StyleInstance.Get();
}

FString N2CStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
    static FString Content = IPluginManager::Get().FindPlugin(TEXT("NodeToCode"))->GetBaseDir() / TEXT("Resources");
    return (Content / RelativePath) + Extension;
}

const FName& N2CStyle::GetStyleSetName() const
{
    static FName StyleName(TEXT("NodeToCodeStyle"));
    return StyleName;
}

const FButtonStyle& N2CStyle::GetButtonStyle() { return N2CButtonStyle; }
const FButtonStyle& N2CStyle::GetSimpleButtonStyle() { return N2CSimpleButtonStyle; }
const FButtonStyle& N2CStyle::GetNoBorderStyle() { return N2CNoBorderStyle; }
const FSlateBrush& N2CStyle::GetPanelBorderBrush() { return N2CPanelBorderBrush; }
const FSlateBrush& N2CStyle::GetDarkPanelBorderBrush() { return N2CDarkPanelBorderBrush; }
const FComboBoxStyle& N2CStyle::GetComboBoxStyle() { return N2CComboBoxStyle; }
const FTableRowStyle& N2CStyle::GetComboRowStyle() { return N2CComboRowStyle; }

TSharedRef<FSlateStyleSet> N2CStyle::Create()
{
    static FName StyleSetName(TEXT("NodeToCodeStyle"));
    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(StyleSetName));

    // Set content root using plugin manager
    Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("NodeToCode"))->GetBaseDir() / TEXT("Resources"));

    Style->Set("NodeToCode.ToolbarButton",
        new N2C_PLUGIN_BRUSH(TEXT("button_icon"), FVector2D(128.0f, 128.0f)));

    // Populate styles from current palette
    UpdateButtonStyles();
    UpdateBorderBrushes();
    UpdateComboBoxStyle();

    return Style;
}

void N2CStyle::UpdateButtonStyles()
{
	const FN2CUIColors& Colors = UN2CSettings::GetUIColors();

	const FSlateColor NormalFg(FN2CUIColors::ToLinear(Colors.BtnForeground));
	const FSlateColor HoveredFg(FN2CUIColors::ToLinear(Colors.BtnForegroundHover));
	const FSlateColor DisabledFg(FN2CUIColors::ToLinear(Colors.BtnForegroundDisabled));

	// ── N2C.Button ── Solid background action buttons
	N2CButtonStyle.SetNormal(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnNormal), 4.0f));
	N2CButtonStyle.SetHovered(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnHovered), 4.0f));
	N2CButtonStyle.SetPressed(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnPressed), 4.0f));
	N2CButtonStyle.SetDisabled(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnDisabled), 4.0f));
	N2CButtonStyle.SetNormalForeground(NormalFg);
	N2CButtonStyle.SetHoveredForeground(HoveredFg);
	N2CButtonStyle.SetPressedForeground(NormalFg);
	N2CButtonStyle.SetDisabledForeground(DisabledFg);
	N2CButtonStyle.SetNormalPadding(FMargin(4.0f, 2.0f));
	N2CButtonStyle.SetPressedPadding(FMargin(4.0f, 3.0f, 4.0f, 1.0f));

	// ── N2C.SimpleButton ── Transparent/minimal icon buttons
	N2CSimpleButtonStyle.SetNormal(FSlateNoResource());
	N2CSimpleButtonStyle.SetHovered(FSlateRoundedBoxBrush(FN2CUIColors::ToLinearWithAlpha(Colors.BtnHovered, 0.5f), 4.0f));
	N2CSimpleButtonStyle.SetPressed(FSlateRoundedBoxBrush(FN2CUIColors::ToLinearWithAlpha(Colors.BtnPressed, 0.6f), 4.0f));
	N2CSimpleButtonStyle.SetDisabled(FSlateNoResource());
	N2CSimpleButtonStyle.SetNormalForeground(NormalFg);
	N2CSimpleButtonStyle.SetHoveredForeground(HoveredFg);
	N2CSimpleButtonStyle.SetPressedForeground(NormalFg);
	N2CSimpleButtonStyle.SetDisabledForeground(DisabledFg);
	N2CSimpleButtonStyle.SetNormalPadding(FMargin(0.0f));
	N2CSimpleButtonStyle.SetPressedPadding(FMargin(0.0f));

	// ── N2C.NoBorder ── Completely invisible button for wrappers/close
	N2CNoBorderStyle.SetNormal(FSlateNoResource());
	N2CNoBorderStyle.SetHovered(FSlateNoResource());
	N2CNoBorderStyle.SetPressed(FSlateNoResource());
	N2CNoBorderStyle.SetDisabled(FSlateNoResource());
	N2CNoBorderStyle.SetNormalForeground(NormalFg);
	N2CNoBorderStyle.SetHoveredForeground(HoveredFg);
	N2CNoBorderStyle.SetPressedForeground(NormalFg);
	N2CNoBorderStyle.SetDisabledForeground(DisabledFg);
	N2CNoBorderStyle.SetNormalPadding(FMargin(0.0f));
	N2CNoBorderStyle.SetPressedPadding(FMargin(0.0f));
}

void N2CStyle::UpdateBorderBrushes()
{
	const FN2CUIColors& Colors = UN2CSettings::GetUIColors();

	// Panel border: rounded box with subtle outline, fill tinted by BorderBackgroundColor at widget level
	N2CPanelBorderBrush = FSlateRoundedBoxBrush(
		FLinearColor::White, 4.0f,
		FN2CUIColors::ToLinear(Colors.BorderSubtle), 1.0f
	);

	// Dark panel border: same shape, darker context
	N2CDarkPanelBorderBrush = FSlateRoundedBoxBrush(
		FLinearColor::White, 4.0f,
		FN2CUIColors::ToLinear(Colors.BorderSubtle), 1.0f
	);
}

void N2CStyle::UpdateComboBoxStyle()
{
	const FN2CUIColors& Colors = UN2CSettings::GetUIColors();

	const FSlateColor NormalFg(FN2CUIColors::ToLinear(Colors.BtnForeground));
	const FSlateColor HoveredFg(FN2CUIColors::ToLinear(Colors.BtnForegroundHover));
	const FSlateColor DisabledFg(FN2CUIColors::ToLinear(Colors.BtnForegroundDisabled));

	// ── Combo button style ──
	FButtonStyle ComboBtn;
	ComboBtn.SetNormal(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BgInput), 4.0f));
	ComboBtn.SetHovered(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BgHover), 4.0f));
	ComboBtn.SetPressed(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnPressed), 4.0f));
	ComboBtn.SetDisabled(FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnDisabled), 4.0f));
	ComboBtn.SetNormalForeground(NormalFg);
	ComboBtn.SetHoveredForeground(HoveredFg);
	ComboBtn.SetPressedForeground(NormalFg);
	ComboBtn.SetDisabledForeground(DisabledFg);
	ComboBtn.SetNormalPadding(FMargin(4.0f, 2.0f));
	ComboBtn.SetPressedPadding(FMargin(4.0f, 3.0f, 4.0f, 1.0f));

	N2CComboBoxStyle.ComboButtonStyle.SetButtonStyle(ComboBtn);

	// Copy the engine's registered down arrow brush and tint with our foreground color
	const FComboButtonStyle& EngineComboBtn = FAppStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton");
	FSlateBrush ArrowBrush = EngineComboBtn.DownArrowImage;
	ArrowBrush.TintColor = NormalFg;
	N2CComboBoxStyle.ComboButtonStyle.SetDownArrowImage(ArrowBrush);
	N2CComboBoxStyle.ComboButtonStyle.SetDownArrowPadding(FMargin(2.0f, 0.0f));

	N2CComboBoxStyle.ComboButtonStyle.SetMenuBorderBrush(
		FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BgPanelDarker), 4.0f,
			FN2CUIColors::ToLinear(Colors.BorderColor), 1.0f));
	N2CComboBoxStyle.ComboButtonStyle.SetMenuBorderPadding(FMargin(2.0f));
	N2CComboBoxStyle.SetContentPadding(FMargin(4.0f, 2.0f));
	N2CComboBoxStyle.SetMenuRowPadding(FMargin(2.0f));

	// ── Dropdown row style ──
	const FSlateBrush TransparentBrush = FSlateNoResource();
	const FSlateBrush HoverBrush = FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BgHover), 4.0f);
	const FSlateBrush ActiveBrush = FSlateRoundedBoxBrush(FN2CUIColors::ToLinear(Colors.BtnPressed), 4.0f);

	N2CComboRowStyle.SetEvenRowBackgroundBrush(TransparentBrush);
	N2CComboRowStyle.SetOddRowBackgroundBrush(TransparentBrush);
	N2CComboRowStyle.SetEvenRowBackgroundHoveredBrush(HoverBrush);
	N2CComboRowStyle.SetOddRowBackgroundHoveredBrush(HoverBrush);
	N2CComboRowStyle.SetActiveBrush(ActiveBrush);
	N2CComboRowStyle.SetActiveHoveredBrush(HoverBrush);
	N2CComboRowStyle.SetInactiveBrush(ActiveBrush);
	N2CComboRowStyle.SetInactiveHoveredBrush(HoverBrush);
	N2CComboRowStyle.SetTextColor(FSlateColor(FN2CUIColors::ToLinear(Colors.TextPrimary)));
	N2CComboRowStyle.SetSelectedTextColor(FSlateColor(FN2CUIColors::ToLinear(Colors.TextPrimary)));
}
