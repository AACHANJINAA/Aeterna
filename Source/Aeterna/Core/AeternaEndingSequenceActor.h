// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "AeternaEndingSequenceActor.generated.h"

class UAeternaOpeningMovieWidget;
class UMediaSource;
class UScenarioManagerSubsystem;
class USoundBase;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|Fade", meta=(ClampMin="0.0"))
	float PreMovieFadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|Fade", meta=(ClampMin="0.0"))
	float MovieFadeInSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|Fade", meta=(ClampMin="0.0"))
	float MovieFadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|Exit", meta=(ClampMin="0.0"))
	float ExitDelayAfterEndingSeconds = 3.0f;

	/**
	 *  엔딩 영상이 도는 동안 흐르는 배경음입니다.
	 *  비워두면 /Game/Resource/Audio/Gallery_of_light_1을 찾아 씁니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|BGM")
	TObjectPtr<USoundBase> EndingBgm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|BGM", meta=(ClampMin="0.0"))
	float EndingBgmVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|BGM", meta=(ClampMin="0.0"))
	float EndingBgmFadeInSeconds = 1.0f;

	/** 영상이 끝나거나 Q로 건너뛸 때 배경음을 걷어내는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending|BGM", meta=(ClampMin="0.0"))
	float EndingBgmFadeOutSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ending")
	TSubclassOf<UAeternaOpeningMovieWidget> EndingWidgetClass;

private:
	UFUNCTION()
	void HandleScenarioCompleted(FName ScenarioId);

	void PlayEndingMovieAfterPreFade();
	void RemoveEndingWidgetAfterFade();
	void QuitGameAfterEnding();
	void BindScenarioEvents();
	void UnbindScenarioEvents();
	void SetPlayerInputLocked(bool bLocked) const;
	void PlayEndingBgm() const;
	void StopEndingBgm() const;
	USoundBase* ResolveEndingBgm() const;

	UPROPERTY()
	TObjectPtr<UAeternaOpeningMovieWidget> EndingWidget;

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;
	FTimerHandle PreMovieFadeTimerHandle;
	FTimerHandle RemoveWidgetTimerHandle;
	FTimerHandle ExitGameTimerHandle;
	bool bEndingFinished = false;
	bool bEndingMovieStarted = false;
};
