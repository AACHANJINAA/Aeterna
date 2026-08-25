// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaVanishRuleComponent.h"

#include "Aeterna.h"
#include "Camera/CameraComponent.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Player/AeternaCharacter.h"
#include "Player/Components/AeternaCameraFallComponent.h"

UAeternaVanishRuleComponent::UAeternaVanishRuleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAeternaVanishRuleComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheVanishActors();

	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		BoundScenarioManager = ScenarioManager;
		ScenarioManager->OnScenarioStarted.AddUniqueDynamic(this, &UAeternaVanishRuleComponent::HandleScenarioStarted);

		if (ScenarioManager->HasCurrentScenario())
		{
			HandleScenarioStarted(ScenarioManager->GetCurrentScenarioId());
		}
	}
}

void UAeternaVanishRuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioStarted.RemoveDynamic(this, &UAeternaVanishRuleComponent::HandleScenarioStarted);
	}
	BoundScenarioManager.Reset();

	Super::EndPlay(EndPlayReason);
}

void UAeternaVanishRuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 위반 후에는 공용 낙하 컴포넌트가 연출을 이어갑니다.
	if (VanishState == EAeternaVanishState::Falling || !bRuleActive)
	{
		return;
	}

	switch (VanishState)
	{
	case EAeternaVanishState::Frozen:
		// 화석 쪽으로 걸어오던 중에 발동하므로, 손을 뗄 시간을 먼저 준 뒤 판정합니다.
		if (FreezeGraceRemainingSeconds > 0.0f)
		{
			FreezeGraceRemainingSeconds -= DeltaTime;
			return;
		}

		// 시점 회전은 허용하고 이동 입력만 봅니다.
		if (HasMovementInput())
		{
			TriggerViolation();
			return;
		}

		FreezeRemainingSeconds -= DeltaTime;
		if (FreezeRemainingSeconds <= 0.0f)
		{
			SurviveFreeze();
		}
		return;

	case EAeternaVanishState::Cooldown:
		CooldownRemainingSeconds -= DeltaTime;
		if (CooldownRemainingSeconds <= 0.0f)
		{
			VanishState = EAeternaVanishState::Idle;
		}
		return;

	case EAeternaVanishState::Idle:
		if (ShouldTriggerVanish())
		{
			BeginFreeze();
		}
		return;

	default:
		return;
	}
}

void UAeternaVanishRuleComponent::ResetVanishRule()
{
	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		if (UAeternaCameraFallComponent* CameraFallComponent = AeternaCharacter->GetCameraFallComponent())
		{
			CameraFallComponent->ResetFall();
		}
	}

	SetVanishActorsHidden(false);

	VanishState = EAeternaVanishState::Idle;
	FreezeRemainingSeconds = 0.0f;
	FreezeGraceRemainingSeconds = 0.0f;
	CooldownRemainingSeconds = 0.0f;
}

void UAeternaVanishRuleComponent::HandleScenarioStarted(FName ScenarioId)
{
	bRuleActive = (ScenarioId == RuleScenarioId);
	ResetVanishRule();
}

void UAeternaVanishRuleComponent::CacheVanishActors()
{
	VanishActors.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FName VanishTag(GetVanishTag());
	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (Actor && Actor->Tags.Contains(VanishTag))
		{
			VanishActors.Add(Actor);
		}
	}

	if (VanishActors.Num() == 0)
	{
		UE_LOG(LogAeterna, Warning, TEXT("[Vanish] Vanish 태그가 붙은 액터가 없습니다. 사라짐 규칙이 발동하지 않습니다."));
		return;
	}

	UE_LOG(LogAeterna, Log, TEXT("[Vanish] Vanish 태그 액터 %d개를 찾았습니다."), VanishActors.Num());
}

void UAeternaVanishRuleComponent::SetVanishActorsHidden(bool bHidden)
{
	TArray<AActor*> AttachedActors;

	for (AActor* VanishActor : VanishActors)
	{
		if (!VanishActor)
		{
			continue;
		}

		VanishActor->SetActorHiddenInGame(bHidden);

		// SetActorHiddenInGame은 자식으로 붙은 액터에 전파되지 않습니다.
		// 골격이 여러 액터로 조립돼 있으면 하나만 사라져 보이지 않으므로 직접 훑습니다.
		AttachedActors.Reset();
		VanishActor->GetAttachedActors(AttachedActors, true, true);
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AttachedActor)
			{
				AttachedActor->SetActorHiddenInGame(bHidden);
			}
		}
	}
}

