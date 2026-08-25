// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaObjectiveHudWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UAeternaObjectiveHudUserWidget::SetObjectiveEntries(const TArray<FAeternaObjectiveHudEntry>& InEntries)
{
	HandleObjectiveEntriesChanged(InEntries);
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
				.ColorAndOpacity(FSlateColor(IncompleteTextColor))
				.Text(FText::GetEmpty())
			];

		ObjectiveTextWidgets.Add(ObjectiveTextWidget);
	}

	return SNew(SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FSlateColor(BorderColor))
			.Padding(FMargin(1.0f))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FSlateColor(BackgroundColor))
				.Padding(FMargin(14.0f, 9.0f))
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
		ObjectiveTextWidget->SetText(BuildObjectiveLine(InItemTexts[Index], bCompleted));
		ObjectiveTextWidget->SetColorAndOpacity(FSlateColor(bCompleted ? CompleteTextColor : IncompleteTextColor));
	}
}

void UAeternaObjectiveHudWidget::HandleObjectiveEntriesChanged_Implementation(const TArray<FAeternaObjectiveHudEntry>& InEntries)
{
	TArray<FText> ItemTexts;
	TArray<bool> ItemCompleted;
	ItemTexts.Reserve(InEntries.Num());
	ItemCompleted.Reserve(InEntries.Num());

	for (const FAeternaObjectiveHudEntry& Entry : InEntries)
	{
		ItemTexts.Add(Entry.ItemText);
		ItemCompleted.Add(Entry.bCompleted);
	}

	SetObjectiveItems(ItemTexts, ItemCompleted);
}

FText UAeternaObjectiveHudWidget::BuildObjectiveLine(const FText& ItemText, bool bCompleted) const
{
	return FText::Format(
		NSLOCTEXT("Aeterna", "NativeObjectiveHudItemLine", "{0}  {1} / 1"),
		ItemText,
		FText::AsNumber(bCompleted ? 1 : 0));
}
