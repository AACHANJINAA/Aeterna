// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AeternaPlayerComponent.generated.h"

class AAeternaCharacter;
class UCameraComponent;
class UCharacterMovementComponent;

UCLASS(Abstract, ClassGroup=(Aeterna))
class AETERNA_API UAeternaPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAeternaPlayerComponent();

	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player")
	TObjectPtr<AAeternaCharacter> PlayerCharacter;

	AAeternaCharacter* GetAeternaCharacter() const { return PlayerCharacter.Get(); }
	UCameraComponent* GetFirstPersonCamera() const;
	UCharacterMovementComponent* GetAeternaCharacterMovement() const;
};
