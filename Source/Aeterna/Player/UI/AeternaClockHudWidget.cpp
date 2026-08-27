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
	LastShownClockMinutes = InClockMinutes;

	// 카운트다운 중에는 클럭이 표시를 덮지 않습니다.
	if (bCountdownActive || !ClockTextWidget)
	{
		return;
	}

	ClockTextWidget->SetText(BuildClockText(InClockMinutes));
}

void UAeternaClockHudWidget::SetCountdownSeconds(float RemainingSeconds)
{
	if (!ClockTextWidget)
	{
		return;
	}

	if (!bCountdownActive)
	{
		bCountdownActive = true;
		ClockTextWidget->SetColorAndOpacity(FSlateColor(CountdownDigitColor));
	}

	ClockTextWidget->SetText(BuildCountdownText(RemainingSeconds));
}

void UAeternaClockHudWidget::ClearCountdown()
{
	if (!bCountdownActive)
	{
		return;
	}

	bCountdownActive = false;

	if (ClockTextWidget)
	{
		ClockTextWidget->SetColorAndOpacity(FSlateColor(DigitColor));
		ClockTextWidget->SetText(BuildClockText(LastShownClockMinutes));
	}
}

FText UAeternaClockHudWidget::BuildCountdownText(float RemainingSeconds) const
{
	// 0.4초가 남았는데 00:00으로 보이면 이미 끝난 것처럼 읽힙니다. 올림으로 표시합니다.
	const int32 TotalSeconds = FMath::Max(0, FMath::CeilToInt(RemainingSeconds));
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), TotalSeconds / 60, TotalSeconds % 60));
}

FText UAeternaClockHudWidget::BuildClockText(int32 InClockMinutes) const
{
	const int32 ClampedMinutes = FMath::Clamp(InClockMinutes, 0, 6 * 60);
	const int32 Hours = ClampedMinutes / 60;
	const int32 Minutes = ClampedMinutes % 60;

	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Hours, Minutes));
}
