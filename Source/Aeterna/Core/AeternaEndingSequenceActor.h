// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AeternaEndingSequenceActor.generated.h"

class UAeternaOpeningMovieWidget;
class UMediaSource;
class UScenarioManagerSubsystem;

UCLASS()
class AETERNA_API AAeternaEndingSequenceActor : public AActor
{
	GENERATED_BODY()

public:
	AAeternaEndingSequenceActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Ending")
	void PlayEndingSequence();

	UFUNCTION(BlueprintCallable, Category="Ending")
	void FinishEndingSequence();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	bool bPlayOnS03Completed = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	bool bLockPlayerInputDuringEnding = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	FName EndingScenarioId = TEXT("S03_ForbiddenLight");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	int32 EndingWidgetZOrder = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	TObjectPtr<UMediaSource> EndingMediaSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	FString EndingVideoContentPath = TEXT("Movies/ENDING.mp4");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending", meta=(ClampMin="0.1"))
	float SkipHoldSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending", meta=(ClampMin="0.0"))
	float MovieVolumeMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	TSubclassOf<UAeternaOpeningMovieWidget> EndingWidgetClass;

private:
	UFUNCTION()
	void HandleScenarioCompleted(FName ScenarioId);

	void BindScenarioEvents();
	void UnbindScenarioEvents();
	void SetPlayerInputLocked(bool bLocked) const;

	UPROPERTY()
	TObjectPtr<UAeternaOpeningMovieWidget> EndingWidget;

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;
	bool bEndingFinished = false;
};
