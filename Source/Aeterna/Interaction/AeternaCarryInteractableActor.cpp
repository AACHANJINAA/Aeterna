// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/AeternaCarryInteractableActor.h"

AAeternaCarryInteractableActor::AAeternaCarryInteractableActor()
{
	ApplyCarryDefaults();
}

void AAeternaCarryInteractableActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyCarryDefaults();
}

void AAeternaCarryInteractableActor::ApplyCarryDefaults()
{
	InteractionType = EAeternaInteractionType::Pickup;
	InteractionPromptText = FText::FromString(TEXT("제자리 운반 가능"));
	bRepeatable = false;
	bCountsAsScanPoint = false;
	ActiveScenarioIds.Reset();
	ActiveScenarioIds.Add(TEXT("S02_GrandHallFossil"));

	if (bEnsureCarryTag && !CarryTag.IsNone() && !Tags.Contains(CarryTag))
	{
		Tags.Add(CarryTag);
	}
}
