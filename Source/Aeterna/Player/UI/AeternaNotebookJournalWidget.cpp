// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaNotebookJournalWidget.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UAeternaNotebookJournalWidget::RebuildWidget()
{
	TSharedRef<SWidget> BackgroundWidget =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FSlateColor(PanelColor));

	if (BackgroundTexture)
	{
		BackgroundBrush.SetResourceObject(BackgroundTexture);
		BackgroundBrush.ImageSize = WidgetSize;
		BackgroundWidget =
			SNew(SImage)
			.Image(&BackgroundBrush)
			.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.92f)));
	}

	return SAssignNew(RootSizeBox, SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				BackgroundWidget
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(72.0f, 64.0f, 72.0f, 64.0f))
			[
				SAssignNew(NotebookTextWidget, STextBlock)
				.Text(NotebookText)
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 24))
				.ColorAndOpacity(FSlateColor(TextColor))
				.AutoWrapText(true)
			]
		];
}

void UAeternaNotebookJournalWidget::SetNotebookText(const FText& InNotebookText)
{
	NotebookText = InNotebookText;
	if (NotebookTextWidget)
	{
		NotebookTextWidget->SetText(NotebookText);
	}
}

void UAeternaNotebookJournalWidget::SetBackgroundTexture(UTexture2D* InBackgroundTexture)
{
	BackgroundTexture = InBackgroundTexture;
}

void UAeternaNotebookJournalWidget::SetWidgetSize(FVector2D InWidgetSize)
{
	WidgetSize = FVector2D(FMath::Max(1.0f, InWidgetSize.X), FMath::Max(1.0f, InWidgetSize.Y));
	if (RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(WidgetSize.X);
		RootSizeBox->SetHeightOverride(WidgetSize.Y);
	}
}
