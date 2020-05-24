
#pragma once

#if WITH_EDITOR

#include "Misc/Paths.h"
#include "Widgets/SNullWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/CoreStyle.h"
#include "Application/SlateWindowHelper.h"
#include "SlateOptMacros.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SMultiLineEditableText.h"

#include "Engine/Classes/Engine/DataTable.h"

class SExportAnimWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SExportAnimWidget)
	{ }
	SLATE_END_ARGS()

	/**
		* Construct the widget
		*
		* @param InArgs   Declaration from which to construct the widget.
		*/
	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct(const FArguments& InDelcaration);

	END_SLATE_FUNCTION_BUILD_OPTIMIZATION


	virtual bool OnVisualizeTooltip(const TSharedPtr<SWidget>& TooltipContent) override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FReply OnExportAnimationToFloatTextureClick();
	FReply OnExportAnimationToCommonTextureClick();

	void RefreshUI();

	static SExportAnimWidget* Instance() 
	{
		return sInst;
	}
	static void DestroyInstance();

protected:
	TSharedPtr<SBorder> TooltipArea;
	TSharedPtr<SMultiLineEditableText> txtSelectAnimation;
	TSharedPtr<SEditableTextBox> inputTexWidth;
	TSharedPtr<SEditableTextBox> inputTexHeight;
	TSharedPtr<SEditableTextBox> inputExportFilePath;
	double time = 0, timeAccum = 0;
	static SExportAnimWidget* sInst;
	static FDelegateHandle sDelegateHandle;
};

#endif