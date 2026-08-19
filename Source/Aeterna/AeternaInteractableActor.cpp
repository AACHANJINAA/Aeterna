// Copyright Epic Games, Inc. All Rights Reserved.

#include "AeternaInteractableActor.h"
#include "AeternaCharacter.h"
#include "Engine/Engine.h"

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
					if (GEngine)
					{
						const FString Message = FString::Printf(TEXT("Scan Already Registered: %s"), *ResolvedScanPointId.ToString());
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Message);
					}
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
		if (GEngine)
		{
			const FString Message = FString::Printf(TEXT("Interaction Complete: %s"), *GetName());
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Message);
		}

		BP_Interacted(Interactor);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Interaction failed: %s"), *GetName());
	if (GEngine)
	{
		const FString Message = FString::Printf(TEXT("Interaction Failed: %s"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Message);
	}

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
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Scan It"));
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
			InteractionInfo.PromptText = FText::FromString(TEXT("Can Charge Battery"));
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
