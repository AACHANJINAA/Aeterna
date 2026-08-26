// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/AeternaInteractableActor.h"

#include "Core/ScenarioManagerSubsystem.h"
#include "Player/AeternaCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

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
			bHandledInteraction = RegisterScanProgress(AeternaCharacter);
			break;

		case EAeternaInteractionType::Charge:
			AeternaCharacter->AddPlayerBattery(BatteryChargeAmount);
			if (bCountsAsScanPoint)
			{
				RegisterScanProgress(AeternaCharacter);
			}
			bHandledInteraction = true;
			break;

		default:
			if (bCountsAsScanPoint)
			{
				RegisterScanProgress(AeternaCharacter);
			}
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
		PlayInteractionSound();
		BP_Interacted(Interactor);
		return true;
	}

	return false;
}

bool AAeternaInteractableActor::RegisterScanProgress(AAeternaCharacter* AeternaCharacter)
{
	if (!AeternaCharacter)
	{
		return false;
	}

	const FName ResolvedScanPointId = ScanPointId.IsNone() ? GetFName() : ScanPointId;
	if (AeternaCharacter->HasScannedPoint(ResolvedScanPointId))
	{
		return false;
	}

	return AeternaCharacter->RegisterScanPoint(ResolvedScanPointId);
}

bool AAeternaInteractableActor::CanInteract_Implementation(AActor* Interactor) const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	const FName CurrentScenarioId = ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;
	return IsActiveForScenario(CurrentScenarioId) && (bRepeatable || !bInteractionCompleted);
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

void AAeternaInteractableActor::PlayInteractionSound()
{
	if (!InteractionSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, InteractionSound, GetActorLocation(), InteractionSoundVolume);
}
