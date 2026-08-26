// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/AeternaBatteryPickupActor.h"

#include "Components/BoxComponent.h"

AAeternaBatteryPickupActor::AAeternaBatteryPickupActor()
{
	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	SetRootComponent(InteractionBounds);

	// 배터리는 작은 물건입니다. 박스가 메시보다 크면 허공을 겨눠도 반응하고,
	// 카메라가 박스 안으로 들어가 대상을 놓치기도 쉬워집니다.
	InteractionBounds->SetBoxExtent(FVector(15.0f, 15.0f, 15.0f));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	InteractionType = EAeternaInteractionType::Charge;
	InteractionPromptText = FText::FromString(TEXT("배터리 회수"));
	BatteryChargeAmount = 60.0f;
	bRepeatable = false;

	// 목표 진행도에는 관여하지 않습니다.
	bCountsAsScanPoint = false;
	ScanPointId = NAME_None;

	// 비워두면 모든 밤에서 동작합니다 (IsActiveForScenario).
	ActiveScenarioIds.Reset();
}
