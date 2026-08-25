// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaCarryComponent.h"

#include "Aeterna.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Player/AeternaCharacter.h"

UAeternaCarryComponent::UAeternaCarryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAeternaCarryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsCarrying())
	{
		UpdateCarriedTransform(DeltaTime);

		// 자석 판정은 매 프레임 확인합니다. E를 놓지 않아도 자리에 닿으면 설치됩니다.
		TrySnapToSocket();
		return;
	}

	UpdateCarryTarget();
}

bool UAeternaCarryComponent::TryStartCarry()
{
	if (IsCarrying())
	{
		return false;
	}

	AActor* TargetActor = FindCarryTargetUnderCrosshair();
	if (!TargetActor)
	{
		return false;
	}

	const FName CarryId = GetCarryId(TargetActor);
	if (CarryId.IsNone() || InstalledIds.Contains(CarryId))
	{
		return false;
	}

	// 자리를 미리 고정하지는 않지만, 받아줄 자리가 하나도 없으면 배치 실수이므로 알립니다.
	float NearestSocketDistance = 0.0f;
	if (!FindNearestAvailableSocket(TargetActor, NearestSocketDistance))
	{
		UE_LOG(LogAeterna, Warning,
			TEXT("[Carry] %s가 들어갈 자리가 없습니다. 같은 메시에 Carry 태그가 붙고 이번 밤에 숨겨진 액터가 하나 이상 있어야 합니다."),
			*GetNameSafe(TargetActor));
		return false;
	}

	CarriedActor = TargetActor;
	CarriedId = CarryId;

	EnsureMovable(CarriedActor);

	// 운반 중에는 플레이어와 벽을 밀지 않도록 콜리전을 끕니다.
	bCarriedCollisionEnabled = CarriedActor->GetActorEnableCollision();
	CarriedActor->SetActorEnableCollision(false);

	SetCarryTargetActor(nullptr);

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_CarryStarted(CarriedActor, CarriedId);
	}

	return true;
}

void UAeternaCarryComponent::StopCarry()
{
	if (!IsCarrying())
	{
		return;
	}

	// 놓기 직전에 한 번 더 확인해, 자리에 닿은 채로 놓으면 설치로 처리합니다.
	if (TrySnapToSocket())
	{
		return;
	}

	PlaceCarriedActorOnGround();

	AActor* DroppedActor = CarriedActor;
	const FName DroppedId = CarriedId;
	ReleaseCarriedActor();

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_CarryStopped(DroppedActor, DroppedId);
	}
}

void UAeternaCarryComponent::UpdateCarryTarget()
{
	AActor* TargetActor = FindCarryTargetUnderCrosshair();
	if (TargetActor && InstalledIds.Contains(GetCarryId(TargetActor)))
	{
		TargetActor = nullptr;
	}

	SetCarryTargetActor(TargetActor);
}

void UAeternaCarryComponent::ResetInstallProgress()
{
	InstalledIds.Reset();
	UsedSocketActors.Reset();
}

AActor* UAeternaCarryComponent::FindCarryTargetUnderCrosshair() const
{
	const UCameraComponent* CameraComponent = GetFirstPersonCamera();
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	UWorld* World = GetWorld();
	if (!CameraComponent || !AeternaCharacter || !World)
	{
		return nullptr;
	}

	const FVector TraceStart = CameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + CameraComponent->GetForwardVector() * CarryTraceDistance;

	// 1) 복합 콜리전 라인 트레이스. 단순 콜리전이 없는 프롭 메시도 삼각형 단위로 잡힙니다.
	FCollisionQueryParams LineQueryParams(SCENE_QUERY_STAT(AeternaCarryLineTrace), true, AeternaCharacter);

	TArray<FHitResult> LineHits;
	World->LineTraceMultiByChannel(LineHits, TraceStart, TraceEnd, ECC_Visibility, LineQueryParams);
	for (const FHitResult& HitResult : LineHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && IsCarryTagged(HitActor) && !HitActor->IsHidden())
		{
			return HitActor;
		}
	}

	// 2) 스피어 스윕으로 한 번 더. 조준이 살짝 빗나가도 잡히지만 단순 콜리전만 봅니다.
	if (CarryTraceRadius > 0.0f)
	{
		FCollisionQueryParams SweepQueryParams(SCENE_QUERY_STAT(AeternaCarrySweepTrace), false, AeternaCharacter);

		TArray<FHitResult> SweepHits;
		World->SweepMultiByChannel(SweepHits, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CarryTraceRadius), SweepQueryParams);
		for (const FHitResult& HitResult : SweepHits)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && IsCarryTagged(HitActor) && !HitActor->IsHidden())
			{
				return HitActor;
			}
		}
	}

	// 3) 콜리전이 아예 없는 메시를 위한 바운즈 판정. 박물관 프롭은 콜리전 설정이 제각각입니다.
	return bAllowBoundsFallback ? FindCarryTargetByBounds() : nullptr;
}

