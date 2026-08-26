// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaBatteryHudWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UAeternaBatteryHudWidget::RebuildWidget()
{
	SegmentWidgets.Reset();

	TSharedRef<SHorizontalBox> SegmentBox = SNew(SHorizontalBox);
	const int32 SafeSegmentCount = FMath::Max(SegmentCount, 1);

	for (int32 Index = 0; Index < SafeSegmentCount; ++Index)
	{
		TSharedPtr<SBorder> SegmentWidget;
		SegmentBox->AddSlot()
			.FillWidth(1.0f)
			.Padding(FMargin(1.5f, 0.0f))
			[
				SAssignNew(SegmentWidget, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(EmptySegmentColor))
			];

		SegmentWidgets.Add(SegmentWidget);
	}

	return SAssignNew(RootSizeBox, SBox)
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
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(22.0f, 14.0f, 22.0f, 14.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SAssignNew(LabelTextWidget, STextBlock)
						.Text(LabelText)
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), 12))
						.ColorAndOpacity(FSlateColor(TextColor))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(PercentTextWidget, STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), 12))
						.ColorAndOpacity(FSlateColor(TextColor))
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FSlateColor(BorderColor))
					.Padding(FMargin(2.0f))
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FSlateColor(BackgroundColor))
						.Padding(FMargin(4.0f))
						[
							SegmentBox
						]
					]
				]
			]
		];
}

void UAeternaBatteryHudWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetBattery(1.0f, 1.0f, 1.0f);
}

void UAeternaBatteryHudWidget::SetBattery(float Current, float Max, float Normalized)
{
	const float BatteryPercent = FMath::Clamp(Normalized, 0.0f, 1.0f);
	const int32 FilledSegments = FMath::CeilToInt(BatteryPercent * static_cast<float>(SegmentWidgets.Num()));
	FLinearColor FillColor = GetFillColor(BatteryPercent);
	FillColor.A = 1.0f;

	for (int32 Index = 0; Index < SegmentWidgets.Num(); ++Index)
	{
		if (TSharedPtr<SBorder> SegmentWidget = SegmentWidgets[Index])
		{
			const FLinearColor SegmentColor = Index < FilledSegments ? FillColor : EmptySegmentColor;
			SegmentWidget->SetBorderBackgroundColor(FSlateColor(SegmentColor));
		}
	}

	if (LabelTextWidget)
	{
		LabelTextWidget->SetText(LabelText);
		LabelTextWidget->SetColorAndOpacity(FSlateColor(TextColor));
	}

	if (PercentTextWidget)
	{
		const int32 RoundedPercent = FMath::RoundToInt(BatteryPercent * 100.0f);
		PercentTextWidget->SetText(FText::Format(NSLOCTEXT("Aeterna", "NativeBatteryHudPercent", "{0}%"), FText::AsNumber(RoundedPercent)));
		PercentTextWidget->SetColorAndOpacity(FSlateColor(TextColor));
	}
}

void UAeternaBatteryHudWidget::SetBatteryLabel(const FText& InLabelText)
{
	LabelText = InLabelText;
	if (LabelTextWidget)
	{
		LabelTextWidget->SetText(LabelText);
	}
}

void UAeternaBatteryHudWidget::SetWidgetSize(FVector2D InWidgetSize)
{
	WidgetSize = FVector2D(FMath::Max(1.0f, InWidgetSize.X), FMath::Max(1.0f, InWidgetSize.Y));
	if (RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(WidgetSize.X);
		RootSizeBox->SetHeightOverride(WidgetSize.Y);
	}
}

FLinearColor UAeternaBatteryHudWidget::GetFillColor(float Normalized) const
{
	const float BatteryAlpha = FMath::Clamp(Normalized, 0.0f, 1.0f);
	const float WeightedAlpha = FMath::Pow(BatteryAlpha, BatteryColorExponent);
	FLinearColor FillColor = FMath::Lerp(LowBatteryColor, FullBatteryColor, WeightedAlpha);
	FillColor.A = 1.0f;
	return FillColor;
}
