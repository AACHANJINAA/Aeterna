// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaCarryComponent.generated.h"

class UStaticMesh;
class UScenarioManagerSubsystem;

/**
 *  E를 누르고 있는 동안 물체를 들고, 제자리에 가까이 대면 자동으로 설치합니다.
 *
 *  대상은 액터 태그로만 지정합니다 — 액터마다 컴포넌트를 붙이지 않습니다.
 *  들 물체와 제자리 양쪽에 `Carry` 태그를 붙이면 됩니다.
 *
 *  제자리는 미리 짝지어 두지 않습니다. 지금 밤에 숨겨져 있는(= 다른 밤에 속한)
 *  같은 메시의 `Carry` 액터가 모두 후보이고, 들고 다가간 것 중 가장 가까운
 *  빈 자리에 붙습니다. 메시가 같으면 어느 자리에 놓아도 결과가 같으므로
 *  쌍을 손으로 지정할 필요가 없습니다.
 *
 *  특정 자리에만 들어가야 하면 양쪽에 같은 `Carry_<ID>` 태그를 붙입니다.
 *  그 경우 같은 ID끼리만 후보가 됩니다.
 *
 *  판정은 이 컴포넌트가 하고, 연출은 캐릭터의 BP 이벤트에 연결합니다.
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaCarryComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaCarryComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 운반 대상을 표시하는 기본 태그입니다. */
	static const TCHAR* GetCarryTag() { return TEXT("Carry"); }

	/** 들어갈 자리를 직접 지정할 때 쓰는 태그 접두어입니다. */
	static const TCHAR* GetCarryTagPrefix() { return TEXT("Carry_"); }

	/** 시야 전방의 운반 대상을 집습니다. 대상이 없으면 false를 반환합니다. */
	UFUNCTION(BlueprintCallable, Category="Carry")
	bool TryStartCarry();

	/** 이미 상호작용 포커스가 잡힌 운반 대상을 집습니다. */
	UFUNCTION(BlueprintCallable, Category="Carry")
	bool TryStartCarryTarget(AActor* TargetActor);

	/** 들고 있던 물체를 그 자리에 놓습니다. */
	UFUNCTION(BlueprintCallable, Category="Carry")
	void StopCarry();

	/** 시야 중앙의 운반 후보를 갱신합니다. */
	UFUNCTION(BlueprintCallable, Category="Carry")
	void UpdateCarryTarget();

	UFUNCTION(BlueprintPure, Category="Carry")
	bool IsCarrying() const { return CarriedActor != nullptr; }

	UFUNCTION(BlueprintPure, Category="Carry")
	AActor* GetCarriedActor() const { return CarriedActor; }

	UFUNCTION(BlueprintPure, Category="Carry")
	AActor* GetCarryTargetActor() const { return CarryTargetActor; }

	/** 현재 들고 있는 물체의 진행도 기록 ID입니다. */
	UFUNCTION(BlueprintPure, Category="Carry")
	FName GetCarriedId() const { return CarriedId; }

	/** 해당 ID가 이미 제자리에 설치됐는지 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Carry")
	bool IsInstalled(FName CarryId) const { return InstalledIds.Contains(CarryId); }

	/** 밤이 바뀔 때 설치 기록과 사용된 자리를 비웁니다. */
	UFUNCTION(BlueprintCallable, Category="Carry")
	void ResetInstallProgress();

protected:
	/** 운반이 가능한 시나리오 목록입니다. 비우면 모든 밤에서 가능합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	TArray<FName> ActiveScenarioIds;

	/** 카메라 기준 운반 위치입니다 (X 전방 / Y 우측 / Z 상단). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry", meta=(Units="cm"))
	FVector CarryOffset = FVector(110.0f, 0.0f, -60.0f);

	/** 운반 위치 추종 속도입니다. 낮을수록 묵직하게 따라옵니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry", meta=(ClampMin="0.1"))
	float CarryInterpSpeed = 14.0f;

	/** 운반 회전 추종 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry", meta=(ClampMin="0.1"))
	float CarryRotationInterpSpeed = 10.0f;

	/** 집을 수 있는 최대 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry", meta=(ClampMin="0.0", Units="cm"))
	float CarryTraceDistance = 300.0f;

	/** 조준 판정 반경입니다. 0이면 라인 트레이스만 씁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry", meta=(ClampMin="0.0", Units="cm"))
	float CarryTraceRadius = 24.0f;

	/** 콜리전이 없는 메시도 집을 수 있게, 트레이스가 빗나가면 바운즈로 한 번 더 찾습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	bool bAllowBoundsFallback = true;

	/** 바운즈 판정에 더해줄 여유입니다. 클수록 조준이 관대해집니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry", meta=(ClampMin="0.0", Units="cm"))
	float BoundsFallbackTolerance = 25.0f;

	/** 자리에 이 거리 안으로 들어오면 자동으로 설치됩니다. 클수록 관대합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry|Snap", meta=(ClampMin="0.0", Units="cm"))
	float SnapDistance = 100.0f;

	/** 놓을 때 바닥을 찾는 아래 방향 트레이스 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry|Drop", meta=(ClampMin="0.0", Units="cm"))
	float GroundTraceDistance = 500.0f;

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	AActor* FindCarryTargetUnderCrosshair() const;
	AActor* FindCarryTargetByBounds() const;
	bool IsCarryTargetOccluded(const AActor* TargetActor, const FVector& RayStart) const;

	/** 들고 있는 물체가 들어갈 수 있는 자리 중 가장 가까운 것을 찾습니다. */
	AActor* FindNearestAvailableSocket(const AActor* SourceActor, float& OutDistance) const;

	/** 해당 액터가 SourceActor를 받아줄 수 있는 자리인지 판정합니다. */
	bool IsSocketCandidate(const AActor* SocketActor, const AActor* SourceActor) const;

	void UpdateCarriedTransform(float DeltaTime);
	bool TrySnapToSocket();
	void InstallCarriedActor(AActor* SocketActor);
	void PlaceCarriedActorOnGround();
	void ReleaseCarriedActor();
	void SetCarryTargetActor(AActor* NewTargetActor);

	static bool IsCarryTagged(const AActor* Actor);
	static FName GetExplicitCarryId(const AActor* Actor);
	static FName GetCarryId(const AActor* Actor);
	static const UStaticMesh* GetCarryStaticMesh(const AActor* Actor);
	static void EnsureMovable(AActor* Actor);

	UPROPERTY(Transient)
	TObjectPtr<AActor> CarriedActor;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CarryTargetActor;

	/** 이미 채워진 자리입니다. 두 번째 물체가 같은 자리에 붙지 않게 합니다. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> UsedSocketActors;

	UPROPERTY(Transient)
	TSet<FName> InstalledIds;

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;

	FName CarriedId;
	bool bCarriedCollisionEnabled = true;
	bool bCarryEnabled = true;
};
