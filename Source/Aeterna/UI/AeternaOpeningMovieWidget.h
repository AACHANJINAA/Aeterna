// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "AeternaOpeningMovieWidget.generated.h"

class AAeternaOpeningSequenceActor;
class SImage;
class SProgressBar;
class STextBlock;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaSource;
class UMediaTexture;

DECLARE_MULTICAST_DELEGATE(FOnAeternaOpeningMovieFinished);

UCLASS()
class AETERNA_API UAeternaOpeningMovieWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOpeningSequenceActor(AAeternaOpeningSequenceActor* InOpeningSequenceActor);
	void SetOpeningMovieSource(
		UMediaSource* InMediaSource,
		const FString& InVideoContentPath,
		float InSkipHoldSeconds,
		float InMovieVolumeMultiplier = 3.0f,
		float InMovieFadeInSeconds = 0.0f,
		float InMovieFadeOutSeconds = 0.0f);

	FOnAeternaOpeningMovieFinished OnMovieFinished;

	UFUNCTION(BlueprintCallable, Category="Opening")
	void PlayOpeningMovie();

	UFUNCTION(BlueprintCallable, Category="Opening")
	void FinishOpeningMovie();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintPure, Category="Opening")
	AAeternaOpeningSequenceActor* GetOpeningSequenceActor() const { return OpeningSequenceActor.Get(); }

private:
	UFUNCTION()
	void HandleMediaEndReached();

	UFUNCTION()
	void HandleMediaOpened(FString OpenedUrl);

	UFUNCTION()
	void HandleMediaOpenFailed(FString FailedUrl);

	void CompleteOpeningMovie();
	FString BuildContentVideoPath() const;
	void StopMovie();
	void RefreshVideoBrush();
	void RefreshSkipPrompt();
	void SetVideoAlpha(float InAlpha);

	TWeakObjectPtr<AAeternaOpeningSequenceActor> OpeningSequenceActor;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSource> OpeningMediaSource;

	FString OpeningVideoContentPath = TEXT("Movies/OPENING.mp4");

	UPROPERTY(Transient)
	TObjectPtr<UMediaPlayer> MediaPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UMediaSoundComponent> MediaSoundComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMediaTexture> MediaTexture;

	FSlateBrush VideoBrush;
	float SkipHoldSeconds = 1.5f;
	float MovieVolumeMultiplier = 3.0f;
	float MovieFadeInSeconds = 0.0f;
	float MovieFadeOutSeconds = 0.0f;
	float MovieFadeElapsedSeconds = 0.0f;
	float VideoAlpha = 1.0f;
	float CurrentSkipHoldSeconds = 0.0f;
	bool bMovieFinishing = false;
	bool bMovieFadeInActive = false;
	bool bMovieFadeOutActive = false;

	TSharedPtr<SImage> VideoImage;
	TSharedPtr<SProgressBar> SkipProgressBar;
	TSharedPtr<STextBlock> SkipTextBlock;
};
