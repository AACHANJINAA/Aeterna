// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaFootstepComponent.h"

#include "Aeterna.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AeternaCharacter.h"
#include "Sound/SoundBase.h"

UAeternaFootstepComponent::UAeternaFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAeternaFootstepComponent::ResetFootsteps()
{
	bWasMoving = false;
	bLeftFootNext = true;
	DistanceSinceLastStep = 0.0f;
}

void UAeternaFootstepComponent::UpdateFootsteps(float DeltaSeconds)
{
	if (!bEnableFootsteps)
	{
		ResetFootsteps();
		return;
	}

	const UCharacterMovementComponent* Movement = GetAeternaCharacterMovement();
	if (!Movement)
	{
		return;
	}

	const float Speed = Movement->Velocity.Size2D();
	const bool bMoving = Movement->IsMovingOnGround() && Speed > MinimumWalkSpeed;

	if (!bMoving)
	{
		bWasMoving = false;
		DistanceSinceLastStep = 0.0f;
		return;
	}

	// 걷기 시작하는 첫 걸음은 즉시 냅니다.
	// WASD를 짧게 눌렀다 떼면 소리가 하나만 나고 끝나는 지점입니다.
	if (!bWasMoving)
	{
		bWasMoving = true;
		DistanceSinceLastStep = 0.0f;
		PlayNextFootstep();
		return;
	}

	DistanceSinceLastStep += Speed * DeltaSeconds;

	// StrideDistance는 1cm 아래로 못 내려가므로 이 루프는 반드시 끝납니다.
	while (DistanceSinceLastStep >= StrideDistance)
	{
		DistanceSinceLastStep -= StrideDistance;
		PlayNextFootstep();
	}
}

void UAeternaFootstepComponent::PlayNextFootstep()
{
	USoundBase* Footstep = bLeftFootNext ? LeftFootstepSound : RightFootstepSound;

	// 소리가 비어 있어도 발은 바꿉니다. 한쪽만 지정해도 박자가 어긋나지 않습니다.
	bLeftFootNext = !bLeftFootNext;

	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!Footstep || !AeternaCharacter)
	{
		return;
	}

	// 루프가 켜진 에셋은 걸음마다 끝나지 않는 소리를 하나씩 쌓습니다.
	// 멈춰도 계속 돌기 때문에, 재생하지 않고 이유를 남깁니다.
	if (Footstep->IsLooping())
	{
		if (!bWarnedLoopingFootstep)
		{
			bWarnedLoopingFootstep = true;
			UE_LOG(LogAeterna, Warning,
				TEXT("[Footstep] %s 에 Looping이 켜져 있어 재생하지 않습니다. 발소리는 단발이어야 합니다. 에셋을 열어 Looping을 끄십시오."),
				*Footstep->GetName());
		}
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(AeternaCharacter, Footstep, AeternaCharacter->GetActorLocation(), FootstepVolume);
}
