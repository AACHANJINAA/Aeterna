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

	/**
	 *  시계 자리에 남은 시간을 초 단위로 띄웁니다.
	 *  켜져 있는 동안에는 SetClockMinutes가 무시됩니다 — 클럭이 카운트다운을 덮어쓰면 안 됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category="Clock|HUD")
	void SetCountdownSeconds(float RemainingSeconds);

	/** 카운트다운을 끄고 시계 표시로 돌아갑니다. */
	UFUNCTION(BlueprintCallable, Category="Clock|HUD")
	void ClearCountdown();

	UFUNCTION(BlueprintPure, Category="Clock|HUD")
	bool IsCountdownActive() const { return bCountdownActive; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Layout")
	FVector2D WidgetSize = FVector2D(338.0f, 110.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor PanelColor = FLinearColor(0.015f, 0.06f, 0.055f, 0.34f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor BorderColor = FLinearColor(0.15f, 1.0f, 0.78f, 0.82f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor DigitColor = FLinearColor(0.42f, 1.0f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor AccentColor = FLinearColor(0.82f, 1.0f, 0.94f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Text")
	int32 DigitFontSize = 50;

	/** 카운트다운 중 숫자 색입니다. 시계와 구분되어야 합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Color")
	FLinearColor CountdownDigitColor = FLinearColor(1.0f, 0.18f, 0.16f, 1.0f);

private:
	FText BuildClockText(int32 InClockMinutes) const;
	FText BuildCountdownText(float RemainingSeconds) const;

	TSharedPtr<STextBlock> ClockTextWidget;

	bool bCountdownActive = false;
	int32 LastShownClockMinutes = 60;
};
