// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioLoopStarterActor.h"

#include "Core/ScenarioManagerSubsystem.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Components/AeternaScanProgressComponent.h"

AScenarioLoopStarterActor::AScenarioLoopStarterActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AScenarioLoopStarterActor::BeginPlay()
{
	Super::BeginPlay();

	if (bStartOnBeginPlay)
	{
		StartScenarioLoop();
	}
}

void AScenarioLoopStarterActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindScanCompletion();
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
