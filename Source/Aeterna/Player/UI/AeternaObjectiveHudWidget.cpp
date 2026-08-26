// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaObjectiveHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void UAeternaObjectiveHudUserWidget::SetObjectiveEntries(const TArray<FAeternaObjectiveHudEntry>& InEntries)
{
	HandleObjectiveEntriesChanged(InEntries);

	if (!WidgetTree)
	{
		return;
	}

	for (int32 Index = InEntries.Num(); Index < 8; ++Index)
	{
		const FName NumberedTextBlockName(*FString::Printf(TEXT("ObjectiveText%d"), Index));
		if (UTextBlock* StaleTextBlock = WidgetTree->FindWidget<UTextBlock>(NumberedTextBlockName))
		{
			StaleTextBlock->SetText(FText::GetEmpty());
		}

		const FName CopiedTextBlockName(*FString::Printf(TEXT("ObjectiveText0_%d"), Index));
		if (UTextBlock* StaleTextBlock = WidgetTree->FindWidget<UTextBlock>(CopiedTextBlockName))
		{
			StaleTextBlock->SetText(FText::GetEmpty());
		}
	}
}

void UAeternaObjectiveHudUserWidget::HandleObjectiveEntriesChanged_Implementation(const TArray<FAeternaObjectiveHudEntry>& InEntries)
{
	(void)InEntries;
}

TSharedRef<SWidget> UAeternaObjectiveHudWidget::RebuildWidget()
{
	ObjectiveTextWidgets.Reset();

	TSharedRef<SVerticalBox> ObjectiveList = SNew(SVerticalBox);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TSharedPtr<STextBlock> ObjectiveTextWidget;
		ObjectiveList->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 2.0f))
			.HAlign(HAlign_Right)
			[
				SAssignNew(ObjectiveTextWidget, STextBlock)
				.Justification(ETextJustify::Right)
				.Font(FCoreStyle::GetDefaultFontStyle(TEXT("Mono"), 18))
				.ColorAndOpacity(FSlateColor(IncompleteTextColor))
				.Text(FText::GetEmpty())
			];

		ObjectiveTextWidgets.Add(ObjectiveTextWidget);
	}

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
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(22.0f, 18.0f, 22.0f, 18.0f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(BackgroundColor))
				.Padding(FMargin(14.0f, 10.0f))
				[
					ObjectiveList
				]
			]
		];
}

void UAeternaObjectiveHudWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetObjectiveItems(TArray<FText>(), TArray<bool>());
}

void UAeternaObjectiveHudWidget::SetObjectiveItems(const TArray<FText>& InItemTexts, const TArray<bool>& InItemCompleted)
{
	for (int32 Index = 0; Index < ObjectiveTextWidgets.Num(); ++Index)
	{
		TSharedPtr<STextBlock> ObjectiveTextWidget = ObjectiveTextWidgets[Index];
		if (!ObjectiveTextWidget)
		{
			continue;
		}

		if (!InItemTexts.IsValidIndex(Index))
		{
			ObjectiveTextWidget->SetText(FText::GetEmpty());
			continue;
		}

		const bool bCompleted = InItemCompleted.IsValidIndex(Index) && InItemCompleted[Index];
		ObjectiveTextWidget->SetText(BuildObjectiveLine(InItemTexts[Index], bCompleted ? 1 : 0, 1));
		ObjectiveTextWidget->SetColorAndOpacity(FSlateColor(bCompleted ? CompleteTextColor : IncompleteTextColor));
	}
}

void UAeternaObjectiveHudWidget::HandleObjectiveEntriesChanged_Implementation(const TArray<FAeternaObjectiveHudEntry>& InEntries)
{
	for (int32 Index = 0; Index < ObjectiveTextWidgets.Num(); ++Index)
	{
		TSharedPtr<STextBlock> ObjectiveTextWidget = ObjectiveTextWidgets[Index];
		if (!ObjectiveTextWidget)
		{
			continue;
		}

		if (!InEntries.IsValidIndex(Index))
		{
			ObjectiveTextWidget->SetText(FText::GetEmpty());
			continue;
		}

		const FAeternaObjectiveHudEntry& Entry = InEntries[Index];
		const int32 SafeRequiredCount = FMath::Max(Entry.RequiredCount, 1);
		const int32 SafeCurrentCount = FMath::Clamp(Entry.CurrentCount, 0, SafeRequiredCount);
		const bool bCompleted = Entry.bCompleted || SafeCurrentCount >= SafeRequiredCount;
		ObjectiveTextWidget->SetText(BuildObjectiveLine(Entry.ItemText, SafeCurrentCount, SafeRequiredCount));
		ObjectiveTextWidget->SetColorAndOpacity(FSlateColor(bCompleted ? CompleteTextColor : IncompleteTextColor));
	}
}

FText UAeternaObjectiveHudWidget::BuildObjectiveLine(const FText& ItemText, int32 CurrentCount, int32 RequiredCount) const
{
	return FText::Format(
		NSLOCTEXT("Aeterna", "NativeObjectiveHudItemLine", "{0}  {1} / {2}"),
		ItemText,
		FText::AsNumber(CurrentCount),
		FText::AsNumber(RequiredCount));
}