AActor* UAeternaCarryComponent::FindCarryTargetByBounds() const
{
	const UCameraComponent* CameraComponent = GetFirstPersonCamera();
	UWorld* World = GetWorld();
	if (!CameraComponent || !World)
	{
		return nullptr;
	}

	const FVector RayStart = CameraComponent->GetComponentLocation();
	const FVector RayDirection = CameraComponent->GetForwardVector();

	AActor* BestActor = nullptr;
	float BestDistanceAlongRay = MAX_flt;

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;

		// 지금 밤에 존재하지 않는 쪽(자리)은 집기 후보에서 뺍니다.
		if (!Actor || !IsCarryTagged(Actor) || Actor->IsHidden())
		{
			continue;
		}

		FVector BoundsOrigin;
		FVector BoundsExtent;
		Actor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

		const float DistanceAlongRay = FVector::DotProduct(BoundsOrigin - RayStart, RayDirection);
		if (DistanceAlongRay < 0.0f || DistanceAlongRay > CarryTraceDistance || DistanceAlongRay >= BestDistanceAlongRay)
		{
			continue;
		}

		const FVector ClosestPointOnRay = RayStart + RayDirection * DistanceAlongRay;
		if (FVector::Dist(ClosestPointOnRay, BoundsOrigin) > BoundsExtent.GetMax() + BoundsFallbackTolerance)
		{
			continue;
		}

		// 벽 너머의 물체를 집지 않도록 가림 판정을 한 번 합니다.
		if (IsCarryTargetOccluded(Actor, RayStart))
		{
			continue;
		}

		BestActor = Actor;
		BestDistanceAlongRay = DistanceAlongRay;
	}

	return BestActor;
}

bool UAeternaCarryComponent::IsCarryTargetOccluded(const AActor* TargetActor, const FVector& RayStart) const
{
	UWorld* World = GetWorld();
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!World || !TargetActor || !AeternaCharacter)
	{
		return false;
	}

	FVector BoundsOrigin;
	FVector BoundsExtent;
	TargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaCarryOcclusion), true, AeternaCharacter);
	QueryParams.AddIgnoredActor(TargetActor);

	FHitResult HitResult;
	if (!World->LineTraceSingleByChannel(HitResult, RayStart, BoundsOrigin, ECC_Visibility, QueryParams))
	{
		return false;
	}

	// 대상 바운즈 안까지 파고든 히트는 가림으로 보지 않습니다 (전시대 받침 등).
	return FVector::Dist(HitResult.ImpactPoint, BoundsOrigin) > BoundsExtent.GetMax();
}

AActor* UAeternaCarryComponent::FindNearestAvailableSocket(const AActor* SourceActor, float& OutDistance) const
{
	UWorld* World = GetWorld();
	OutDistance = MAX_flt;
	if (!World || !SourceActor)
	{
		return nullptr;
	}

	const FVector SourceLocation = SourceActor->GetActorLocation();

	AActor* BestSocketActor = nullptr;

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!IsSocketCandidate(Actor, SourceActor))
		{
			continue;
		}

		const float Distance = FVector::Dist(SourceLocation, Actor->GetActorLocation());
		if (Distance < OutDistance)
		{
			OutDistance = Distance;
			BestSocketActor = Actor;
		}
	}

	return BestSocketActor;
}

bool UAeternaCarryComponent::IsSocketCandidate(const AActor* SocketActor, const AActor* SourceActor) const
{
	if (!SocketActor || SocketActor == SourceActor || !IsCarryTagged(SocketActor))
	{
		return false;
	}

	// 자리는 이번 밤에 숨겨져 있는 쪽입니다. 지금 보이는 물체는 서로의 자리가 될 수 없습니다.
	if (!SocketActor->IsHidden())
	{
		return false;
	}

	if (UsedSocketActors.Contains(const_cast<AActor*>(SocketActor)))
	{
		return false;
	}

	// Carry_<ID>로 지정했으면 같은 ID끼리만, 아니면 같은 메시끼리 맞춥니다.
	const FName SourceExplicitId = GetExplicitCarryId(SourceActor);
	if (!SourceExplicitId.IsNone())
	{
		return GetExplicitCarryId(SocketActor) == SourceExplicitId;
	}

	if (!GetExplicitCarryId(SocketActor).IsNone())
	{
		return false;
	}

	const UStaticMesh* SourceMesh = GetCarryStaticMesh(SourceActor);
	return SourceMesh && GetCarryStaticMesh(SocketActor) == SourceMesh;
}

void UAeternaCarryComponent::UpdateCarriedTransform(float DeltaTime)
{
	const UCameraComponent* CameraComponent = GetFirstPersonCamera();
	if (!CameraComponent || !CarriedActor)
	{
		return;
	}

	const FTransform CameraTransform = CameraComponent->GetComponentTransform();
	const FVector TargetLocation = CameraTransform.TransformPosition(CarryOffset);
	const FRotator TargetRotation = CameraTransform.Rotator();

	const FVector NewLocation = FMath::VInterpTo(CarriedActor->GetActorLocation(), TargetLocation, DeltaTime, CarryInterpSpeed);
	const FRotator NewRotation = FMath::RInterpTo(CarriedActor->GetActorRotation(), TargetRotation, DeltaTime, CarryRotationInterpSpeed);

	CarriedActor->SetActorLocationAndRotation(NewLocation, NewRotation);
}

