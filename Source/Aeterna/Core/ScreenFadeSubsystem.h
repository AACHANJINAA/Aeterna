// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScreenFadeSubsystem.generated.h"

class UScreenFadeWidget;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScreenFadeFinishedSignature, float, TargetAlpha);

/**
 *  화면 페이드 진행을 관리합니다.
 *  판정과 진행은 여기서 하고, 그리기는 UScreenFadeWidget이 담당합니다.
 */
UCLASS(BlueprintType)
class AETERNA_API UScreenFadeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	/** 화면을 검게 덮습니다. DelaySeconds 만큼 기다린 뒤 DurationSeconds 동안 진행합니다. */
	UFUNCTION(BlueprintCallable, Category="Fade")
	void StartFadeOut(float DurationSeconds = 1.5f, float DelaySeconds = 0.0f);

	/** 덮인 화면을 다시 걷어냅니다. */
	UFUNCTION(BlueprintCallable, Category="Fade")
	void StartFadeIn(float DurationSeconds = 1.5f, float DelaySeconds = 0.0f);

	/** 진행 중인 페이드를 멈추고 알파를 즉시 설정합니다. */
	UFUNCTION(BlueprintCallable, Category="Fade")
	void SetFadeAlphaImmediate(float InFadeAlpha);

	/** 페이드를 즉시 걷어냅니다. */
	UFUNCTION(BlueprintCallable, Category="Fade")
	void ClearFade();

	UFUNCTION(BlueprintCallable, Category="Fade")
	void SetFadeColor(FLinearColor InFadeColor);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleText(FText InTitleText);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleTexture(UTexture2D* InTitleTexture);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void SetTitleAlphaImmediate(float InTitleAlpha);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void StartTitleFadeIn(float DurationSeconds = 1.5f, float DelaySeconds = 0.0f);

	UFUNCTION(BlueprintCallable, Category="Fade|Title")
	void ClearTitleText();

	UFUNCTION(BlueprintPure, Category="Fade")
	bool IsFading() const { return bFadeActive; }

	UFUNCTION(BlueprintPure, Category="Fade")
	float GetFadeAlpha() const { return CurrentAlpha; }

	/** 페이드아웃이 끝나 화면이 완전히 덮인 상태인지 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Fade")
	bool IsScreenFadedOut() const { return !bFadeActive && CurrentAlpha >= 1.0f; }

	/** 페이드 진행이 끝났을 때 브로드캐스트합니다. 인자는 도달한 알파입니다. */
	UPROPERTY(BlueprintAssignable, Category="Fade")
	FScreenFadeFinishedSignature OnFadeFinished;

private:
	void StartFade(float InTargetAlpha, float DurationSeconds, float DelaySeconds);
	void StartTitleFade(float InTargetAlpha, float DurationSeconds, float DelaySeconds);
	void ApplyFadeAlpha(float InFadeAlpha);
	void ApplyTitleAlpha(float InTitleAlpha);
	UScreenFadeWidget* EnsureFadeWidget();
	void RemoveFadeWidget();

	UPROPERTY(Transient)
	TObjectPtr<UScreenFadeWidget> FadeWidget;

	FLinearColor FadeColor = FLinearColor::Black;
	FText TitleText;
	TObjectPtr<UTexture2D> TitleTexture;

	bool bFadeActive = false;
	bool bTitleFadeActive = false;
	float CurrentAlpha = 0.0f;
	float StartAlpha = 0.0f;
	float TargetAlpha = 0.0f;
	float FadeDuration = 0.0f;
	float ElapsedSeconds = 0.0f;
	float RemainingDelaySeconds = 0.0f;
	float CurrentTitleAlpha = 1.0f;
	float StartTitleAlpha = 1.0f;
	float TargetTitleAlpha = 1.0f;
	float TitleFadeDuration = 0.0f;
	float TitleElapsedSeconds = 0.0f;
	float TitleRemainingDelaySeconds = 0.0f;
};
