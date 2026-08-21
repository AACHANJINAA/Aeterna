// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AeternaBatteryHudWidget.generated.h"

class SWidget;

UCLASS()
class AETERNA_API UAeternaBatteryHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Battery|HUD")
	void SetBattery(float Current, float Max, float Normalized);

	UFUNCTION(BlueprintCallable, Category="Battery|HUD")
	void SetBatteryLabel(const FText& InLabelText);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Layout")
	FVector2D WidgetSize = FVector2D(420.0f, 82.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Layout")
	int32 SegmentCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Text")
	FText LabelText = FText::FromString(TEXT("BATTERY"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color")
	FLinearColor BackgroundColor = FLinearColor(0.005f, 0.006f, 0.006f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color")
	FLinearColor BorderColor = FLinearColor(0.42f, 0.42f, 0.36f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color")
	FLinearColor FullBatteryColor = FLinearColor(0.98f, 0.72f, 0.08f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color")
	FLinearColor LowBatteryColor = FLinearColor(0.9f, 0.08f, 0.04f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color")
	FLinearColor EmptySegmentColor = FLinearColor(0.035f, 0.038f, 0.035f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color")
	FLinearColor TextColor = FLinearColor(0.96f, 0.98f, 0.9f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Color", meta=(ClampMin="0.1"))
	float BatteryColorExponent = 1.8f;

private:
	FLinearColor GetFillColor(float Normalized) const;

	TArray<TSharedPtr<class SBorder>> SegmentWidgets;
	TSharedPtr<class STextBlock> LabelTextWidget;
	TSharedPtr<class STextBlock> PercentTextWidget;
};
