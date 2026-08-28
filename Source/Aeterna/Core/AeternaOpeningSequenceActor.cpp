// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/AeternaOpeningSequenceActor.h"

#include "Core/BgmSubsystem.h"
#include "Core/ScenarioLoopStarterActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
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
	PlayIntroBgm();
}

void AAeternaOpeningSequenceActor::FinishOpeningSequence()
{
	if (bOpeningFinished)
	{
		return;
	}

	bOpeningFinished = true;

	StopIntroBgm();

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

void AAeternaOpeningSequenceActor::PlayIntroBgm() const
{
	UBgmSubsystem* BgmSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UBgmSubsystem>() : nullptr;
	if (!BgmSubsystem)
	{
		return;
	}

	if (USoundBase* Bgm = ResolveIntroBgm())
	{
		BgmSubsystem->PlayBgm(Bgm, IntroBgmVolume, IntroBgmFadeInSeconds, IntroBgmFadeOutSeconds);
	}
}

void AAeternaOpeningSequenceActor::StopIntroBgm() const
{
	UBgmSubsystem* BgmSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UBgmSubsystem>() : nullptr;
	if (!BgmSubsystem)
	{
		return;
	}

	// 밤 1의 곡은 곧바로 StartScenarioLoop이 켭니다. 여기서는 오프닝 곡만 걷어냅니다.
	if (BgmSubsystem->GetCurrentBgm() == ResolveIntroBgm())
	{
		BgmSubsystem->StopBgm(IntroBgmFadeOutSeconds);
	}
}

USoundBase* AAeternaOpeningSequenceActor::ResolveIntroBgm() const
{
	if (IntroBgm)
	{
		return IntroBgm;
	}

	const TCHAR* BgmPaths[] =
	{
		TEXT("/Game/Resource/Audio/IntroBGM.IntroBGM"),
		TEXT("/Game/Resource/Audio/IntroBGM")
	};

	for (const TCHAR* BgmPath : BgmPaths)
	{
		if (USoundBase* Bgm = LoadObject<USoundBase>(nullptr, BgmPath))
		{
			return Bgm;
		}
	}

	return nullptr;
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
