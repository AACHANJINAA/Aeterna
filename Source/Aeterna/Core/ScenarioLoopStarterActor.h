// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameClockSubsystem.h"
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

private:
	UFUNCTION()
	void HandleRequiredScanCountReached(int32 CurrentCount, int32 RequiredCount);

	void ConfigurePlayerScanProgress();
	void BindScanCompletion();
	void UnbindScanCompletion();

	TWeakObjectPtr<UAeternaScanProgressComponent> BoundScanProgressComponent;
};
