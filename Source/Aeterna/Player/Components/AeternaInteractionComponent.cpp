// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaInteractionComponent.h"

#include "Interaction/AeternaInteractableActor.h"

#include "Camera/CameraComponent.h"
#include "Engine/OverlapResult.h"

UAeternaInteractionComponent::UAeternaInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAeternaInteractionComponent::InitializeInteraction(UCameraComponent* InCameraComponent)
{
	CameraComponent = InCameraComponent;
}

bool UAeternaInteractionComponent::TryAcceptInteractable(AActor* CandidateActor, AActor* Interactor, FAeternaInteractionInfo& OutInteractionInfo) const
{
	if (!CandidateActor || !CandidateActor->GetClass()->ImplementsInterface(UAeternaInteractableInterface::StaticClass()))
	{
		return false;
	}

	if (AAeternaInteractableActor* NativeInteractable = Cast<AAeternaInteractableActor>(CandidateActor))
	{
		OutInteractionInfo = NativeInteractable->GetInteractionInfo_Implementation(Interactor);
	}
	else
	{
		OutInteractionInfo = IAeternaInteractableInterface::Execute_GetInteractionInfo(CandidateActor, Interactor);
	}

	return OutInteractionInfo.Type != EAeternaInteractionType::None || !OutInteractionInfo.PromptText.IsEmpty();
}

AActor* UAeternaInteractionComponent::FindInteractableAroundCamera(AActor* Interactor, FAeternaInteractionInfo& OutInteractionInfo) const
{
	UWorld* World = GetWorld();
	if (!World || !CameraComponent || InsideBoundsProbeRadius <= 0.0f)
	{
		return nullptr;
	}

	const FVector CameraLocation = CameraComponent->GetComponentLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaInteractInsideBounds), false, Interactor);
	if (!World->OverlapMultiByChannel(Overlaps, CameraLocation, FQuat::Identity, ECC_Visibility,
			FCollisionShape::MakeSphere(InsideBoundsProbeRadius), QueryParams))
	{
		return nullptr;
	}

	// 여러 개가 겹치면 가장 가까운 것을 고릅니다.
	AActor* BestActor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlapActor = Overlap.GetActor();
		FAeternaInteractionInfo CandidateInfo;
		if (!TryAcceptInteractable(OverlapActor, Interactor, CandidateInfo))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OverlapActor->GetActorLocation(), CameraLocation);
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestActor = OverlapActor;
			OutInteractionInfo = CandidateInfo;
		}
	}

	return BestActor;
}

bool UAeternaInteractionComponent::UpdateFocusedInteractable(AActor* Interactor)
{
	AActor* NewFocusedActor = nullptr;
	FAeternaInteractionInfo NewInteractionInfo;

	if (CameraComponent && GetWorld())
	{
		const FVector TraceStart = CameraComponent->GetComponentLocation();
		const FVector TraceEnd = TraceStart + CameraComponent->GetForwardVector() * InteractionTraceDistance;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaInteractFocusTrace), false, Interactor);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			if (TryAcceptInteractable(HitResult.GetActor(), Interactor, NewInteractionInfo))
			{
				NewFocusedActor = HitResult.GetActor();
			}
		}

		// 라인트레이스는 시작점이 도형 안에 있으면 그 도형을 맞히지 못합니다.
		// 작은 오브젝트에 얼굴을 들이밀면 상호작용 박스 안으로 카메라가 들어가
		// 대상을 통째로 놓치므로, 실패했을 때만 주변을 한 번 더 훑습니다.
		if (!NewFocusedActor)
		{
			NewInteractionInfo = FAeternaInteractionInfo();
			NewFocusedActor = FindInteractableAroundCamera(Interactor, NewInteractionInfo);
		}
	}

	const bool bFocusChanged = FocusedInteractableActor != NewFocusedActor;
	FocusedInteractableActor = NewFocusedActor;
	FocusedInteractionInfo = NewInteractionInfo;

	return bFocusChanged;
}

bool UAeternaInteractionComponent::TryInteract(AActor* Interactor)
{
	UpdateFocusedInteractable(Interactor);

	AActor* HitActor = FocusedInteractableActor.Get();
	if (!HitActor)
	{
		return false;
	}

	if (AAeternaInteractableActor* NativeInteractable = Cast<AAeternaInteractableActor>(HitActor))
	{
		return NativeInteractable->PerformInteraction(Interactor);
	}

	IAeternaInteractableInterface::Execute_Interact(HitActor, Interactor);
	return true;
}
