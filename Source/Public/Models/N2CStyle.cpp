// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "N2CStyle.h"
#include "Core/N2CSettings.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"

TSharedPtr<FSlateStyleSet> N2CStyle::StyleInstance = nullptr;

// Static FButtonStyle instances — stable addresses allow SButton to see live updates
FButtonStyle N2CStyle::N2CButtonStyle;
FButtonStyle N2CStyle::N2CSimpleButtonStyle;
FButtonStyle N2CStyle::N2CNoBorderStyle;

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

TSharedRef<FSlateStyleSet> N2CStyle::Create()
{
    static FName StyleSetName(TEXT("NodeToCodeStyle"));
    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(StyleSetName));

    // Set content root using plugin manager
    Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("NodeToCode"))->GetBaseDir() / TEXT("Resources"));

    Style->Set("NodeToCode.ToolbarButton",
        new N2C_PLUGIN_BRUSH(TEXT("button_icon"), FVector2D(128.0f, 128.0f)));

    // Populate button styles from current palette
    UpdateButtonStyles();

    return Style;
}

void N2CStyle::UpdateButtonStyles()
{
	const FN2CUIColors& Colors = UN2CSettings::GetUIColors();

	const FSlateColor NormalFg(FN2CUIColors::ToLinear(Colors.BtnForeground));
	const FSlateColor HoveredFg(FN2CUIColors::ToLinear(Colors.BtnForegroundHover));
	const FSlateColor DisabledFg(FN2CUIColors::ToLinear(Colors.BtnForegroundDisabled));

	// ── N2C.Button ── Solid background action buttons
	N2CButtonStyle.SetNormal(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnNormal)));
	N2CButtonStyle.SetHovered(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnHovered)));
	N2CButtonStyle.SetPressed(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnPressed)));
	N2CButtonStyle.SetDisabled(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnDisabled)));
	N2CButtonStyle.SetNormalForeground(NormalFg);
	N2CButtonStyle.SetHoveredForeground(HoveredFg);
	N2CButtonStyle.SetPressedForeground(NormalFg);
	N2CButtonStyle.SetDisabledForeground(DisabledFg);
	N2CButtonStyle.SetNormalPadding(FMargin(4.0f, 2.0f));
	N2CButtonStyle.SetPressedPadding(FMargin(4.0f, 3.0f, 4.0f, 1.0f));

	// ── N2C.SimpleButton ── Transparent/minimal icon buttons
	N2CSimpleButtonStyle.SetNormal(FSlateNoResource());
	N2CSimpleButtonStyle.SetHovered(FSlateColorBrush(FN2CUIColors::ToLinearWithAlpha(Colors.BtnHovered, 0.5f)));
	N2CSimpleButtonStyle.SetPressed(FSlateColorBrush(FN2CUIColors::ToLinearWithAlpha(Colors.BtnPressed, 0.6f)));
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
