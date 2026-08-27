// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/AeternaOpeningSequenceActor.h"

#include "Core/ScenarioLoopStarterActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/AeternaOpeningMovieWidget.h"

AAeternaOpeningSequenceActor::AAeternaOpeningSequenceActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAeternaOpeningSequenceActor::BeginPlay()
{
	Super::BeginPlay();

	if (bPlayOnBeginPlay)
	{
		PlayOpeningSequence();
	}
}

void AAeternaOpeningSequenceActor::PlayOpeningSequence()
{
	if (bOpeningFinished)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController)
	{
		FinishOpeningSequence();
		return;
	}

	SetPlayerInputLocked(true);

	UClass* WidgetClass = OpeningWidgetClass.Get();
	if (!WidgetClass)
	{
		WidgetClass = UAeternaOpeningMovieWidget::StaticClass();
	}

	OpeningWidget = CreateWidget<UAeternaOpeningMovieWidget>(PlayerController, WidgetClass);
	if (!OpeningWidget)
	{
		FinishOpeningSequence();
		return;
	}

	OpeningWidget->SetOpeningSequenceActor(this);
	OpeningWidget->SetOpeningMovieSource(OpeningMediaSource, OpeningVideoContentPath, SkipHoldSeconds, MovieVolumeMultiplier);
	OpeningWidget->AddToViewport(OpeningWidgetZOrder);
	OpeningWidget->PlayOpeningMovie();
}

void AAeternaOpeningSequenceActor::FinishOpeningSequence()
{
	if (bOpeningFinished)
	{
		return;
	}

	bOpeningFinished = true;

	if (OpeningWidget)
	{
		OpeningWidget->RemoveFromParent();
		OpeningWidget = nullptr;
	}

	SetPlayerInputLocked(false);

	if (AScenarioLoopStarterActor* Starter = ResolveScenarioStarter())
	{
		Starter->StartScenarioLoop();
	}
}

AScenarioLoopStarterActor* AAeternaOpeningSequenceActor::ResolveScenarioStarter() const
{
	if (ScenarioStarter)
	{
		return ScenarioStarter;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AScenarioLoopStarterActor> It(World); It; ++It)
	{
		if (It->GetScenarioId() == TEXT("S01_Handover") || It->GetScenarioId() == TEXT("S01"))
		{
			return *It;
		}
	}

	return nullptr;
}

void AAeternaOpeningSequenceActor::SetPlayerInputLocked(bool bLocked) const
{
	if (!bLockPlayerInputDuringOpening)
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
