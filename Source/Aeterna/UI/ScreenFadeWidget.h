// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "ScreenFadeWidget.generated.h"

class SBorder;
class SImage;
class STextBlock;
class SWidget;
class UTexture2D;

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

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleText(const FText& InTitleText);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleTexture(UTexture2D* InTitleTexture);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleTextureScale(float InTitleTextureScale);

	/**
	 *  true면 비율을 무시하고 화면을 꽉 채웁니다 (Day 카드).
	 *  false면 비율을 지킨 채 줄여 위아래에 검은 여백을 남깁니다 (타이틀 로고).
	 */
	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleTextureFillScreen(bool bInFillScreen);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleAlpha(float InTitleAlpha);

	UFUNCTION(BlueprintPure, Category="Fade")
	float GetFadeAlpha() const { return FadeAlpha; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fade")
	FLinearColor FadeColor = FLinearColor::Black;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fade", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FadeAlpha = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fade|Title")
	FLinearColor TitleColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fade|Title")
	int32 TitleFontSize = 96;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fade|Title")
	FText TitleText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fade|Title", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TitleAlpha = 1.0f;

	/**
	 *  타이틀 이미지가 차지하는 화면 비율입니다.
	 *  비율을 유지한 채 이 값만큼만 채우므로 위아래에 검은 여백이 남습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fade|Title", meta=(ClampMin="0.05", ClampMax="1.0"))
	float TitleViewportCoverage = 0.6f;

private:
	void ApplyFadeColor();
	void ApplyTitleText();
	void ApplyTitleTexture();
	void ApplyTitleAlpha();
	void UpdateTitleImageSize();

	TSharedPtr<SBorder> FadeBorder;
	TSharedPtr<SImage> TitleImage;
	TSharedPtr<STextBlock> TitleTextBlock;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> TitleTexture;

	FSlateBrush TitleImageBrush;
	float TitleTextureScale = 1.0f;
	bool bTitleTextureFillsScreen = false;
};
