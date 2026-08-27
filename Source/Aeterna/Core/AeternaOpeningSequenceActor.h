// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AeternaOpeningSequenceActor.generated.h"

class AScenarioLoopStarterActor;
class UAeternaOpeningMovieWidget;
class UMediaSource;

UCLASS()
class AETERNA_API AAeternaOpeningSequenceActor : public AActor
{
	GENERATED_BODY()

public:
	AAeternaOpeningSequenceActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Opening")
	void PlayOpeningSequence();

	UFUNCTION(BlueprintCallable, Category="Opening")
	void FinishOpeningSequence();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening")
	bool bPlayOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening")
	bool bLockPlayerInputDuringOpening = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening")
	int32 OpeningWidgetZOrder = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening")
	TObjectPtr<UMediaSource> OpeningMediaSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening")
	FString OpeningVideoContentPath = TEXT("Movies/OPENING.mp4");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening", meta=(ClampMin="0.1"))
	float SkipHoldSeconds = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening", meta=(ClampMin="0.0"))
	float MovieVolumeMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Opening")
	TSubclassOf<UAeternaOpeningMovieWidget> OpeningWidgetClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Opening")
	TObjectPtr<AScenarioLoopStarterActor> ScenarioStarter;

private:
	AScenarioLoopStarterActor* ResolveScenarioStarter() const;
	void SetPlayerInputLocked(bool bLocked) const;

	UPROPERTY()
	TObjectPtr<UAeternaOpeningMovieWidget> OpeningWidget;

	bool bOpeningFinished = false;
};
