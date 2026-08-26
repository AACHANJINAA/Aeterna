// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableActor.h"
#include "AeternaCarryInteractableActor.generated.h"

/**
 * 보이는 운반 대상입니다.
 *
 * 프롬프트/레이더/상호작용 포커스는 InteractableActor 흐름을 따르고,
 * 실제 운반과 제자리 설치는 플레이어 CarryComponent가 처리합니다.
 */
UCLASS(Blueprintable)
class AETERNA_API AAeternaCarryInteractableActor : public AAeternaInteractableActor
{
	GENERATED_BODY()

public:
	AAeternaCarryInteractableActor();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	/** CarryComponent의 기존 socket 매칭과 호환하기 위해 보이는 운반 대상에 붙일 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	FName CarryTag = TEXT("Carry");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	bool bEnsureCarryTag = true;

private:
	void ApplyCarryDefaults();
};