bool UAeternaVanishRuleComponent::ShouldTriggerVanish() const
{
	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return false;
	}

	const FVector PlayerLocation = AeternaCharacter->GetActorLocation();
	for (const AActor* VanishActor : VanishActors)
	{
		if (!VanishActor)
		{
			continue;
		}

		if (FVector::Dist(PlayerLocation, VanishActor->GetActorLocation()) > TriggerRadius)
		{
			continue;
		}

		// 보고 있지 않을 때 사라지면 플레이어는 영문도 모르고 게임오버가 됩니다.
		if (IsVanishActorVisible(VanishActor))
		{
			return true;
		}
	}

	return false;
}

bool UAeternaVanishRuleComponent::IsVanishActorVisible(const AActor* VanishActor) const
{
	const UCameraComponent* CameraComponent = GetFirstPersonCamera();
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	UWorld* World = GetWorld();
	if (!VanishActor || !CameraComponent || !AeternaCharacter || !World)
	{
		return false;
	}

	FVector BoundsOrigin;
	FVector BoundsExtent;
	VanishActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

	const FVector CameraLocation = CameraComponent->GetComponentLocation();
	const FVector ToActor = BoundsOrigin - CameraLocation;
	const float DistanceToActor = ToActor.Size();
	if (DistanceToActor <= KINDA_SMALL_NUMBER)
	{
		return true;
	}

	const float CosAngle = FVector::DotProduct(ToActor / DistanceToActor, CameraComponent->GetForwardVector());
	if (CosAngle < FMath::Cos(FMath::DegreesToRadians(TriggerViewHalfAngle)))
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaVanishVisibility), true, AeternaCharacter);
	QueryParams.AddIgnoredActor(VanishActor);

	FHitResult HitResult;
	if (!World->LineTraceSingleByChannel(HitResult, CameraLocation, BoundsOrigin, ECC_Visibility, QueryParams))
	{
		return true;
	}

	// 대상 바운즈 안까지 도달했으면 가린 것으로 보지 않습니다.
	return HitResult.Distance >= DistanceToActor - BoundsExtent.GetMax();
}

bool UAeternaVanishRuleComponent::HasMovementInput() const
{
	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return false;
	}

	// 시점 회전은 이동 입력에 잡히지 않으므로 그대로 허용됩니다.
	return AeternaCharacter->GetLastMovementInputVector().SizeSquared() > (MoveInputThreshold * MoveInputThreshold);
}

void UAeternaVanishRuleComponent::BeginFreeze()
{
	VanishState = EAeternaVanishState::Frozen;
	FreezeRemainingSeconds = FreezeSeconds;
	FreezeGraceRemainingSeconds = FreezeGraceSeconds;

	SetVanishActorsHidden(true);

	UE_LOG(LogAeterna, Log, TEXT("[Vanish] 화석이 사라졌습니다. 유예 %.1fs 후 %.1fs 동안 정지 판정."), FreezeGraceSeconds, FreezeSeconds);

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_VanishRuleStarted(FreezeSeconds);
	}
}

void UAeternaVanishRuleComponent::SurviveFreeze()
{
	VanishState = EAeternaVanishState::Cooldown;
	CooldownRemainingSeconds = CooldownSeconds;
	FreezeRemainingSeconds = 0.0f;
	FreezeGraceRemainingSeconds = 0.0f;

	SetVanishActorsHidden(false);

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_VanishRuleSurvived();
	}
}

void UAeternaVanishRuleComponent::TriggerViolation()
{
	VanishState = EAeternaVanishState::Falling;
	FreezeRemainingSeconds = 0.0f;
	FreezeGraceRemainingSeconds = 0.0f;

	UE_LOG(LogAeterna, Log, TEXT("[Vanish] 정지 구간에서 이동 입력 — 위반"));

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		// 낙하 연출은 규칙 4와 공용입니다.
		if (UAeternaCameraFallComponent* CameraFallComponent = AeternaCharacter->GetCameraFallComponent())
		{
			CameraFallComponent->StartFall();
		}

		// 턱 닫히는 소리 등 연출은 BP에서 붙입니다.
		AeternaCharacter->BP_VanishRuleViolated();
	}

	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}

	bRuleActive = false;
}
