// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameClockSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScenarioManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EScenarioRunState : uint8
{
	None,
	Starting,
	Running,
	Restarting,
	Failed,
	Completed
};

UENUM(BlueprintType)
enum class EScenarioFailureReason : uint8
{
	None,
	RuleViolation,
	TimeExpired,
	BatteryDepleted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScenarioChangedSignature, FName, ScenarioId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScenarioStateChangedSignature, FName, ScenarioId, EScenarioRunState, RunState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScenarioFailedSignature, FName, ScenarioId, EScenarioFailureReason, FailureReason);

UCLASS(BlueprintType)
class AETERNA_API UScenarioManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void StartScenario(FName ScenarioId);

	UFUNCTION(BlueprintCallable, Category="Scenario", meta=(AutoCreateRefTerm="ClockEvents"))
	void StartScenarioLoop(FName ScenarioId, int32 StartClockMinutes, int32 EndClockMinutes, float GameMinutesPerRealSecond, const TArray<FGameClockEventDefinition>& ClockEvents);

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void MarkScenarioRunning();

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void RequestRestartCurrentScenario();

	UFUNCTION(BlueprintCallable, Category="Scenario|Failure")
	void FailCurrentScenarioByRuleViolation();

	UFUNCTION(BlueprintCallable, Category="Scenario|Failure")
	void FailCurrentScenarioByTimeExpired();

	UFUNCTION(BlueprintCallable, Category="Scenario|Failure")
	void FailCurrentScenarioByBatteryDepleted();

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void CompleteCurrentScenario();

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void ClearCurrentScenario();

	UFUNCTION(BlueprintCallable, Category="Scenario|Debug")
	void SetLoopDebugLogVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category="Scenario")
	FName GetCurrentScenarioId() const { return CurrentScenarioId; }

	UFUNCTION(BlueprintPure, Category="Scenario")
	EScenarioRunState GetRunState() const { return RunState; }

	UFUNCTION(BlueprintPure, Category="Scenario|Failure")
	EScenarioFailureReason GetLastFailureReason() const { return LastFailureReason; }

	UFUNCTION(BlueprintPure, Category="Scenario")
	bool HasCurrentScenario() const { return !CurrentScenarioId.IsNone(); }

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioStarted;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioRestartRequested;

	UPROPERTY(BlueprintAssignable, Category="Scenario|Failure")
	FScenarioFailedSignature OnScenarioFailed;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioCompleted;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioTimeExpired;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioStateChangedSignature OnScenarioStateChanged;

private:
	void SetRunState(EScenarioRunState NewRunState);
	void FailCurrentScenario(EScenarioFailureReason FailureReason);
	void ShowLoopDebugLog() const;
	void UpdateLoopTimeDebugLog() const;
	FString BuildLoopDebugLogText() const;
	FString BuildLoopTimeDebugLogText() const;

	UFUNCTION()
	void HandleClockFinished(int32 ClockMinutes);

	FName CurrentScenarioId;
	EScenarioRunState RunState = EScenarioRunState::None;
	EScenarioFailureReason LastFailureReason = EScenarioFailureReason::None;
	bool bShowLoopDebugLog = false;
};
