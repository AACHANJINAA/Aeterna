// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/AeternaNotebookPickupActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Player/AeternaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAeternaNotebookPickupActor::AAeternaNotebookPickupActor()
{
	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	SetRootComponent(InteractionBounds);

	InteractionBounds->SetBoxExtent(FVector(28.0f, 22.0f, 10.0f));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	NotebookMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NotebookMesh"));
	NotebookMesh->SetupAttachment(InteractionBounds);
	NotebookMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> NotebookMeshAsset(TEXT("/Game/DetectiveOffice/StaticMeshes/SM_cardboard_objects_Baked_002.SM_cardboard_objects_Baked_002"));
	if (NotebookMeshAsset.Succeeded())
	{
		NotebookMesh->SetStaticMesh(NotebookMeshAsset.Object);
	}

	InteractionType = EAeternaInteractionType::Pickup;
	InteractionPromptText = FText::FromString(TEXT("노트 줍기"));
	bRepeatable = false;
	bCountsAsScanPoint = false;
	ScanPointId = NAME_None;
	ActiveScenarioIds.Reset();
	ActiveScenarioIds.Add(TEXT("S01_Handover"));
}

void AAeternaNotebookPickupActor::BeginPlay()
{
	Super::BeginPlay();
	SetPickupVisualEnabled(!IsInteractionCompleted() && ShouldShowForCurrentScenario());
}

void AAeternaNotebookPickupActor::ResetInteraction()
{
	Super::ResetInteraction();

	SetPickupVisualEnabled(ShouldShowForCurrentScenario());
}

void AAeternaNotebookPickupActor::OnInteractionPerformed(AActor* Interactor)
{
	Super::OnInteractionPerformed(Interactor);

	if (AAeternaCharacter* AeternaCharacter = Cast<AAeternaCharacter>(Interactor))
	{
		AeternaCharacter->AcquireNotebook();
	}

	if (bHideAfterPickup)
	{
		SetPickupVisualEnabled(false);
	}
}

bool AAeternaNotebookPickupActor::ShouldShowForCurrentScenario() const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	const FName CurrentScenarioId = ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;
	return CurrentScenarioId.IsNone() || IsActiveForScenario(CurrentScenarioId);
}

void AAeternaNotebookPickupActor::SetPickupVisualEnabled(bool bEnabled)
{
	if (NotebookMesh)
	{
		NotebookMesh->SetVisibility(bEnabled, true);
		NotebookMesh->SetHiddenInGame(!bEnabled, true);
	}

	if (InteractionBounds)
	{
		InteractionBounds->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}
