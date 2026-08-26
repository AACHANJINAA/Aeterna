// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaFootstepComponent.generated.h"

class USoundBase;

/**
 *  걸은 거리를 누적해 보폭마다 좌우 발소리를 번갈아 냅니다.
 *  거리 기준이므로 Shift로 빨라지면 발소리도 저절로 잦아집니다.
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaFootstepComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaFootstepComponent();

	/** 매 프레임 호출합니다. 멈춰 있으면 아무것도 하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category="Footstep")
	void UpdateFootsteps(float DeltaSeconds);

	/** 밤이 다시 시작될 때처럼 걸음 상태를 처음으로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category="Footstep")
	void ResetFootsteps();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Footstep")
	bool bEnableFootsteps = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Footstep")
	TObjectPtr<USoundBase> LeftFootstepSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Footstep")
	TObjectPtr<USoundBase> RightFootstepSound;

	/**
	 *  한 걸음으로 치는 이동 거리. 짧을수록 발소리가 잦아집니다.
	 *  기본 걷기 속도 250cm/s에서 초당 두 걸음이 나오는 값입니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Footstep", meta=(ClampMin="1.0", Units="cm"))
	float StrideDistance = 125.0f;

	/** 이 속도 아래면 멈춘 것으로 봅니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Footstep", meta=(ClampMin="0.0"))
	float MinimumWalkSpeed = 10.0f;

	/**
	 *  발소리 크기. 원본이 피크 -17dBFS로 녹음돼 있어 그대로 쓰면 묻힙니다.
	 *  2.2배는 약 +7dB로, 올려도 피크가 -10dBFS라 클리핑되지 않습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Footstep", meta=(ClampMin="0.0"))
	float FootstepVolume = 2.2f;

private:
	void PlayNextFootstep();

	/** 루프가 켜진 에셋을 걸렀는지. 경고를 한 번만 남기려고 둡니다. */
	bool bWarnedLoopingFootstep = false;

	/** 직전 프레임에 걷고 있었는지. 걷기 시작하는 순간을 잡는 데 씁니다. */
	bool bWasMoving = false;

	/** 다음에 낼 발. 좌우를 번갈아 갑니다. */
	bool bLeftFootNext = true;

	float DistanceSinceLastStep = 0.0f;
};
