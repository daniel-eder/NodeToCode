// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#include "N2CCodeEditorStyle.h"

#include "N2CCodeLanguage.h"
#include "Core/N2CSettings.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "N2CCodeEditorStyle"

TSharedPtr<FSlateStyleSet> FN2CCodeEditorStyle::StyleSet = nullptr;

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

void FN2CCodeEditorStyle::Initialize()
{
    // Ensure we don't double-initialize
    if (StyleSet.IsValid())
    {
        return;
    }

    // Ensure we're on the game thread
    check(IsInGameThread());

    StyleSet = MakeShareable(new FSlateStyleSet("N2CCodeEditor"));
    
    // Base editor style
    const FTextBlockStyle NormalText = CreateDefaultTextStyle(TEXT("CPP"));

    StyleSet->Set("N2CCodeEditor.TextEditor.NormalText", NormalText);
    const UN2CSettings* Settings = GetDefault<UN2CSettings>();
    if (Settings && Settings->CodeEditorTheme.Themes.Contains(TEXT("Unreal Engine")))
    {
        const FN2CCodeEditorColors& Colors = *Settings->CodeEditorTheme.Themes.Find(TEXT("Unreal Engine"));
        StyleSet->Set("N2CCodeEditor.TextEditor.Border", new FSlateColorBrush(FLinearColor(Colors.Background)));
    }
    else
    {
        // Fallback to a default color if themes aren't initialized yet
        StyleSet->Set("N2CCodeEditor.TextEditor.Border", new FSlateColorBrush(FN2CUIColors::ToLinear(UN2CSettings::GetUIColors().FallbackBorder)));
    }

    // Initialize language-specific styles
    InitializeLanguageStyles();

    FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FN2CCodeEditorStyle::InitializeLanguageStyles()
{
    const UN2CSettings* Settings = GetDefault<UN2CSettings>();
    const FN2CCodeEditorColors* Colors = Settings->GetThemeColors(EN2CCodeLanguage::Cpp, FName("Unreal Engine"));
    if (!Colors) return;

    // Register styles for each language using the same theme colors
    InitializeStylesForLanguage(TEXT("CPP"), *Colors);
    InitializeStylesForLanguage(TEXT("Python"), *Colors);
    InitializeStylesForLanguage(TEXT("JavaScript"), *Colors);
    InitializeStylesForLanguage(TEXT("CSharp"), *Colors);
    InitializeStylesForLanguage(TEXT("Swift"), *Colors);
    InitializeStylesForLanguage(TEXT("Pseudocode"), *Colors, true);
}

void FN2CCodeEditorStyle::InitializeStylesForLanguage(const FName& LanguageId, const FN2CCodeEditorColors& Colors, bool bIsPseudocode)
{
    const FTextBlockStyle BaseStyle = CreateDefaultTextStyle(LanguageId);
    const FName ThemeName = TEXT("Unreal Engine");

    // Set background color
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Background"), *LanguageId.ToString(), *ThemeName.ToString()),
        new FSlateColorBrush(FLinearColor(Colors.Background)));

    // For Pseudocode, all syntax elements use NormalText color
    const FLinearColor NormalColor(Colors.NormalText);

    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Normal"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(NormalColor));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Operator"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Operators)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Keyword"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Keywords)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.String"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Strings)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Number"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Numbers)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Comment"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Comments)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Preprocessor"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Preprocessor)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.Parentheses"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.Parentheses)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.CurlyBraces"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.CurlyBraces)));
    StyleSet->Set(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.SquareBrackets"), *LanguageId.ToString(), *ThemeName.ToString()),
        FTextBlockStyle(BaseStyle).SetColorAndOpacity(bIsPseudocode ? NormalColor : FLinearColor(Colors.SquareBrackets)));
}

FTextBlockStyle FN2CCodeEditorStyle::CreateDefaultTextStyle(const FName& TypeName)
{
    // Get current font size from settings if available, otherwise use default
    int32 FontSize = 12;

    return FTextBlockStyle()
        .SetFont(DEFAULT_FONT("Mono", FontSize))
        .SetColorAndOpacity(FSlateColor(FLinearColor::White))
        .SetShadowOffset(FVector2D::ZeroVector)
        .SetShadowColorAndOpacity(FN2CUIColors::ToLinear(UN2CSettings::GetUIColors().TextShadow))
        .SetHighlightColor(FN2CUIColors::ToLinear(UN2CSettings::GetUIColors().TextHighlight));
}

void FN2CCodeEditorStyle::Shutdown()
{
    if (StyleSet.IsValid())
    {
        // Ensure no references remain before unregistering
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
        StyleSet.Reset();
    }
}

void FN2CCodeEditorStyle::Reinitialize()
{
    Shutdown();
    Initialize();
}

const ISlateStyle& FN2CCodeEditorStyle::Get()
{
    check(StyleSet.IsValid());
    return *StyleSet.Get();
}

const FName& FN2CCodeEditorStyle::GetStyleSetName()
{
    check(StyleSet.IsValid());
    return StyleSet->GetStyleSetName();
}

const FTextBlockStyle& FN2CCodeEditorStyle::GetLanguageStyle(const FName& LanguageId, const FName& ThemeName, const FName& StyleId)
{
    check(StyleSet.IsValid());
    const FName FullStyleId = FName(*FString::Printf(TEXT("N2CCodeEditor.%s.%s.%s"), 
        *LanguageId.ToString(), *ThemeName.ToString(), *StyleId.ToString()));
    
    // Add fallback in case the style isn't found
    if (!StyleSet->HasWidgetStyle<FTextBlockStyle>(FullStyleId))
    {
        // Try the default theme
        const FName DefaultStyleId = FName(*FString::Printf(TEXT("N2CCodeEditor.%s.Unreal Engine.%s"), 
            *LanguageId.ToString(), *StyleId.ToString()));
            
        if (!StyleSet->HasWidgetStyle<FTextBlockStyle>(DefaultStyleId))
        {
            return StyleSet->GetWidgetStyle<FTextBlockStyle>("N2CCodeEditor.TextEditor.NormalText");
        }
        return StyleSet->GetWidgetStyle<FTextBlockStyle>(DefaultStyleId);
    }
    
    return StyleSet->GetWidgetStyle<FTextBlockStyle>(FullStyleId);
}

FString FN2CCodeEditorStyle::GetLanguageString(EN2CCodeLanguage Language)
{
    switch (Language)
    {
        case EN2CCodeLanguage::Cpp:
            return TEXT("CPP");
        case EN2CCodeLanguage::Python:
            return TEXT("Python");
        case EN2CCodeLanguage::JavaScript:
            return TEXT("JavaScript");
        case EN2CCodeLanguage::CSharp:
            return TEXT("CSharp");
        case EN2CCodeLanguage::Swift:
            return TEXT("Swift");
        case EN2CCodeLanguage::Pseudocode:
            return TEXT("Pseudocode");
        default:
            return TEXT("CPP");
    }
}

#undef DEFAULT_FONT
#undef LOCTEXT_NAMESPACE
