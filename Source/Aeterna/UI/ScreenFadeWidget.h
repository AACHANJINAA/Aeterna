// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScreenFadeWidget.generated.h"

class SBorder;
class SWidget;

/**
 *  화면 전체를 덮는 단색 페이드 위젯입니다.
 *  연출 판정은 UScreenFadeSubsystem이 하고, 이 위젯은 알파 값만 그립니다.
 */
UCLASS()
class AETERNA_API UScreenFadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	/** 페이드 색을 설정합니다. 알파는 SetFadeAlpha가 따로 관리합니다. */
	UFUNCTION(BlueprintCallable, Category="Fade")
	void SetFadeColor(const FLinearColor& InFadeColor);

	/** 현재 페이드 알파를 0~1로 설정합니다. */
	UFUNCTION(BlueprintCallable, Category="Fade")
	void SetFadeAlpha(float InFadeAlpha);

	UFUNCTION(BlueprintPure, Category="Fade")
	float GetFadeAlpha() const { return FadeAlpha; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fade")
	FLinearColor FadeColor = FLinearColor::Black;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fade", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FadeAlpha = 0.0f;

private:
	void ApplyFadeColor();

	TSharedPtr<SBorder> FadeBorder;
};
