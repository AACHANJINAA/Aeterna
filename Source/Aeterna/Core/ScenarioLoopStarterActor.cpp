// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioLoopStarterActor.h"

#include "Core/ScenarioManagerSubsystem.h"
#include "Core/ScreenFadeSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Components/AeternaCarryComponent.h"
#include "Player/Components/AeternaScanProgressComponent.h"

AScenarioLoopStarterActor::AScenarioLoopStarterActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AScenarioLoopStarterActor::BeginPlay()
{
	Super::BeginPlay();

	BindScenarioCompletion();

	if (bStartOnBeginPlay)
	{
		StartScenarioLoop();
	}
}

void AScenarioLoopStarterActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindScanCompletion();
	UnbindScenarioCompletion();
	UnbindFadeFinished();
	Super::EndPlay(EndPlayReason);
}

void AScenarioLoopStarterActor::StartScenarioLoop()
{
	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->StartScenarioLoop(ScenarioId, StartClockMinutes, EndClockMinutes, GameMinutesPerRealSecond, ClockEvents);
	}

	ConfigurePlayerScanProgress();
}

void AScenarioLoopStarterActor::CompleteScenario()
{
	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->CompleteCurrentScenario();
	}
}

void AScenarioLoopStarterActor::FailScenarioByRuleViolation()
{
	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}
}

void AScenarioLoopStarterActor::HandleRequiredScanCountReached(int32 CurrentCount, int32 RequiredCount)
{
	(void)CurrentCount;
	(void)RequiredCount;

	if (bCompleteScenarioOnRequiredScans)
	{
		CompleteScenario();
	}
}

void AScenarioLoopStarterActor::HandleScenarioCompleted(FName CompletedScenarioId)
{
	if (!bFadeOutOnScenarioComplete || CompletedScenarioId != ScenarioId || bTransitionPending)
	{
		return;
	}

	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return;
	}

	bTransitionPending = true;
	SetPlayerInputLocked(true);

	BindFadeFinished();
	ScreenFadeSubsystem->SetFadeColor(ScenarioCompleteFadeColor);
	ScreenFadeSubsystem->StartFadeOut(ScenarioCompleteFadeDurationSeconds, ScenarioCompleteFadeDelaySeconds);
}

void AScenarioLoopStarterActor::HandleFadeFinished(float TargetAlpha)
{
	// 화면이 완전히 덮인 순간에만 다음 밤으로 넘어갑니다. 페이드인 종료는 무시합니다.
	if (!bTransitionPending || TargetAlpha < 1.0f)
	{
		return;
	}

	bTransitionPending = false;
	UnbindFadeFinished();

	if (!bStartNextScenarioAfterFadeOut || !NextScenarioStarter)
	{
		// 이어질 밤이 없으면 검은 화면을 유지하되 입력만 돌려줍니다.
		SetPlayerInputLocked(false);
		return;
	}

	StartNextScenario();
}

void AScenarioLoopStarterActor::StartNextScenario()
{
	MovePlayerToNextScenarioStart();

	// StartScenarioLoop이 OnScenarioStarted를 브로드캐스트하고,
	// UScenarioVariantSubsystem이 Night_ 태그 액터들을 그 밤의 상태로 갈아끼웁니다.
	NextScenarioStarter->StartScenarioLoop();

	SetPlayerInputLocked(false);

	if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
	{
		ScreenFadeSubsystem->StartFadeIn(NextScenarioFadeInDurationSeconds, NextScenarioFadeInDelaySeconds);
	}
}

void AScenarioLoopStarterActor::MovePlayerToNextScenarioStart()
{
	if (!NextScenarioPlayerStart)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector TargetLocation = NextScenarioPlayerStart->GetActorLocation();
	const FRotator TargetRotation = NextScenarioPlayerStart->GetActorRotation();

	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
	{
		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}

	PlayerPawn->TeleportTo(TargetLocation, TargetRotation);

	if (AController* PlayerController = PlayerPawn->GetController())
	{
		PlayerController->SetControlRotation(TargetRotation);
	}
}

void AScenarioLoopStarterActor::SetPlayerInputLocked(bool bLocked)
{
	if (!bLockInputDuringTransition)
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
	}
	else
	{
		PlayerPawn->EnableInput(PlayerController);
	}
}

void AScenarioLoopStarterActor::ConfigurePlayerScanProgress()
{
	if (!bConfigurePlayerScanProgress)
	{
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	UAeternaScanProgressComponent* ScanProgressComponent = PlayerCharacter
		? PlayerCharacter->FindComponentByClass<UAeternaScanProgressComponent>()
		: nullptr;
	if (!ScanProgressComponent)
	{
		return;
	}

	UnbindScanCompletion();
	BoundScanProgressComponent = ScanProgressComponent;

	if (bResetScanProgressOnStart)
	{
		ScanProgressComponent->ResetScanProgress();

		if (UAeternaCarryComponent* CarryComponent = PlayerCharacter->FindComponentByClass<UAeternaCarryComponent>())
		{
			CarryComponent->ResetInstallProgress();
		}
	}

	ScanProgressComponent->SetRequiredScanCount(RequiredScanCount);
	BindScanCompletion();

	if (bCompleteScenarioOnRequiredScans && ScanProgressComponent->HasCompletedRequiredScans())
	{
		CompleteScenario();
	}
}

void AScenarioLoopStarterActor::BindScanCompletion()
{
	if (UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get())
	{
		ScanProgressComponent->OnRequiredScanCountReached.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleRequiredScanCountReached);
	}
}

void AScenarioLoopStarterActor::UnbindScanCompletion()
{
	if (UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get())
	{
		ScanProgressComponent->OnRequiredScanCountReached.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleRequiredScanCountReached);
	}
	BoundScanProgressComponent.Reset();
}

void AScenarioLoopStarterActor::BindScenarioCompletion()
{
	UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	if (!ScenarioManager)
	{
		return;
	}

	BoundScenarioManager = ScenarioManager;
	ScenarioManager->OnScenarioCompleted.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleScenarioCompleted);
}

void AScenarioLoopStarterActor::UnbindScenarioCompletion()
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioCompleted.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleScenarioCompleted);
	}
	BoundScenarioManager.Reset();
}

void AScenarioLoopStarterActor::BindFadeFinished()
{
	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return;
	}

	BoundScreenFadeSubsystem = ScreenFadeSubsystem;
	ScreenFadeSubsystem->OnFadeFinished.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleFadeFinished);
}

void AScenarioLoopStarterActor::UnbindFadeFinished()
{
	if (UScreenFadeSubsystem* ScreenFadeSubsystem = BoundScreenFadeSubsystem.Get())
	{
		ScreenFadeSubsystem->OnFadeFinished.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleFadeFinished);
	}
	BoundScreenFadeSubsystem.Reset();
}
