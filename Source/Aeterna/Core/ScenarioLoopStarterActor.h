// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameClockSubsystem.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "ScenarioLoopStarterActor.generated.h"

class UAeternaScanProgressComponent;

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
};
