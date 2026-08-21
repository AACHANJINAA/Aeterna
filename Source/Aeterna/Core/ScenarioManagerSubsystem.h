// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScenarioManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EScenarioRunState : uint8
{
	None,
	Starting,
	Running,
	Restarting,
	Completed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScenarioChangedSignature, FName, ScenarioId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScenarioStateChangedSignature, FName, ScenarioId, EScenarioRunState, RunState);

UCLASS(BlueprintType)
class AETERNA_API UScenarioManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Scenario")
	void StartScenario(FName ScenarioId);

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void MarkScenarioRunning();

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void RequestRestartCurrentScenario();

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void CompleteCurrentScenario();

	UFUNCTION(BlueprintCallable, Category="Scenario")
	void ClearCurrentScenario();

	UFUNCTION(BlueprintPure, Category="Scenario")
	FName GetCurrentScenarioId() const { return CurrentScenarioId; }

	UFUNCTION(BlueprintPure, Category="Scenario")
	EScenarioRunState GetRunState() const { return RunState; }

	UFUNCTION(BlueprintPure, Category="Scenario")
	bool HasCurrentScenario() const { return !CurrentScenarioId.IsNone(); }

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioStarted;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioRestartRequested;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioChangedSignature OnScenarioCompleted;

	UPROPERTY(BlueprintAssignable, Category="Scenario")
	FScenarioStateChangedSignature OnScenarioStateChanged;

private:
	void SetRunState(EScenarioRunState NewRunState);

	FName CurrentScenarioId;
	EScenarioRunState RunState = EScenarioRunState::None;
};
