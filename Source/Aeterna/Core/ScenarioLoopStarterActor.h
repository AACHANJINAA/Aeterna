// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameClockSubsystem.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ScenarioLoopStarterActor.generated.h"

class UAeternaScanProgressComponent;
class USoundBase;
class UTexture2D;

UCLASS()
class AETERNA_API AScenarioLoopStarterActor : public AActor
{
	GENERATED_BODY()

public:
	AScenarioLoopStarterActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Scenario")
	void StartScenarioLoop();

	UFUNCTION(BlueprintCallable, Category="Scenario|Debug")
	void DebugStartScenarioFromBeginning();

	UFUNCTION(BlueprintPure, Category="Scenario")
	FName GetScenarioId() const { return ScenarioId; }

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Scenario")
	void CompleteScenario();

	UFUNCTION(BlueprintCallable, CallInEditor, Category="Scenario|Failure")
	void FailScenarioByRuleViolation();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario")
	bool bStartOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario")
	FName ScenarioId = TEXT("S01");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Clock", meta=(ClampMin="0"))
	int32 StartClockMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Clock", meta=(ClampMin="0"))
	int32 EndClockMinutes = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Clock", meta=(ClampMin="0.0"))
	float GameMinutesPerRealSecond = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Clock")
	TArray<FGameClockEventDefinition> ClockEvents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Scan")
	bool bConfigurePlayerScanProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Scan", meta=(ClampMin="0"))
	int32 RequiredScanCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Scan")
	bool bResetScanProgressOnStart = true;

