// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#pragma once

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateTypes.h"

class N2CStyle : public FSlateStyleSet
{
public:
    static void Initialize();
    static void Shutdown();
    static void Reinitialize();
    static const ISlateStyle& Get();
    virtual const FName& GetStyleSetName() const override;

    // Add helper method for content paths
    static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

    // Button style accessors — return stable pointers for live SButton updates
    static const FButtonStyle& GetButtonStyle();
    static const FButtonStyle& GetSimpleButtonStyle();
    static const FButtonStyle& GetNoBorderStyle();

    // Update button styles in-place from current palette (called on settings change)
    static void UpdateButtonStyles();

private:
    static TSharedRef<FSlateStyleSet> Create();
    static TSharedPtr<FSlateStyleSet> StyleInstance;

    // Static FButtonStyle instances with stable addresses so SButton pointers remain valid
    static FButtonStyle N2CButtonStyle;
    static FButtonStyle N2CSimpleButtonStyle;
    static FButtonStyle N2CNoBorderStyle;
};

// Add macro for plugin brushes
#define N2C_PLUGIN_BRUSH(RelativePath,...) FSlateImageBrush(N2CStyle::InContent(RelativePath,".png"),__VA_ARGS__)
