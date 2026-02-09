// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * Button style variant — maps to N2CStyle button styles.
 */
enum class EN2CButtonVariant : uint8
{
	/** Solid background action button (N2CStyle::GetButtonStyle) */
	Standard,
	/** Flat/minimal button (N2CStyle::GetSimpleButtonStyle) */
	Simple,
	/** Completely invisible wrapper (N2CStyle::GetNoBorderStyle) */
	NoBorder
};

/**
 * Reusable icon+text button atom.
 * Replaces 20+ inline SButton + SImage + STextBlock patterns.
 *
 * Supports three modes:
 * - Icon + Text: both IconBrush and Text set
 * - Icon only:   only IconBrush set
 * - Text only:   only Text set
 *
 * Usage:
 *   SNew(SN2CIconButton)
 *   .IconBrush(FAppStyle::GetBrush("Icons.Convert"))
 *   .Text(LOCTEXT("Translate", "Translate"))
 *   .ButtonVariant(EN2CButtonVariant::Standard)
 *   .OnClicked(this, &MyClass::HandleClick)
 */
class NODETOCODE_API SN2CIconButton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SN2CIconButton)
		: _IconBrush(nullptr)
		, _IconSize(FVector2D(16.0f, 16.0f))
		, _ButtonVariant(EN2CButtonVariant::Standard)
		, _ContentPadding(FMargin(8.0f, 4.0f))
		, _IsEnabled(true)
		, _IconColor(FSlateColor::UseForeground())
		, _TextColor(FSlateColor::UseForeground())
	{}
		/** Icon brush (nullptr = text-only button) */
		SLATE_ARGUMENT(const FSlateBrush*, IconBrush)
		/** Icon size override */
		SLATE_ARGUMENT(FVector2D, IconSize)
		/** Optional text label (empty = icon-only button) */
		SLATE_ATTRIBUTE(FText, Text)
		/** Tooltip text */
		SLATE_ATTRIBUTE(FText, ToolTipText)
		/** Button style variant */
		SLATE_ARGUMENT(EN2CButtonVariant, ButtonVariant)
		/** Inner padding */
		SLATE_ARGUMENT(FMargin, ContentPadding)
		/** Click handler */
		SLATE_EVENT(FOnClicked, OnClicked)
		/** Is enabled */
		SLATE_ATTRIBUTE(bool, IsEnabled)
		/** Visibility */
		SLATE_ATTRIBUTE(EVisibility, Visibility)
		/** Optional icon color/tint override */
		SLATE_ATTRIBUTE(FSlateColor, IconColor)
		/** Optional font override (default: uses Button variant's standard font) */
		SLATE_ARGUMENT(TOptional<FSlateFontInfo>, Font)
		/** Optional text color override */
		SLATE_ATTRIBUTE(FSlateColor, TextColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
