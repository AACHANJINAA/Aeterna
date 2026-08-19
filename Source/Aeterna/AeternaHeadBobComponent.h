// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AeternaPlayerComponent.h"
#include "AeternaHeadBobComponent.generated.h"

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaHeadBobComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaHeadBobComponent();

	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;
	void InitializeHeadBob(float InSprintSpeed);

	UFUNCTION(BlueprintCallable, Category="Camera|Head Bob")
	void UpdateHeadBob(float DeltaSeconds, bool bSprinting);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob")
	bool bEnableHeadBob = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0"))
	float IdleHeadBobFrequency = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="cm"))
	float IdleHeadBobVerticalAmount = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="deg"))
	float IdleHeadBobPitchAmount = 0.14f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0"))
	float WalkHeadBobFrequency = 1.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="cm"))
	float WalkHeadBobVerticalAmount = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="cm"))
	float WalkHeadBobSideAmount = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="deg"))
	float WalkHeadBobPitchAmount = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0"))
	float SprintHeadBobFrequency = 2.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="cm"))
	float SprintHeadBobVerticalAmount = 3.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="cm"))
	float SprintHeadBobSideAmount = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0", Units="deg"))
	float SprintHeadBobPitchAmount = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera|Head Bob", meta=(ClampMin="0.0"))
	float HeadBobBlendSpeed = 8.0f;

private:
	FVector BaseCameraRelativeLocation = FVector::ZeroVector;
	FRotator BaseCameraRelativeRotation = FRotator::ZeroRotator;
	float SprintSpeed = 600.0f;
	float HeadBobPhase = 0.0f;
	float HeadBobMoveBlend = 0.0f;
};
