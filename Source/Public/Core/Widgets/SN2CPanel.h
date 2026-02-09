// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * Panel variant determines background color and border brush.
 */
enum class EN2CPanelVariant : uint8
{
	/** BgPanel + PanelBorderBrush (lighter panel) */
	Light,
	/** BgPanelDarker + DarkPanelBorderBrush */
	Dark,
	/** BgOverlayPanel with configurable alpha + PanelBorderBrush */
	Overlay
};

/**
 * Reusable styled border panel atom.
 * Replaces repeated SBorder + N2CStyle brush + UIBind color patterns.
 *
 * Usage:
 *   SNew(SN2CPanel)
 *   .Variant(EN2CPanelVariant::Dark)
 *   .Padding(FMargin(12.0f, 10.0f))
 *   [
 *       // child content
 *   ]
 */
class NODETOCODE_API SN2CPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SN2CPanel)
		: _Variant(EN2CPanelVariant::Light)
		, _Padding(8.0f)
		, _OverlayAlpha(0.85f)
	{}
		/** Panel style variant */
		SLATE_ARGUMENT(EN2CPanelVariant, Variant)
		/** Inner padding */
		SLATE_ARGUMENT(FMargin, Padding)
		/** Alpha for Overlay variant (ignored for Light/Dark) */
		SLATE_ARGUMENT(float, OverlayAlpha)
		/** Child content */
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
