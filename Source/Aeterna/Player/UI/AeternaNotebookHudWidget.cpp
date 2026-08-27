// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaNotebookHudWidget.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UAeternaNotebookHudWidget::RebuildWidget()
{
	TSharedRef<SWidget> IconWidget = BuildFallbackIcon();
	if (IconTexture)
	{
		IconBrush.SetResourceObject(IconTexture);
		IconBrush.ImageSize = FVector2D(48.0f, 48.0f);
		IconWidget = SNew(SImage)
			.Image(&IconBrush)
			.ColorAndOpacity(FSlateColor(IconTint));
	}

	return SAssignNew(RootSizeBox, SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(6.0f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(PanelColor))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SBox)
				.WidthOverride(36.0f)
				.HeightOverride(2.0f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FSlateColor(BorderColor))
				]
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			[
				SNew(SBox)
				.WidthOverride(36.0f)
				.HeightOverride(2.0f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FSlateColor(BorderColor))
				]
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				IconWidget
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0.0f, 0.0f, 12.0f, 10.0f))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("Aeterna", "NotebookHudTabHint", "TAB"))
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 12))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		];
}

void UAeternaNotebookHudWidget::SetNotebookIconTexture(UTexture2D* InIconTexture)
{
	IconTexture = InIconTexture;
}

void UAeternaNotebookHudWidget::SetWidgetSize(FVector2D InWidgetSize)
{
	WidgetSize = FVector2D(FMath::Max(1.0f, InWidgetSize.X), FMath::Max(1.0f, InWidgetSize.Y));
	if (RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(WidgetSize.X);
		RootSizeBox->SetHeightOverride(WidgetSize.Y);
	}
}

TSharedRef<SWidget> UAeternaNotebookHudWidget::BuildFallbackIcon() const
{
	return SNew(STextBlock)
		.Text(NSLOCTEXT("Aeterna", "NotebookHudFallbackIcon", "N"))
		.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 32))
		.ColorAndOpacity(FSlateColor(IconTint));
}
