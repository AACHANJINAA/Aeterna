// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameClockSubsystem.h"
#include "GameFramework/Actor.h"
#include "ScenarioLoopStarterActor.generated.h"

UCLASS()
class AETERNA_API AScenarioLoopStarterActor : public AActor
{
	GENERATED_BODY()

public:
	AScenarioLoopStarterActor();

	virtual void BeginPlay() override;

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
};
