// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaHeadBobComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAeternaHeadBobComponent::UAeternaHeadBobComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAeternaHeadBobComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);

	if (UCameraComponent* CameraComponent = GetFirstPersonCamera())
	{
		BaseCameraRelativeLocation = CameraComponent->GetRelativeLocation();
		BaseCameraRelativeRotation = CameraComponent->GetRelativeRotation();
	}
}

void UAeternaHeadBobComponent::InitializeHeadBob(float InSprintSpeed)
{
	SprintSpeed = InSprintSpeed;

	if (UCameraComponent* CameraComponent = GetFirstPersonCamera())
	{
		BaseCameraRelativeLocation = CameraComponent->GetRelativeLocation();
		BaseCameraRelativeRotation = CameraComponent->GetRelativeRotation();
	}
}

void UAeternaHeadBobComponent::UpdateHeadBob(float DeltaSeconds, bool bSprinting)
{
	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	if (!CameraComponent)
	{
		return;
	}

	if (!bEnableHeadBob)
	{
		HeadBobMoveBlend = 0.0f;
		CameraComponent->SetRelativeLocation(BaseCameraRelativeLocation);
		CameraComponent->SetRelativeRotation(BaseCameraRelativeRotation);
		return;
	}

	const FVector OwnerVelocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
	const FVector HorizontalVelocity = FVector(OwnerVelocity.X, OwnerVelocity.Y, 0.0f);
	const float HorizontalSpeed = HorizontalVelocity.Size();
	UCharacterMovementComponent* MovementComponent = GetAeternaCharacterMovement();
	const bool bMovingOnGround = HorizontalSpeed > 5.0f && MovementComponent && MovementComponent->IsMovingOnGround();
	const float TargetMoveBlend = bMovingOnGround ? FMath::Clamp(HorizontalSpeed / FMath::Max(SprintSpeed, 1.0f), 0.0f, 1.0f) : 0.0f;
	HeadBobMoveBlend = FMath::FInterpTo(HeadBobMoveBlend, TargetMoveBlend, DeltaSeconds, HeadBobBlendSpeed);

	const float SprintAlpha = bSprinting ? 1.0f : 0.0f;
	const float MoveFrequency = FMath::Lerp(WalkHeadBobFrequency, SprintHeadBobFrequency, SprintAlpha);
	const float MoveVerticalAmount = FMath::Lerp(WalkHeadBobVerticalAmount, SprintHeadBobVerticalAmount, SprintAlpha);
	const float MoveSideAmount = FMath::Lerp(WalkHeadBobSideAmount, SprintHeadBobSideAmount, SprintAlpha);
	const float MovePitchAmount = FMath::Lerp(WalkHeadBobPitchAmount, SprintHeadBobPitchAmount, SprintAlpha);

	const float BobFrequency = FMath::Lerp(IdleHeadBobFrequency, MoveFrequency, HeadBobMoveBlend);
	HeadBobPhase = FMath::Fmod(HeadBobPhase + DeltaSeconds * BobFrequency * 2.0f * UE_PI, 2.0f * UE_PI);

	const float IdleBlend = 1.0f - HeadBobMoveBlend;
	const float IdleVerticalOffset = FMath::Sin(HeadBobPhase) * IdleHeadBobVerticalAmount * IdleBlend;
	const float IdlePitchOffset = FMath::Sin(HeadBobPhase * 0.75f) * IdleHeadBobPitchAmount * IdleBlend;

	const float MoveVerticalOffset = FMath::Sin(HeadBobPhase * 2.0f) * MoveVerticalAmount * HeadBobMoveBlend;
	const float MoveSideOffset = FMath::Sin(HeadBobPhase) * MoveSideAmount * HeadBobMoveBlend;
	const float MovePitchOffset = FMath::Sin(HeadBobPhase * 2.0f + 0.5f * UE_PI) * MovePitchAmount * HeadBobMoveBlend;

	FVector BobbedLocation = BaseCameraRelativeLocation;
	BobbedLocation.Y += MoveSideOffset;
	BobbedLocation.Z += IdleVerticalOffset + MoveVerticalOffset;

	FRotator BobbedRotation = BaseCameraRelativeRotation;
	BobbedRotation.Pitch += IdlePitchOffset + MovePitchOffset;

	CameraComponent->SetRelativeLocation(BobbedLocation);
	CameraComponent->SetRelativeRotation(BobbedRotation);
}
