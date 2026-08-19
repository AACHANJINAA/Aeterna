// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AeternaInteractableInterface.h"
#include "AeternaPlayerComponent.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<AActor> FocusedInteractableActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	FAeternaInteractionInfo FocusedInteractionInfo;

private:
	TObjectPtr<UCameraComponent> CameraComponent;
};
