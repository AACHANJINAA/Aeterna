// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaCameraFallComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Player/AeternaCharacter.h"

UAeternaCameraFallComponent::UAeternaCameraFallComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
}

void UAeternaCameraFallComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	if (!bFalling || !CameraComponent)
	{
		return;
	}

	FallElapsedSeconds += DeltaTime;

	const float Progress = (FallSeconds > KINDA_SMALL_NUMBER)
		? FMath::Clamp(FallElapsedSeconds / FallSeconds, 0.0f, 1.0f)
		: 1.0f;

	// 처음엔 천천히 기울다 바닥에 가까워질수록 빨라집니다.
	const float EasedProgress = FMath::InterpEaseIn(0.0f, 1.0f, Progress, 2.0f);

	CameraComponent->SetWorldLocationAndRotation(
		FMath::Lerp(FallStartLocation, FallTargetLocation, EasedProgress),
		FMath::Lerp(FallStartRotation, FallTargetRotation, EasedProgress));

	if (Progress >= 1.0f)
	{
		SetComponentTickEnabled(false);
	}
}

void UAeternaCameraFallComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ResetFall();
	Super::EndPlay(EndPlayReason);
}

void UAeternaCameraFallComponent::StartFall()
{
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	UWorld* World = GetWorld();
	if (bFalling || !AeternaCharacter || !CameraComponent || !World)
	{
		return;
	}

	SavedCameraParent = CameraComponent->GetAttachParent();
	SavedCameraSocket = CameraComponent->GetAttachSocketName();
	SavedCameraRelativeTransform = CameraComponent->GetRelativeTransform();
	bSavedUsePawnControlRotation = CameraComponent->bUsePawnControlRotation;

	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	FallStartLocation = CameraComponent->GetComponentLocation();
	FallStartRotation = CameraComponent->GetComponentRotation();

	// 바닥을 찾아 그 위로 눕힙니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaCameraFallTrace), false, AeternaCharacter);

	FHitResult HitResult;
	const FVector TraceEnd = FallStartLocation - FVector(0.0f, 0.0f, GroundTraceDistance);
	FallTargetLocation = World->LineTraceSingleByChannel(HitResult, FallStartLocation, TraceEnd, ECC_Visibility, QueryParams)
		? HitResult.ImpactPoint + FVector(0.0f, 0.0f, FallHeightAboveFloor)
		: FallStartLocation - FVector(0.0f, 0.0f, 100.0f);

	FallTargetRotation = FRotator(-10.0f, FallStartRotation.Yaw, FallRollDegrees);

	// 쓰러진 카메라가 자기 몸을 올려다보게 되므로 메시를 감춥니다.
	SetCharacterMeshesVisible(false);

	FallElapsedSeconds = 0.0f;
	bFalling = true;
	SetComponentTickEnabled(true);
}

void UAeternaCameraFallComponent::ResetFall()
{
	USceneComponent* CameraParent = SavedCameraParent.Get();
	UCameraComponent* CameraComponent = GetFirstPersonCamera();
	if (!CameraParent || !CameraComponent)
	{
		return;
	}

	// 메시 가시성 복구가 카메라 부착보다 먼저여야 합니다.
	// SetVisibility는 자식까지 전파되는데, 카메라를 먼저 붙이면 그 자식인
	// 헤드램프까지 강제로 켜져 버립니다.
	SetCharacterMeshesVisible(true);

	CameraComponent->AttachToComponent(CameraParent, FAttachmentTransformRules::KeepRelativeTransform, SavedCameraSocket);
	CameraComponent->SetRelativeTransform(SavedCameraRelativeTransform);
	CameraComponent->bUsePawnControlRotation = bSavedUsePawnControlRotation;

	SavedCameraParent.Reset();
	FallElapsedSeconds = 0.0f;
	bFalling = false;
	SetComponentTickEnabled(false);
}

void UAeternaCameraFallComponent::SetCharacterMeshesVisible(bool bVisible)
{
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter)
	{
		return;
	}

	if (USkeletalMeshComponent* ThirdPersonMesh = AeternaCharacter->GetMesh())
	{
		ThirdPersonMesh->SetVisibility(bVisible, true);
	}

	if (USkeletalMeshComponent* FirstPersonMesh = AeternaCharacter->GetFirstPersonMesh())
	{
		FirstPersonMesh->SetVisibility(bVisible, true);
	}
}
