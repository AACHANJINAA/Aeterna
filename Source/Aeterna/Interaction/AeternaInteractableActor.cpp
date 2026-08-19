// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/AeternaInteractableActor.h"

#include "Player/AeternaCharacter.h"

AAeternaInteractableActor::AAeternaInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAeternaInteractableActor::Interact_Implementation(AActor* Interactor)
{
	PerformInteraction(Interactor);
}

bool AAeternaInteractableActor::PerformInteraction(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return false;
	}

	bool bHandledInteraction = false;

	if (AAeternaCharacter* AeternaCharacter = Cast<AAeternaCharacter>(Interactor))
	{
		switch (InteractionType)
		{
		case EAeternaInteractionType::Scan:
			{
				const FName ResolvedScanPointId = ScanPointId.IsNone() ? GetFName() : ScanPointId;
				if (AeternaCharacter->HasScannedPoint(ResolvedScanPointId))
				{
					UE_LOG(LogTemp, Log, TEXT("Scan already registered: %s"), *ResolvedScanPointId.ToString());
					return false;
				}

				bHandledInteraction = AeternaCharacter->RegisterScanPoint(ResolvedScanPointId);
			}
			break;

		case EAeternaInteractionType::Charge:
			AeternaCharacter->AddPlayerBattery(BatteryChargeAmount);
			bHandledInteraction = true;
			break;

		default:
			bHandledInteraction = true;
			break;
		}
	}

	if (bHandledInteraction && !bRepeatable)
	{
		bInteractionCompleted = true;
	}

	if (bHandledInteraction)
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction complete: %s"), *GetName());

		BP_Interacted(Interactor);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Interaction failed: %s"), *GetName());

	return false;
}

bool AAeternaInteractableActor::CanInteract_Implementation(AActor* Interactor) const
{
	return bRepeatable || !bInteractionCompleted;
}

FAeternaInteractionInfo AAeternaInteractableActor::GetInteractionInfo_Implementation(AActor* Interactor) const
{
	FAeternaInteractionInfo InteractionInfo;
	InteractionInfo.Type = CanInteract_Implementation(Interactor) ? InteractionType : EAeternaInteractionType::None;
	InteractionInfo.PromptText = InteractionPromptText;

	if (InteractionInfo.PromptText.IsEmpty())
	{
		switch (InteractionInfo.Type)
		{
		case EAeternaInteractionType::Scan:
			InteractionInfo.PromptText = FText::FromString(TEXT("Scan Exhibit"));
			break;

		case EAeternaInteractionType::Read:
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Read It"));
			break;

		case EAeternaInteractionType::Pickup:
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Pick It Up"));
			break;

		case EAeternaInteractionType::Install:
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Install It"));
			break;

		case EAeternaInteractionType::Charge:
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Charge It"));
			break;

		case EAeternaInteractionType::UseTerminal:
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Use Terminal"));
			break;

		default:
			InteractionInfo.PromptText = FText::GetEmpty();
			break;
		}
	}

	return InteractionInfo;
}
