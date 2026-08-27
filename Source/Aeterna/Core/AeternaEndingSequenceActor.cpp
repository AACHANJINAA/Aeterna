// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/AeternaEndingSequenceActor.h"

#include "Core/ScenarioManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
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
	UnbindScenarioEvents();
	Super::EndPlay(EndPlayReason);
}

void AAeternaEndingSequenceActor::PlayEndingSequence()
{
	if (bEndingFinished)
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

	EndingWidget->SetOpeningMovieSource(EndingMediaSource, EndingVideoContentPath, SkipHoldSeconds, MovieVolumeMultiplier);
	EndingWidget->OnMovieFinished.AddUObject(this, &AAeternaEndingSequenceActor::FinishEndingSequence);
	EndingWidget->AddToViewport(EndingWidgetZOrder);
	EndingWidget->PlayOpeningMovie();
}

void AAeternaEndingSequenceActor::FinishEndingSequence()
{
	if (bEndingFinished)
	{
		return;
	}

	bEndingFinished = true;

	if (EndingWidget)
	{
		EndingWidget->OnMovieFinished.RemoveAll(this);
		EndingWidget->RemoveFromParent();
		EndingWidget = nullptr;
	}

	SetPlayerInputLocked(false);
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
