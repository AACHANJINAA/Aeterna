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

	RestoreViewTarget();

	Super::EndPlay(EndPlayReason);
}

void UAeternaVanishRuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 위반 후에는 낙하 연출만 진행합니다.
	if (VanishState == EAeternaVanishState::Falling)
	{
		UpdateCameraFall(DeltaTime);
		return;
	}

	if (!bRuleActive)
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
	RestoreViewTarget();
	SetVanishActorsHidden(false);

	VanishState = EAeternaVanishState::Idle;
	FreezeRemainingSeconds = 0.0f;
	FreezeGraceRemainingSeconds = 0.0f;
	CooldownRemainingSeconds = 0.0f;
	FallElapsedSeconds = 0.0f;
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

	BeginCameraFall();

	// 턱 닫히는 소리 등 연출은 BP에서 붙입니다.
	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_VanishRuleViolated();
	}

	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}

	bRuleActive = false;
}

void UAeternaVanishRuleComponent::BeginCameraFall()
{
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	UWorld* World = GetWorld();
	if (!AeternaCharacter || !CameraComponent || !World)
	{
		return;
	}

	// 시점을 다른 카메라로 넘기면 1인칭 렌더링 설정이 날아가 화각이 튀고
	// 3인칭 메시가 드러납니다. 보고 있던 카메라를 그대로 떼어내 쓰러뜨립니다.
	SavedCameraParent = CameraComponent->GetAttachParent();
	SavedCameraSocket = CameraComponent->GetAttachSocketName();
	SavedCameraRelativeTransform = CameraComponent->GetRelativeTransform();
	bSavedUsePawnControlRotation = CameraComponent->bUsePawnControlRotation;

	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	FallStartLocation = CameraComponent->GetComponentLocation();
	FallStartRotation = CameraComponent->GetComponentRotation();

	// 바닥을 찾아 그 위로 눕힙니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaVanishFallTrace), false, AeternaCharacter);

	FHitResult HitResult;
	const FVector TraceEnd = FallStartLocation - FVector(0.0f, 0.0f, 500.0f);
	FallTargetLocation = World->LineTraceSingleByChannel(HitResult, FallStartLocation, TraceEnd, ECC_Visibility, QueryParams)
		? HitResult.ImpactPoint + FVector(0.0f, 0.0f, FallHeightAboveFloor)
		: FallStartLocation - FVector(0.0f, 0.0f, 100.0f);

	FallTargetRotation = FRotator(-10.0f, FallStartRotation.Yaw, FallRollDegrees);

	// 쓰러진 카메라가 자기 몸을 올려다보게 되므로 메시를 감춥니다.
	SetCharacterMeshesVisible(false);

	FallElapsedSeconds = 0.0f;
}

void UAeternaVanishRuleComponent::UpdateCameraFall(float DeltaTime)
{
	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	if (!CameraComponent || !SavedCameraParent.IsValid())
	{
		return;
	}

	FallElapsedSeconds += DeltaTime;

	const float Progress = (FallSeconds > KINDA_SMALL_NUMBER)
		? FMath::Clamp(FallElapsedSeconds / FallSeconds, 0.0f, 1.0f)
		: 1.0f;

	// 처음엔 천천히 기울다 바닥에 가까워질수록 빨라집니다.
	const float EasedProgress = FMath::InterpEaseIn(0.0f, 1.0f, Progress, 2.0f);

	CameraComponent->SetWorldLocationAndRotation(
		FMath::Lerp(FallStartLocation, FallTargetLocation, EasedProgress),
		FMath::Lerp(FallStartRotation, FallTargetRotation, EasedProgress));
}

void UAeternaVanishRuleComponent::RestoreViewTarget()
{
	USceneComponent* CameraParent = SavedCameraParent.Get();
	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	if (!CameraParent || !CameraComponent)
	{
		return;
	}

	// 메시 가시성 복구가 카메라 부착보다 먼저여야 합니다.
	// SetVisibility는 자식까지 전파되는데, 카메라를 먼저 붙이면 그 자식인
	// 헤드램프까지 강제로 켜져 버립니다.
	SetCharacterMeshesVisible(true);

	CameraComponent->AttachToComponent(CameraParent, FAttachmentTransformRules::KeepRelativeTransform, SavedCameraSocket);
	CameraComponent->SetRelativeTransform(SavedCameraRelativeTransform);
	CameraComponent->bUsePawnControlRotation = bSavedUsePawnControlRotation;

	SavedCameraParent.Reset();
}

void UAeternaVanishRuleComponent::SetCharacterMeshesVisible(bool bVisible)
{
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return;
	}

	if (USkeletalMeshComponent* ThirdPersonMesh = AeternaCharacter->GetMesh())
	{
		ThirdPersonMesh->SetVisibility(bVisible, true);
	}

	if (USkeletalMeshComponent* FirstPersonMesh = AeternaCharacter->GetFirstPersonMesh())
	{
		FirstPersonMesh->SetVisibility(bVisible, true);
	}
}
