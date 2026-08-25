// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaClockFreezeRuleComponent.h"

#include "Aeterna.h"
#include "Core/GameClockSubsystem.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/World.h"
#include "Player/AeternaCharacter.h"
#include "Player/Components/AeternaCameraFallComponent.h"

UAeternaClockFreezeRuleComponent::UAeternaClockFreezeRuleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAeternaClockFreezeRuleComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UScenarioManagerSubsystem* ScenarioManager = World->GetSubsystem<UScenarioManagerSubsystem>())
	{
		BoundScenarioManager = ScenarioManager;
		ScenarioManager->OnScenarioStarted.AddUniqueDynamic(this, &UAeternaClockFreezeRuleComponent::HandleScenarioStarted);

		if (ScenarioManager->HasCurrentScenario())
		{
			HandleScenarioStarted(ScenarioManager->GetCurrentScenarioId());
		}
	}

	if (UGameClockSubsystem* GameClock = World->GetSubsystem<UGameClockSubsystem>())
	{
		BoundGameClock = GameClock;
		GameClock->OnClockMinuteChanged.AddUniqueDynamic(this, &UAeternaClockFreezeRuleComponent::HandleClockMinuteChanged);
	}
}

void UAeternaClockFreezeRuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioStarted.RemoveDynamic(this, &UAeternaClockFreezeRuleComponent::HandleScenarioStarted);
	}
	BoundScenarioManager.Reset();

	if (UGameClockSubsystem* GameClock = BoundGameClock.Get())
	{
		GameClock->OnClockMinuteChanged.RemoveDynamic(this, &UAeternaClockFreezeRuleComponent::HandleClockMinuteChanged);
	}
	BoundGameClock.Reset();

	Super::EndPlay(EndPlayReason);
}

void UAeternaClockFreezeRuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRuleActive || !bFreezeWindowOpen)
	{
		return;
	}

	// 유예 없이 즉시 판정합니다. 시계를 보고 미리 멈추는 것이 이 규칙의 내용입니다.
	if (HasMovementInput())
	{
		TriggerViolation();
	}
}

void UAeternaClockFreezeRuleComponent::ResetClockFreezeRule()
{
	if (bFreezeWindowOpen)
	{
		bFreezeWindowOpen = false;

		if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
		{
			AeternaCharacter->BP_ClockFreezeWindowChanged(false);
		}
	}
}

void UAeternaClockFreezeRuleComponent::HandleScenarioStarted(FName ScenarioId)
{
	bRuleActive = (ScenarioId == RuleScenarioId);
	ResetClockFreezeRule();
}

void UAeternaClockFreezeRuleComponent::HandleClockMinuteChanged(int32 ClockMinutes)
{
	const bool bShouldBeOpen = bRuleActive && IsFreezeMinute(ClockMinutes);
	if (bShouldBeOpen == bFreezeWindowOpen)
	{
		return;
	}

	bFreezeWindowOpen = bShouldBeOpen;

	UE_LOG(LogAeterna, Log,
		TEXT("[ClockFreeze] %02d:%02d — 정지 창 %s"),
		ClockMinutes / 60, ClockMinutes % 60,
		bFreezeWindowOpen ? TEXT("열림") : TEXT("닫힘"));

	// 신호는 BP에 맡깁니다. C++은 창이 열리고 닫혔다는 사실만 알립니다.
	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_ClockFreezeWindowChanged(bFreezeWindowOpen);
	}
}

bool UAeternaClockFreezeRuleComponent::IsFreezeMinute(int32 ClockMinutes) const
{
	const int32 MinuteOfHour = ClockMinutes % 60;
	return MinuteOfHour >= FreezeMinuteOfHour && MinuteOfHour < FreezeMinuteOfHour + FreezeWindowMinutes;
}

bool UAeternaClockFreezeRuleComponent::HasMovementInput() const
{
	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return false;
	}

	// 시점 회전은 이동 입력에 잡히지 않으므로 그대로 허용됩니다.
	return AeternaCharacter->GetLastMovementInputVector().SizeSquared() > (MoveInputThreshold * MoveInputThreshold);
}

void UAeternaClockFreezeRuleComponent::TriggerViolation()
{
	bRuleActive = false;
	bFreezeWindowOpen = false;

	UE_LOG(LogAeterna, Log, TEXT("[ClockFreeze] 정지 창에서 이동 입력 — 위반"));

	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (AeternaCharacter)
	{
		// 낙하 연출은 규칙 3과 공용입니다.
		if (UAeternaCameraFallComponent* CameraFallComponent = AeternaCharacter->GetCameraFallComponent())
		{
			CameraFallComponent->StartFall();
		}

		AeternaCharacter->BP_ClockFreezeRuleViolated();
	}

	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}
}