bool UAeternaCarryComponent::TrySnapToSocket()
{
	if (!CarriedActor)
	{
		return false;
	}

	float SocketDistance = 0.0f;
	AActor* SocketActor = FindNearestAvailableSocket(CarriedActor, SocketDistance);
	if (!SocketActor || SocketDistance > SnapDistance)
	{
		return false;
	}

	InstallCarriedActor(SocketActor);
	return true;
}

void UAeternaCarryComponent::InstallCarriedActor(AActor* SocketActor)
{
	AActor* InstalledActor = CarriedActor;
	const FName InstalledId = CarriedId;

	InstalledActor->SetActorTransform(SocketActor->GetActorTransform());
	InstalledIds.Add(InstalledId);
	UsedSocketActors.Add(SocketActor);

	ReleaseCarriedActor();

	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return;
	}

	// 진행도는 기존 스캔 진행도 컴포넌트를 그대로 씁니다 (목표 지점 N개 달성이라는 구조가 같습니다).
	AeternaCharacter->RegisterScanPoint(InstalledId);
	AeternaCharacter->BP_CarryInstalled(InstalledActor, InstalledId);
}

void UAeternaCarryComponent::PlaceCarriedActorOnGround()
{
	UWorld* World = GetWorld();
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!World || !CarriedActor || !AeternaCharacter)
	{
		return;
	}

	FVector BoundsOrigin;
	FVector BoundsExtent;
	CarriedActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

	const FVector TraceStart = BoundsOrigin;
	const FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, GroundTraceDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaCarryDropTrace), false, AeternaCharacter);
	QueryParams.AddIgnoredActor(CarriedActor);

	FHitResult HitResult;
	if (!World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return;
	}

	// 액터 원점이 바운즈 중심과 다를 수 있으므로 그 차이를 유지한 채 바닥에 얹습니다.
	const FVector PivotOffset = CarriedActor->GetActorLocation() - BoundsOrigin;
	CarriedActor->SetActorLocation(HitResult.ImpactPoint + FVector(0.0f, 0.0f, BoundsExtent.Z) + PivotOffset);
}

void UAeternaCarryComponent::ReleaseCarriedActor()
{
	if (CarriedActor)
	{
		CarriedActor->SetActorEnableCollision(bCarriedCollisionEnabled);
	}

	CarriedActor = nullptr;
	CarriedId = NAME_None;
	bCarriedCollisionEnabled = true;
}

void UAeternaCarryComponent::SetCarryTargetActor(AActor* NewTargetActor)
{
	if (CarryTargetActor == NewTargetActor)
	{
		return;
	}

	CarryTargetActor = NewTargetActor;

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_CarryTargetChanged(CarryTargetActor, GetCarryId(CarryTargetActor));
	}
}

bool UAeternaCarryComponent::IsCarryTagged(const AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	const FName BaseTag(GetCarryTag());
	const FString TagPrefix = GetCarryTagPrefix();
	for (const FName& Tag : Actor->Tags)
	{
		if (Tag == BaseTag || Tag.ToString().StartsWith(TagPrefix, ESearchCase::CaseSensitive))
		{
			return true;
		}
	}

	return false;
}

FName UAeternaCarryComponent::GetExplicitCarryId(const AActor* Actor)
{
	if (!Actor)
	{
		return NAME_None;
	}

	const FString TagPrefix = GetCarryTagPrefix();
	for (const FName& Tag : Actor->Tags)
	{
		if (Tag.ToString().StartsWith(TagPrefix, ESearchCase::CaseSensitive))
		{
			return Tag;
		}
	}

	return NAME_None;
}

FName UAeternaCarryComponent::GetCarryId(const AActor* Actor)
{
	if (!IsCarryTagged(Actor))
	{
		return NAME_None;
	}

	// 진행도 기록 키는 액터마다 고유해야 하므로 액터 이름을 씁니다.
	return Actor->GetFName();
}

const UStaticMesh* UAeternaCarryComponent::GetCarryStaticMesh(const AActor* Actor)
{
	const UStaticMeshComponent* StaticMeshComponent = Actor ? Actor->FindComponentByClass<UStaticMeshComponent>() : nullptr;
	return StaticMeshComponent ? StaticMeshComponent->GetStaticMesh() : nullptr;
}

void UAeternaCarryComponent::EnsureMovable(AActor* Actor)
{
	USceneComponent* RootComponent = Actor ? Actor->GetRootComponent() : nullptr;
	if (!RootComponent || RootComponent->Mobility == EComponentMobility::Movable)
	{
		return;
	}

	// 배치 시 Static으로 둬도 들 수 있게, 처음 들 때 한 번 Movable로 바꿉니다.
	RootComponent->SetMobility(EComponentMobility::Movable);
	UE_LOG(LogAeterna, Verbose, TEXT("[Carry] %s의 Mobility를 Movable로 바꿨습니다."), *GetNameSafe(Actor));
}
