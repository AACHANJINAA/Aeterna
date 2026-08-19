// Copyright Epic Games, Inc. All Rights Reserved.

#include "AeternaPlayerComponent.h"
#include "AeternaCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAeternaPlayerComponent::UAeternaPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAeternaPlayerComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	PlayerCharacter = InPlayerCharacter;
}

UCameraComponent* UAeternaPlayerComponent::GetFirstPersonCamera() const
{
	return PlayerCharacter ? PlayerCharacter->GetFirstPersonCameraComponent() : nullptr;
}

UCharacterMovementComponent* UAeternaPlayerComponent::GetAeternaCharacterMovement() const
{
	return PlayerCharacter ? PlayerCharacter->GetCharacterMovement() : nullptr;
}
