// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableActor.h"
#include "AeternaBatteryPickupActor.generated.h"

class UBoxComponent;

/**
 *  밤을 가리지 않고 놓는 헤드램프 충전용 배터리입니다.
 *
 *  밤1의 AS01ScanPointActor(Battery)와 역할이 다릅니다. 그쪽은 목표 3곳 중
 *  하나를 겸하는 튜토리얼 마커라 진행도에 1을 더하지만, 이 액터는 자원일 뿐
 *  진행도에 관여하지 않습니다. 밤2의 목표가 뼈 12개를 같은 집합으로 세기
 *  때문에, 자원 배터리가 진행도를 건드리면 뼈 11개만 맞춰도 밤이 끝납니다.
 */
UCLASS()
class AETERNA_API AAeternaBatteryPickupActor : public AAeternaInteractableActor
{
	GENERATED_BODY()

public:
	AAeternaBatteryPickupActor();

protected:
	/** 조준 판정 영역입니다. 배치한 메시 크기에 맞춰 줄이십시오. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> InteractionBounds;
};
