// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaRedLightRuleComponent.h"

#include "Aeterna.h"
#include "Core/GameClockSubsystem.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "EngineUtils.h"
#include "Player/AeternaCharacter.h"
#include "Player/Components/AeternaClockComponent.h"

UAeternaRedLightRuleComponent::UAeternaRedLightRuleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAeternaRedLightRuleComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheLevelActors();
	ResetRedLightRule();

	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		BoundScenarioManager = ScenarioManager;
		ScenarioManager->OnScenarioStarted.AddUniqueDynamic(this, &UAeternaRedLightRuleComponent::HandleScenarioStarted);

		// 스타터보다 늦게 BeginPlay가 돌아 시작 브로드캐스트를 놓쳤을 수 있습니다.
		if (ScenarioManager->HasCurrentScenario())
		{
			HandleScenarioStarted(ScenarioManager->GetCurrentScenarioId());
		}
	}
}

void UAeternaRedLightRuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioStarted.RemoveDynamic(this, &UAeternaRedLightRuleComponent::HandleScenarioStarted);
	}
	BoundScenarioManager.Reset();

	Super::EndPlay(EndPlayReason);
}

void UAeternaRedLightRuleComponent::HandleScenarioStarted(FName ScenarioId)
{
	bRuleActive = (ScenarioId == RuleScenarioId);

	CacheLevelActors();
	ResetRedLightRule();

	if (bRuleActive)
	{
		RollTriggerMinutes();
	}
}

void UAeternaRedLightRuleComponent::CacheLevelActors()
{
	RedLightActors.Reset();
	SafeZoneActors.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FName RedLightTag(GetRedLightTag());
	const FName SafeZoneTag(GetSafeZoneTag());

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor)
		{
			continue;
		}

		if (Actor->Tags.Contains(RedLightTag))
		{
			RedLightActors.Add(Actor);
		}

		if (Actor->Tags.Contains(SafeZoneTag))
		{
			SafeZoneActors.Add(Actor);
		}
	}
}

void UAeternaRedLightRuleComponent::RollTriggerMinutes()
{
	// 배치가 아직 안 됐으면 무장하지 않습니다. 빨간 불도 경비실도 없는데
	// 발동시키면 도망칠 곳이 없는 채로 제한 시간만 흘러 밤이 실패합니다.
	if (RedLightActors.Num() == 0 || SafeZoneActors.Num() == 0)
	{
		RedLightState = EAeternaRedLightState::Done;
		TriggerClockMinutes = 0;

		UE_LOG(LogAeterna, Warning,
			TEXT("[S03] 빨간 불 규칙을 건너뜁니다 — RedLight 액터 %d개, SafeZone 액터 %d개. 둘 다 하나 이상 배치해야 발동합니다."),
			RedLightActors.Num(), SafeZoneActors.Num());
		return;
	}

	const int32 MinMinutes = FMath::Min(TriggerWindowMinMinutes, TriggerWindowMaxMinutes);
	const int32 MaxMinutes = FMath::Max(TriggerWindowMinMinutes, TriggerWindowMaxMinutes);
	TriggerClockMinutes = FMath::RandRange(MinMinutes, MaxMinutes);

	UE_LOG(LogAeterna, Log, TEXT("[S03] 적색 광원 발동 예정 시각 %02d:%02d"), TriggerClockMinutes / 60, TriggerClockMinutes % 60);
}

void UAeternaRedLightRuleComponent::ResetRedLightRule()
{
	SetRedLightsVisible(false);
	ClearCountdownFromHud();

	ActiveRedLightActor = nullptr;
	RedLightState = EAeternaRedLightState::Waiting;
	FleeRemainingSeconds = 0.0f;
	SafeHoldRemainingSeconds = 0.0f;
	TriggerClockMinutes = 0;
}

void UAeternaRedLightRuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRuleActive)
	{
		return;
	}

	switch (RedLightState)
	{
	case EAeternaRedLightState::Waiting:
	{
		const UGameClockSubsystem* GameClock = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr;
		if (GameClock && TriggerClockMinutes > 0 && GameClock->GetClockMinutes() >= TriggerClockMinutes)
		{
			BeginFlee();
		}
		return;
	}

	case EAeternaRedLightState::Fleeing:
	{
		// 경비실에 닿으면 잠깐 버틴 뒤 해제합니다. 문턱을 스치고 지나가는 것으로는
		// 끝나지 않게 하고, 도착 연출이 들어갈 자리를 만듭니다.
		if (IsPlayerInSafeZone())
		{
			SafeHoldRemainingSeconds -= DeltaTime;
			if (SafeHoldRemainingSeconds <= 0.0f)
			{
				SurviveFlee();
			}
			return;
		}

		SafeHoldRemainingSeconds = SafeHoldSeconds;

		FleeRemainingSeconds -= DeltaTime;
		PushCountdownToHud(FleeRemainingSeconds);

		if (FleeRemainingSeconds <= 0.0f)
		{
			FailFlee();
		}
		return;
	}

	default:
		return;
	}
}

void UAeternaRedLightRuleComponent::TriggerRedLightNow()
{
	if (bRuleActive && RedLightState == EAeternaRedLightState::Waiting)
	{
		BeginFlee();
	}
}

void UAeternaRedLightRuleComponent::BeginFlee()
{
	RedLightState = EAeternaRedLightState::Fleeing;
	FleeRemainingSeconds = FleeTimeLimitSeconds;
	SafeHoldRemainingSeconds = SafeHoldSeconds;

	// 후보 중 한 곳만 켭니다.
	if (RedLightActors.Num() > 0)
	{
		ActiveRedLightActor = RedLightActors[FMath::RandRange(0, RedLightActors.Num() - 1)];
		if (ActiveRedLightActor)
		{
			ActiveRedLightActor->SetActorHiddenInGame(false);
		}
	}

	PushCountdownToHud(FleeRemainingSeconds);

	UE_LOG(LogAeterna, Log, TEXT("[S03] 비정상 점등 감지: 적색"));

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_RedLightStarted(ActiveRedLightActor, FleeTimeLimitSeconds);
	}
}

void UAeternaRedLightRuleComponent::SurviveFlee()
{
	RedLightState = EAeternaRedLightState::Done;
	FleeRemainingSeconds = 0.0f;

	SetRedLightsVisible(false);
	ActiveRedLightActor = nullptr;
	ClearCountdownFromHud();

	UE_LOG(LogAeterna, Log, TEXT("[S03] 적색 광원 소멸 — 업무를 재개하십시오"));

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_RedLightSurvived();
	}
}

void UAeternaRedLightRuleComponent::FailFlee()
{
	RedLightState = EAeternaRedLightState::Done;
	FleeRemainingSeconds = 0.0f;
	ClearCountdownFromHud();

	UE_LOG(LogAeterna, Log, TEXT("[S03] 경비실 미도달 — 규칙 위반"));

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_RedLightViolated();
	}

	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}
}

void UAeternaRedLightRuleComponent::SetRedLightsVisible(bool bVisible)
{
	for (AActor* RedLightActor : RedLightActors)
	{
		if (RedLightActor)
		{
			RedLightActor->SetActorHiddenInGame(!bVisible);
		}
	}
}

bool UAeternaRedLightRuleComponent::IsPlayerInSafeZone() const
{
	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return false;
	}

	const FVector PlayerLocation = AeternaCharacter->GetActorLocation();

	for (const AActor* SafeZoneActor : SafeZoneActors)
	{
		if (!SafeZoneActor)
		{
			continue;
		}

		// 배치한 액터의 경계를 그대로 방으로 봅니다. 박스를 방 크기로 키우면 됩니다.
		FBox SafeBounds = SafeZoneActor->GetComponentsBoundingBox(true);
		if (!SafeBounds.IsValid)
		{
			continue;
		}

		SafeBounds = SafeBounds.ExpandBy(SafeZoneTolerance);
		if (SafeBounds.IsInsideOrOn(PlayerLocation))
		{
			return true;
		}
	}

	return false;
}

void UAeternaRedLightRuleComponent::PushCountdownToHud(float RemainingSeconds)
{
	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		if (UAeternaClockComponent* ClockComponent = AeternaCharacter->FindComponentByClass<UAeternaClockComponent>())
		{
			ClockComponent->SetCountdownSeconds(FMath::Max(0.0f, RemainingSeconds));
		}
	}
}

void UAeternaRedLightRuleComponent::ClearCountdownFromHud()
{
	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		if (UAeternaClockComponent* ClockComponent = AeternaCharacter->FindComponentByClass<UAeternaClockComponent>())
		{
			ClockComponent->ClearCountdown();
		}
	}
}
