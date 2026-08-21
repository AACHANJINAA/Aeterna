// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaClockHudWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UAeternaClockHudWidget::RebuildWidget()
{
	return SNew(SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(10.0f, 8.0f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(PanelColor))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(72.0f)
					.HeightOverride(2.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FSlateColor(BorderColor))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
				[
					SNew(SBox)
					.WidthOverride(18.0f)
					.HeightOverride(2.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FSlateColor(AccentColor))
					]
				]
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(18.0f)
					.HeightOverride(2.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FSlateColor(AccentColor))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
				[
					SNew(SBox)
					.WidthOverride(72.0f)
					.HeightOverride(2.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FSlateColor(BorderColor))
					]
				]
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(ClockTextWidget, STextBlock)
				.Text(FText::FromString(TEXT("00:00")))
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), DigitFontSize))
				.ColorAndOpacity(FSlateColor(DigitColor))
			]
		];
}

void UAeternaClockHudWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetClockMinutes(0);
}

void UAeternaClockHudWidget::SetClockMinutes(int32 InClockMinutes)
{
	if (ClockTextWidget)
	{
		ClockTextWidget->SetText(BuildClockText(InClockMinutes));
	}
}

FText UAeternaClockHudWidget::BuildClockText(int32 InClockMinutes) const
{
	const int32 ClampedMinutes = FMath::Clamp(InClockMinutes, 0, 6 * 60);
	const int32 Hours = ClampedMinutes / 60;
	const int32 Minutes = ClampedMinutes % 60;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Hours, Minutes));
}
