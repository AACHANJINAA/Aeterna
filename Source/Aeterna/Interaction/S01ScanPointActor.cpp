// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/S01ScanPointActor.h"

#include "Components/BoxComponent.h"

AS01ScanPointActor::AS01ScanPointActor()
{
	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	SetRootComponent(InteractionBounds);
	InteractionBounds->SetBoxExtent(FVector(45.0f, 45.0f, 45.0f));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	ApplyScanPointDefaults();
}

void AS01ScanPointActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyScanPointDefaults();
}

void AS01ScanPointActor::ApplyScanPointDefaults()
{
	bRepeatable = false;
	ActiveScenarioIds.Reset();
	ActiveScenarioIds.Add(TEXT("S01_Handover"));

	switch (ScanPointKind)
	{
	case ES01ScanPointKind::DisplayCaseFossil:
		InteractionType = EAeternaInteractionType::Scan;
		InteractionPromptText = FText::FromString(TEXT("전시물 스캔"));
		ScanPointId = TEXT("S01_Scan_DisplayFossil");
		bCountsAsScanPoint = false;
		break;

	case ES01ScanPointKind::Battery:
		InteractionType = EAeternaInteractionType::Charge;
		InteractionPromptText = FText::FromString(TEXT("배터리 회수"));
		ScanPointId = TEXT("S01_Scan_Battery");
		bCountsAsScanPoint = true;
		BatteryChargeAmount = 60.0f;
		break;

	case ES01ScanPointKind::TRexInfoSign:
		InteractionType = EAeternaInteractionType::Scan;
		InteractionPromptText = FText::FromString(TEXT("알림판 스캔"));
		ScanPointId = TEXT("S01_Scan_TRexSign");
		bCountsAsScanPoint = false;
		break;
	}
}
