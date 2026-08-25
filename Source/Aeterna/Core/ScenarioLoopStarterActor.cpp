// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioLoopStarterActor.h"

#include "Core/ScenarioManagerSubsystem.h"
#include "Core/ScenarioVariantSubsystem.h"
#include "Core/ScreenFadeSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AeternaCharacter.h"
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

	// 배터리는 이어지므로, 램프가 켜진 채 재기동되면 암전 구간 내내 잔량이 샙니다.
	if (bTurnOffHeadlampOnStart)
	{
		if (AAeternaCharacter* AeternaCharacter = Cast<AAeternaCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			AeternaCharacter->SetHeadlampOn(false);
		}
	}
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
	bRestartPending = false;
	SetPlayerInputLocked(true);

	BindFadeFinished();
	ScreenFadeSubsystem->SetFadeColor(ScenarioCompleteFadeColor);
	ScreenFadeSubsystem->StartFadeOut(ScenarioCompleteFadeDurationSeconds, ScenarioCompleteFadeDelaySeconds);
}

void AScenarioLoopStarterActor::HandleScenarioFailed(FName FailedScenarioId, EScenarioFailureReason FailureReason)
{
	(void)FailureReason;

	if (!bRestartOnRuleViolation || FailedScenarioId != ScenarioId || bTransitionPending)
	{
		return;
	}

	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return;
	}

	bTransitionPending = true;
	bRestartPending = true;
	SetPlayerInputLocked(true);

	// 유예 시간 동안 위반 연출(눈구멍 등장 등)이 진행되고, 그 뒤 페이드아웃이 시작됩니다.
	BindFadeFinished();
	ScreenFadeSubsystem->SetFadeColor(ScenarioCompleteFadeColor);
	ScreenFadeSubsystem->StartFadeOut(ScenarioCompleteFadeDurationSeconds, FailureFadeDelaySeconds);
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

	if (bRestartPending)
	{
		bRestartPending = false;
		RestartScenario();
		return;
	}

	if (!bStartNextScenarioAfterFadeOut || !NextScenarioStarter)
	{
		// 이어질 밤이 없으면 검은 화면을 유지하되 입력만 돌려줍니다.
		SetPlayerInputLocked(false);
		return;
	}

	StartNextScenario();
}

void AScenarioLoopStarterActor::RestartScenario()
{
	MovePlayerTo(RestartPlayerStart);

	// 플레이어가 옮긴 뼈를 배치 위치로 되돌립니다. 시나리오 시작에서도 한 번 더 적용됩니다.
	if (UScenarioVariantSubsystem* ScenarioVariantSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScenarioVariantSubsystem>() : nullptr)
	{
		ScenarioVariantSubsystem->RestoreAuthoredTransforms();
	}

	// 같은 시나리오를 처음부터 다시 시작합니다. 클럭·스캔 진행도·운반 기록이 초기화됩니다.
	StartScenarioLoop();

	SetPlayerInputLocked(false);

	if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
	{
		ScreenFadeSubsystem->StartFadeIn(NextScenarioFadeInDurationSeconds, NextScenarioFadeInDelaySeconds);
	}
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
	MovePlayerTo(NextScenarioPlayerStart);
}

void AScenarioLoopStarterActor::MovePlayerTo(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FRotator TargetRotation = TargetActor->GetActorRotation();

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
	ScenarioManager->OnScenarioFailed.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleScenarioFailed);
}

void AScenarioLoopStarterActor::UnbindScenarioCompletion()
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioCompleted.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleScenarioCompleted);
		ScenarioManager->OnScenarioFailed.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleScenarioFailed);
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
