// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaInteractionComponent.generated.h"

class UCameraComponent;

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaInteractionComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaInteractionComponent();

	void InitializeInteraction(UCameraComponent* InCameraComponent);

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool UpdateFocusedInteractable(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract(AActor* Interactor);

	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedInteractableActor() const { return FocusedInteractableActor.Get(); }

	UFUNCTION(BlueprintPure, Category="Interaction")
	FAeternaInteractionInfo GetFocusedInteractionInfo() const { return FocusedInteractionInfo; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="0.0", Units="cm"))
	float InteractionTraceDistance = 300.0f;

	/**
	 *  카메라가 상호작용 콜리전 안으로 들어갔을 때 대상을 놓치지 않기 위한 반경입니다.
	 *  라인트레이스는 시작점이 도형 안이면 그 도형을 맞히지 못하므로,
	 *  실패했을 때만 이 반경으로 한 번 더 훑습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="0.0", Units="cm"))
	float InsideBoundsProbeRadius = 25.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<AActor> FocusedInteractableActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	FAeternaInteractionInfo FocusedInteractionInfo;

private:
	/** 이 액터를 상호작용 대상으로 받아들일 수 있으면 정보를 채우고 참을 반환합니다. */
	bool TryAcceptInteractable(AActor* CandidateActor, AActor* Interactor, FAeternaInteractionInfo& OutInteractionInfo) const;

	/** 카메라를 감싸고 있는 상호작용 액터를 찾습니다. 라인트레이스가 실패했을 때만 씁니다. */
	AActor* FindInteractableAroundCamera(AActor* Interactor, FAeternaInteractionInfo& OutInteractionInfo) const;

	TObjectPtr<UCameraComponent> CameraComponent;
};