	/** 밤이 시작될 때 헤드램프를 꺼진 상태로 되돌립니다 (M-05 재기동). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario")
	bool bTurnOffHeadlampOnStart = true;

	/** 밤 시작 직전에 검은 화면 위에 Day 카드를 띄운 뒤 페이드인합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card")
	bool bShowDayCardOnStart = true;

	/** 비워두면 ScenarioId에 따라 Day 1~3을 자동으로 고릅니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card")
	FText StartDayCardText;

	/** 비워두면 ScenarioId에 따라 /Game/Resource/Texture/Day1~3 텍스처를 자동으로 찾습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card")
	TObjectPtr<UTexture2D> StartDayCardTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card", meta=(ClampMin="0.0"))
	float StartDayCardTextureDelaySeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card", meta=(ClampMin="0.0"))
	float StartDayCardTextureFadeInSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card", meta=(ClampMin="0.0"))
	float StartDayCardHoldSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card", meta=(ClampMin="0.0"))
	float StartDayCardFadeInSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Start Card")
	FLinearColor StartDayCardFadeColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Title Logo", meta=(ClampMin="0.01"))
	float TitleLogoTextureScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Title Logo", meta=(ClampMin="0.0"))
	float TitleLogoFadeInSeconds = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Title Logo", meta=(ClampMin="0.0"))
	float TitleLogoHoldSeconds = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Title Logo", meta=(ClampMin="0.0"))
	float TitleLogoFadeOutSeconds = 0.75f;

	/** 밤이 시작될 때 배경음을 켭니다. 같은 곡이면 재시작해도 끊기지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|BGM")
	bool bPlayBgmOnStart = true;

	/** 비워두면 ScenarioId에 따라 /Game/Resource/Audio/BGM3·BGM2·BGM1을 자동으로 찾습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|BGM")
	TObjectPtr<USoundBase> ScenarioBgm;

	/**
	 *  배경음 크기. BGM은 깔리는 바닥이라 효과음보다 아래에 있어야 합니다.
	 *  에셋 자체 음량이 서로 다르면 여기 말고 에셋 Volume으로 먼저 맞추십시오.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|BGM", meta=(ClampMin="0.0"))
	float BgmVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|BGM", meta=(ClampMin="0.0"))
	float BgmFadeInSeconds = 2.0f;

	/** 이전 밤의 곡을 걷어내는 시간. 곡이 바뀔 때만 씁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|BGM", meta=(ClampMin="0.0"))
	float BgmCrossFadeOutSeconds = 2.0f;

	/** 밤이 시작될 때 환경음을 켭니다. BGM 아래에 한 겹 더 깔립니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Ambience")
	bool bPlayAmbienceOnStart = true;

	/**
	 *  후보 중 하나를 무작위로 골라 틉니다.
	 *  비워두면 /Game/Resource/Audio/gamebackground1~3을 후보로 씁니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Ambience")
	TArray<TObjectPtr<USoundBase>> ScenarioAmbienceCandidates;

	/**
	 *  환경음 크기. BGM(0.5)보다 훨씬 아래에 깔려야 소리로 인식되지 않고 공기로 남습니다.
	 *  에셋마다 녹음 음량이 다르면 여기 말고 에셋 Volume으로 먼저 맞추십시오.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Ambience", meta=(ClampMin="0.0"))
	float AmbienceVolume = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Ambience", meta=(ClampMin="0.0"))
	float AmbienceFadeInSeconds = 3.0f;

	/** 이전 밤의 환경음을 걷어내는 시간. 음원이 바뀔 때만 씁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Ambience", meta=(ClampMin="0.0"))
	float AmbienceCrossFadeOutSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Scan")
	bool bCompleteScenarioOnRequiredScans = false;

	/** 시나리오 완료 시 화면을 페이드아웃합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Fade")
	bool bFadeOutOnScenarioComplete = true;

	/** 완료 시점부터 페이드아웃 시작까지의 유예 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Fade", meta=(ClampMin="0.0"))
	float ScenarioCompleteFadeDelaySeconds = 2.0f;

	/** 페이드아웃이 완전히 검게 되기까지 걸리는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Fade", meta=(ClampMin="0.0"))
	float ScenarioCompleteFadeDurationSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Fade")
	FLinearColor ScenarioCompleteFadeColor = FLinearColor::Black;

	/** 페이드아웃이 끝난 뒤 다음 밤 스타터를 이어서 시작합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Next")
	bool bStartNextScenarioAfterFadeOut = true;

	/** 이어서 시작할 다음 밤 스타터입니다. 비우면 검은 화면에서 멈춥니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Scenario|Next")
	TObjectPtr<AScenarioLoopStarterActor> NextScenarioStarter;

	/** 다음 밤 시작 시 플레이어를 옮길 지점입니다. 비우면 옮기지 않습니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Scenario|Next")
	TObjectPtr<AActor> NextScenarioPlayerStart;

	/** 전환 중 플레이어 입력을 잠급니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Next")
	bool bLockInputDuringTransition = true;

	/** 다음 밤 상태를 적용한 뒤 페이드인까지의 유예 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Next", meta=(ClampMin="0.0"))
	float NextScenarioFadeInDelaySeconds = 0.5f;

	/** 페이드인에 걸리는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Next", meta=(ClampMin="0.0"))
	float NextScenarioFadeInDurationSeconds = 1.5f;

	/** 수첩 규칙 위반 시 이 밤을 처음부터 다시 시작합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Failure")
	bool bRestartOnRuleViolation = true;

	/** 위반 시점부터 페이드아웃 시작까지의 유예 시간입니다. 이 사이에 위반 연출이 진행됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario|Failure", meta=(ClampMin="0.0"))
	float FailureFadeDelaySeconds = 2.5f;

	/** 재시작 시 플레이어를 옮길 지점입니다. 비우면 옮기지 않습니다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Scenario|Failure")
	TObjectPtr<AActor> RestartPlayerStart;

private:
	UFUNCTION()
	void HandleRequiredScanCountReached(int32 CurrentCount, int32 RequiredCount);

	UFUNCTION()
	void HandleScenarioFailed(FName FailedScenarioId, EScenarioFailureReason FailureReason);

	UFUNCTION()
	void HandleScenarioCompleted(FName CompletedScenarioId);

	UFUNCTION()
	void HandleFadeFinished(float TargetAlpha);

	void StartNextScenario();
	void RestartScenario();
	void MovePlayerTo(AActor* TargetActor);
	void MovePlayerToNextScenarioStart();
	void SetPlayerInputLocked(bool bLocked);
	void StartScenarioLoopInternal();
	bool PlayStartDayCard();
	void FadeOutTitleLogoIntro();
	void ShowStartDayCardAfterLogoIntro();
	void FinishStartDayCard();
	FText BuildStartDayCardText() const;
	bool ShouldPlayTitleLogoIntro() const;
	UTexture2D* ResolveTitleLogoTexture() const;
	UTexture2D* ResolveStartDayCardTexture() const;

	void PlayScenarioBgm();
	USoundBase* ResolveScenarioBgm() const;

	void PlayScenarioAmbience();
	USoundBase* ResolveScenarioAmbience() const;

	void ConfigurePlayerScanProgress();
	void BindScanCompletion();
	void UnbindScanCompletion();
	void BindScenarioCompletion();
	void UnbindScenarioCompletion();
	void BindFadeFinished();
	void UnbindFadeFinished();

	TWeakObjectPtr<UAeternaScanProgressComponent> BoundScanProgressComponent;
	TWeakObjectPtr<class UScenarioManagerSubsystem> BoundScenarioManager;
	TWeakObjectPtr<class UScreenFadeSubsystem> BoundScreenFadeSubsystem;

	/** 이 스타터가 시작한 밤 전환이 진행 중인지 여부입니다. */
	bool bTransitionPending = false;

	/** 진행 중인 전환이 재시작인지(true) 다음 밤으로 넘어가는 것인지(false) 구분합니다. */
	bool bRestartPending = false;

	bool bPlayedStartDayCardLastStart = false;
	bool bStartScenarioAfterDayCardPending = false;
	FTimerHandle StartLogoIntroTimerHandle;
	FTimerHandle StartLogoFadeOutTimerHandle;
	FTimerHandle StartDayCardTimerHandle;
};
