// Copyright Epic Games, Inc. All Rights Reserved.

#include "AeternaInteractionComponent.h"
#include "AeternaInteractableActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"

UAeternaInteractionComponent::UAeternaInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAeternaInteractionComponent::InitializeInteraction(UCameraComponent* InCameraComponent)
{
	CameraComponent = InCameraComponent;
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
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && HitActor->GetClass()->ImplementsInterface(UAeternaInteractableInterface::StaticClass()))
			{
				if (AAeternaInteractableActor* NativeInteractable = Cast<AAeternaInteractableActor>(HitActor))
				{
					NewInteractionInfo = NativeInteractable->GetInteractionInfo_Implementation(Interactor);
				}
				else
				{
					NewInteractionInfo = IAeternaInteractableInterface::Execute_GetInteractionInfo(HitActor, Interactor);
				}

				if (NewInteractionInfo.Type != EAeternaInteractionType::None || !NewInteractionInfo.PromptText.IsEmpty())
				{
					NewFocusedActor = HitActor;
				}
			}
		}
	}

	const bool bFocusChanged = FocusedInteractableActor != NewFocusedActor;
	FocusedInteractableActor = NewFocusedActor;
	FocusedInteractionInfo = NewInteractionInfo;

	if (bFocusChanged && GEngine)
	{
		if (FocusedInteractableActor)
		{
			const FText PromptText = FocusedInteractionInfo.PromptText.IsEmpty()
				? FText::FromString(TEXT("Interact"))
				: FocusedInteractionInfo.PromptText;
			const FString PromptMessage = FString::Printf(TEXT("[E] %s"), *PromptText.ToString());
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, PromptMessage);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Silver, TEXT("No Interaction"));
		}
	}

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
