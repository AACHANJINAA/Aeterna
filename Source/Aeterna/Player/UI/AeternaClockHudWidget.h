// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AeternaClockHudWidget.generated.h"

class STextBlock;
class SWidget;

UCLASS()
class AETERNA_API UAeternaClockHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Clock|HUD")
	void SetClockMinutes(int32 InClockMinutes);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Layout")
	FVector2D WidgetSize = FVector2D(282.0f, 92.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor PanelColor = FLinearColor(0.015f, 0.06f, 0.055f, 0.34f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor BorderColor = FLinearColor(0.15f, 1.0f, 0.78f, 0.82f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor DigitColor = FLinearColor(0.42f, 1.0f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor AccentColor = FLinearColor(0.82f, 1.0f, 0.94f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Text")
	int32 DigitFontSize = 42;

private:
	FText BuildClockText(int32 InClockMinutes) const;

	TSharedPtr<STextBlock> ClockTextWidget;
};
