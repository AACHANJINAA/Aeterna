// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableActor.h"
#include "AeternaNotebookPickupActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * 시작 구간에서 주워 수첩 UI를 해금하는 노트 픽업입니다.
 *
 * BP에서 NotebookMesh에 SM_cardboard_objects_Baked_002를 지정하고,
 * L_Showcase의 cardboard_objects_Baked_5 위치에 배치해서 씁니다.
 */
UCLASS()
class AETERNA_API AAeternaNotebookPickupActor : public AAeternaInteractableActor
{
	GENERATED_BODY()

public:
	AAeternaNotebookPickupActor();

	virtual void BeginPlay() override;
	virtual void ResetInteraction() override;

protected:
	virtual void OnInteractionPerformed(AActor* Interactor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> NotebookMesh;

	/** 획득 후 월드에서 노트 메시와 상호작용 판정을 숨깁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook")
	bool bHideAfterPickup = true;

private:
	bool ShouldShowForCurrentScenario() const;
	void SetPickupVisualEnabled(bool bEnabled);
};
