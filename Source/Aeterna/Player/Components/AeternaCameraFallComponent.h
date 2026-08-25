// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaCameraFallComponent.generated.h"

class USceneComponent;

/**
 *  위반 시 카메라가 바닥으로 쓰러지는 연출입니다. 규칙 여러 개가 공용합니다.
 *
 *  시점을 다른 카메라로 넘기지 않습니다. 넘기면 1인칭 렌더링 설정
 *  (FirstPersonFieldOfView, FirstPersonScale)이 날아가 화각이 튀고 3인칭
 *  메시가 드러나기 때문입니다. 보고 있던 카메라를 폰에서 떼어내 그대로
 *  쓰러뜨리고, 낙하 중에는 캐릭터 메시를 감춥니다.
 *
 *  물리 시뮬이 아니라 보간이라 매번 같은 모양으로 쓰러집니다 (SPEC_NIGHT2 §8).
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaCameraFallComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaCameraFallComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 카메라를 떼어내 바닥으로 쓰러뜨리기 시작합니다. */
	UFUNCTION(BlueprintCallable, Category="Camera Fall")
	void StartFall();

	/** 카메라를 원래 자리로 되돌리고 메시를 다시 보이게 합니다. */
	UFUNCTION(BlueprintCallable, Category="Camera Fall")
	void ResetFall();

	UFUNCTION(BlueprintPure, Category="Camera Fall")
	bool IsFalling() const { return bFalling; }

protected:
	/** 카메라가 바닥까지 쓰러지는 데 걸리는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Fall", meta=(ClampMin="0.0", Units="s"))
	float FallSeconds = 1.2f;

	/** 쓰러진 카메라가 바닥에서 뜨는 높이입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Fall", meta=(ClampMin="0.0", Units="cm"))
	float FallHeightAboveFloor = 25.0f;

	/** 쓰러진 카메라의 기울기입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Fall", meta=(Units="deg"))
	float FallRollDegrees = 85.0f;

	/** 바닥을 찾는 아래 방향 트레이스 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera Fall", meta=(ClampMin="0.0", Units="cm"))
	float GroundTraceDistance = 500.0f;

private:
	void SetCharacterMeshesVisible(bool bVisible);

	FVector FallStartLocation = FVector::ZeroVector;
	FVector FallTargetLocation = FVector::ZeroVector;
	FRotator FallStartRotation = FRotator::ZeroRotator;
	FRotator FallTargetRotation = FRotator::ZeroRotator;

	/** 낙하 전 카메라 부착 상태입니다. 복구 시 이대로 되돌립니다. */
	TWeakObjectPtr<USceneComponent> SavedCameraParent;
	FName SavedCameraSocket = NAME_None;
	FTransform SavedCameraRelativeTransform = FTransform::Identity;
	bool bSavedUsePawnControlRotation = true;

	float FallElapsedSeconds = 0.0f;
	bool bFalling = false;
};
