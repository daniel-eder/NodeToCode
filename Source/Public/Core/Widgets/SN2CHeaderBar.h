// Copyright (c) 2025 Nick McClure (Protospatial). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * Header bar molecule — dark panel with title text and optional close button.
 *
 * Usage:
 *   SNew(SN2CHeaderBar)
 *   .Title(LOCTEXT("MyHeader", "Translation Viewer"))
 *   .ShowCloseButton(true)
 *   .OnClose(this, &MyClass::HandleClose)
 */
class NODETOCODE_API SN2CHeaderBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SN2CHeaderBar)
		: _ShowCloseButton(true)
		, _Padding(FMargin(12.0f, 10.0f))
	{}
		/** Title text displayed in the header */
		SLATE_ATTRIBUTE(FText, Title)
		/** Whether to show the close (X) button on the right */
		SLATE_ARGUMENT(bool, ShowCloseButton)
		/** Close button click handler */
		SLATE_EVENT(FOnClicked, OnClose)
		/** Optional extra content to the right of the title (before close button) */
		SLATE_NAMED_SLOT(FArguments, RightContent)
		/** Inner padding */
		SLATE_ARGUMENT(FMargin, Padding)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update the title text after construction */
	void SetTitle(const FText& NewTitle);

private:
	TSharedPtr<STextBlock> TitleTextBlock;
};
