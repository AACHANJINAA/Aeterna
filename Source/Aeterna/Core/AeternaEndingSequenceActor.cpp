// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/AeternaEndingSequenceActor.h"

#include "Core/ScenarioManagerSubsystem.h"
#include "Core/ScreenFadeSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/AeternaOpeningMovieWidget.h"

AAeternaEndingSequenceActor::AAeternaEndingSequenceActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAeternaEndingSequenceActor::BeginPlay()
{
	Super::BeginPlay();
	BindScenarioEvents();
}

void AAeternaEndingSequenceActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(PreMovieFadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RemoveWidgetTimerHandle);
	GetWorldTimerManager().ClearTimer(ExitGameTimerHandle);
	UnbindScenarioEvents();
	Super::EndPlay(EndPlayReason);
}

void AAeternaEndingSequenceActor::PlayEndingSequence()
{
	if (bEndingFinished || bEndingMovieStarted)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		FinishEndingSequence();
		return;
	}

	SetPlayerInputLocked(true);

	if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
	{
		ScreenFadeSubsystem->SetFadeColor(FLinearColor::Black);
		ScreenFadeSubsystem->StartFadeOut(PreMovieFadeOutSeconds, 0.0f);
	}

	GetWorldTimerManager().ClearTimer(PreMovieFadeTimerHandle);
	if (PreMovieFadeOutSeconds <= 0.0f)
	{
		PlayEndingMovieAfterPreFade();
		return;
	}

	GetWorldTimerManager().SetTimer(
		PreMovieFadeTimerHandle,
		this,
		&AAeternaEndingSequenceActor::PlayEndingMovieAfterPreFade,
		PreMovieFadeOutSeconds,
		false);
}

void AAeternaEndingSequenceActor::PlayEndingMovieAfterPreFade()
{
	if (bEndingFinished || bEndingMovieStarted)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		FinishEndingSequence();
		return;
	}

	UClass* WidgetClass = EndingWidgetClass.Get();
	if (!WidgetClass)
	{
		WidgetClass = UAeternaOpeningMovieWidget::StaticClass();
	}

	EndingWidget = CreateWidget<UAeternaOpeningMovieWidget>(PlayerController, WidgetClass);
	if (!EndingWidget)
	{
		FinishEndingSequence();
		return;
	}

	bEndingMovieStarted = true;
	EndingWidget->SetOpeningMovieSource(EndingMediaSource, EndingVideoContentPath, SkipHoldSeconds, MovieVolumeMultiplier, MovieFadeInSeconds, MovieFadeOutSeconds);
	EndingWidget->OnMovieFinished.AddUObject(this, &AAeternaEndingSequenceActor::FinishEndingSequence);
	EndingWidget->AddToViewport(EndingWidgetZOrder);

	if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
	{
		ScreenFadeSubsystem->ClearFade();
	}

	EndingWidget->PlayOpeningMovie();
}

void AAeternaEndingSequenceActor::FinishEndingSequence()
{
	if (bEndingFinished)
	{
		return;
	}

	bEndingFinished = true;
	bEndingMovieStarted = false;
	GetWorldTimerManager().ClearTimer(PreMovieFadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RemoveWidgetTimerHandle);
	GetWorldTimerManager().ClearTimer(ExitGameTimerHandle);

	if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
	{
		ScreenFadeSubsystem->SetFadeColor(FLinearColor::Black);
		ScreenFadeSubsystem->SetFadeAlphaImmediate(1.0f);
	}

	GetWorldTimerManager().SetTimer(
		RemoveWidgetTimerHandle,
		this,
		&AAeternaEndingSequenceActor::RemoveEndingWidgetAfterFade,
		0.05f,
		false);

	if (ExitDelayAfterEndingSeconds <= 0.0f)
	{
		QuitGameAfterEnding();
		return;
	}

	GetWorldTimerManager().SetTimer(
		ExitGameTimerHandle,
		this,
		&AAeternaEndingSequenceActor::QuitGameAfterEnding,
		ExitDelayAfterEndingSeconds,
		false);
}

void AAeternaEndingSequenceActor::RemoveEndingWidgetAfterFade()
{
	if (EndingWidget)
	{
		EndingWidget->OnMovieFinished.RemoveAll(this);
		EndingWidget->RemoveFromParent();
		EndingWidget = nullptr;
	}

	SetPlayerInputLocked(false);
}

void AAeternaEndingSequenceActor::QuitGameAfterEnding()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}

void AAeternaEndingSequenceActor::HandleScenarioCompleted(FName ScenarioId)
{
	if (bPlayOnS03Completed && ScenarioId == EndingScenarioId)
	{
		PlayEndingSequence();
	}
}

void AAeternaEndingSequenceActor::BindScenarioEvents()
{
	UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	if (!ScenarioManager)
	{
		return;
	}

	BoundScenarioManager = ScenarioManager;
	ScenarioManager->OnScenarioCompleted.AddUniqueDynamic(this, &AAeternaEndingSequenceActor::HandleScenarioCompleted);
}

void AAeternaEndingSequenceActor::UnbindScenarioEvents()
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioCompleted.RemoveDynamic(this, &AAeternaEndingSequenceActor::HandleScenarioCompleted);
	}
	BoundScenarioManager.Reset();
}

void AAeternaEndingSequenceActor::SetPlayerInputLocked(bool bLocked) const
{
	if (!bLockPlayerInputDuringEnding)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerPawn || !PlayerController)
	{
		return;
	}

	if (bLocked)
	{
		PlayerPawn->DisableInput(PlayerController);
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
	}
	else
	{
		PlayerPawn->EnableInput(PlayerController);
		PlayerController->ResetIgnoreMoveInput();
		PlayerController->ResetIgnoreLookInput();
	}
}
