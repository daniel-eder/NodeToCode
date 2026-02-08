// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "N2CStyle.h"
#include "Core/N2CSettings.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Framework/Application/SlateApplication.h"

TSharedPtr<FSlateStyleSet> N2CStyle::StyleInstance = nullptr;

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

TSharedRef<FSlateStyleSet> N2CStyle::Create()
{
    static FName StyleSetName(TEXT("NodeToCodeStyle"));
    TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(StyleSetName));

    // Set content root using plugin manager
    Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("NodeToCode"))->GetBaseDir() / TEXT("Resources"));

    Style->Set("NodeToCode.ToolbarButton",
        new N2C_PLUGIN_BRUSH(TEXT("button_icon"), FVector2D(128.0f, 128.0f)));

    // Register button styles from UI palette
    InitializeButtonStyles(*Style);

    return Style;
}

void N2CStyle::InitializeButtonStyles(FSlateStyleSet& Style)
{
	const FN2CUIColors& Colors = UN2CSettings::GetUIColors();

	const FSlateColor NormalFg(FN2CUIColors::ToLinear(Colors.BtnForeground));
	const FSlateColor HoveredFg(FN2CUIColors::ToLinear(Colors.BtnForegroundHover));
	const FSlateColor DisabledFg(FN2CUIColors::ToLinear(Colors.BtnForegroundDisabled));

	// ── N2C.Button ── Solid background action buttons (replaces FAppStyle "Button")
	{
		FButtonStyle ButtonStyle;
		ButtonStyle.SetNormal(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnNormal)));
		ButtonStyle.SetHovered(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnHovered)));
		ButtonStyle.SetPressed(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnPressed)));
		ButtonStyle.SetDisabled(FSlateColorBrush(FN2CUIColors::ToLinear(Colors.BtnDisabled)));
		ButtonStyle.SetNormalForeground(NormalFg);
		ButtonStyle.SetHoveredForeground(HoveredFg);
		ButtonStyle.SetPressedForeground(NormalFg);
		ButtonStyle.SetDisabledForeground(DisabledFg);
		ButtonStyle.SetNormalPadding(FMargin(4.0f, 2.0f));
		ButtonStyle.SetPressedPadding(FMargin(4.0f, 3.0f, 4.0f, 1.0f));
		Style.Set("N2C.Button", ButtonStyle);
	}

	// ── N2C.SimpleButton ── Transparent/minimal icon buttons (replaces FAppStyle "SimpleButton")
	{
		FButtonStyle SimpleStyle;
		SimpleStyle.SetNormal(FSlateNoResource());
		SimpleStyle.SetHovered(FSlateColorBrush(FN2CUIColors::ToLinearWithAlpha(Colors.BtnHovered, 0.5f)));
		SimpleStyle.SetPressed(FSlateColorBrush(FN2CUIColors::ToLinearWithAlpha(Colors.BtnPressed, 0.6f)));
		SimpleStyle.SetDisabled(FSlateNoResource());
		SimpleStyle.SetNormalForeground(NormalFg);
		SimpleStyle.SetHoveredForeground(HoveredFg);
		SimpleStyle.SetPressedForeground(NormalFg);
		SimpleStyle.SetDisabledForeground(DisabledFg);
		SimpleStyle.SetNormalPadding(FMargin(0.0f));
		SimpleStyle.SetPressedPadding(FMargin(0.0f));
		Style.Set("N2C.SimpleButton", SimpleStyle);
	}

	// ── N2C.NoBorder ── Completely invisible button for wrappers/close (replaces FAppStyle "NoBorder")
	{
		FButtonStyle NoBorderStyle;
		NoBorderStyle.SetNormal(FSlateNoResource());
		NoBorderStyle.SetHovered(FSlateNoResource());
		NoBorderStyle.SetPressed(FSlateNoResource());
		NoBorderStyle.SetDisabled(FSlateNoResource());
		NoBorderStyle.SetNormalForeground(NormalFg);
		NoBorderStyle.SetHoveredForeground(HoveredFg);
		NoBorderStyle.SetPressedForeground(NormalFg);
		NoBorderStyle.SetDisabledForeground(DisabledFg);
		NoBorderStyle.SetNormalPadding(FMargin(0.0f));
		NoBorderStyle.SetPressedPadding(FMargin(0.0f));
		Style.Set("N2C.NoBorder", NoBorderStyle);
	}
}
