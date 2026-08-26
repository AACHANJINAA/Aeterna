// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AeternaObjectiveHudWidget.generated.h"

class SWidget;

USTRUCT(BlueprintType)
struct FAeternaObjectiveHudEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Objective|HUD")
	FText ItemText;

	UPROPERTY(BlueprintReadOnly, Category="Objective|HUD")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Objective|HUD")
	int32 RequiredCount = 1;

	UPROPERTY(BlueprintReadOnly, Category="Objective|HUD")
	bool bCompleted = false;
};

UCLASS(Abstract, Blueprintable)
class AETERNA_API UAeternaObjectiveHudUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Objective|HUD")
	void SetObjectiveEntries(const TArray<FAeternaObjectiveHudEntry>& InEntries);

protected:
	UFUNCTION(BlueprintNativeEvent, Category="Objective|HUD")
	void HandleObjectiveEntriesChanged(const TArray<FAeternaObjectiveHudEntry>& InEntries);
	virtual void HandleObjectiveEntriesChanged_Implementation(const TArray<FAeternaObjectiveHudEntry>& InEntries);
};

UCLASS()
class AETERNA_API UAeternaObjectiveHudWidget : public UAeternaObjectiveHudUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	void SetObjectiveItems(const TArray<FText>& InItemTexts, const TArray<bool>& InItemCompleted);

protected:
	virtual void HandleObjectiveEntriesChanged_Implementation(const TArray<FAeternaObjectiveHudEntry>& InEntries) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Layout")
	FVector2D WidgetSize = FVector2D(360.0f, 126.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Color")
	FLinearColor PanelColor = FLinearColor(0.015f, 0.06f, 0.055f, 0.34f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Color")
	FLinearColor BackgroundColor = FLinearColor(0.006f, 0.02f, 0.018f, 0.55f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Color")
	FLinearColor BorderColor = FLinearColor(0.15f, 1.0f, 0.78f, 0.82f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Color")
	FLinearColor AccentColor = FLinearColor(0.82f, 1.0f, 0.94f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Color")
	FLinearColor IncompleteTextColor = FLinearColor(0.82f, 1.0f, 0.94f, 0.70f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Color")
	FLinearColor CompleteTextColor = FLinearColor(0.42f, 1.0f, 0.82f, 1.0f);

private:
	FText BuildObjectiveLine(const FText& ItemText, int32 CurrentCount, int32 RequiredCount) const;

	TArray<TSharedPtr<class STextBlock>> ObjectiveTextWidgets;
};
