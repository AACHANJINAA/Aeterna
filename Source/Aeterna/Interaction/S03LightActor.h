// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableActor.h"
#include "S03LightActor.generated.h"

class UBoxComponent;
class ULightComponent;
class UPointLightComponent;
class UGameClockSubsystem;

/**
 *  수첩 규칙 "1시 이후 켜진 불은 끄십시오" 대상 조명입니다 (SPEC_NIGHT3 §3).
 *
 *  정해진 게임 시각에 저절로 켜지고, 켜져 있는 동안에만 E로 끌 수 있습니다.
 *  끈 것은 스캔 진행도에 1을 더하므로 스타터의 RequiredScanCount가 곧
 *  꺼야 하는 조명 수가 됩니다.
 */
UCLASS()
class AETERNA_API AS03LightActor : public AAeternaInteractableActor
{
	GENERATED_BODY()

public:
	AS03LightActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	/** 밤이 시작될 때 소등 상태로 되돌리고 점등 예약을 다시 겁니다. */
	virtual void ResetInteraction() override;

	/** E가 처리되면 불을 끕니다. */
	virtual void OnInteractionPerformed(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category="S03 Light")
	void SetLightOn(bool bOn);

	UFUNCTION(BlueprintPure, Category="S03 Light")
	bool IsLightOn() const { return bLightOn; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPointLightComponent> LightComponent;

	/**
	 *  이 게임 시각(분)이 되면 저절로 켜집니다. 01:00이 60입니다.
	 *  0 이하면 밤이 시작될 때부터 켜져 있습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S03 Light")
	int32 TurnOnClockMinutes = 70;

	/** 켜지고 이 시간 동안은 E를 받지 않습니다. 점등 순간 반사적으로 누르는 것을 거릅니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S03 Light", meta=(ClampMin="0.0", Units="s"))
	float InteractionLockAfterTurnOnSeconds = 1.0f;

	/** 구역명입니다. 로그에 그대로 실립니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S03 Light")
	FText ZoneName;

	/** 켜지고 꺼질 때 BP에서 연출을 붙입니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="S03 Light", meta=(DisplayName="Light State Changed"))
	void BP_LightStateChanged(bool bOn);

private:
	UFUNCTION()
	void HandleClockMinuteChanged(int32 ClockMinutes);

	/** 지금 돌고 있는 밤이 이 조명이 속한 밤인지. 다른 밤에서는 꺼져 있어야 합니다. */
	bool IsActiveInCurrentScenario() const;
	void ApplyScenarioVisibility();

	FString GetZoneLogName() const;

	bool bLightOn = false;
	float TurnOnWorldTimeSeconds = 0.0f;

	TWeakObjectPtr<UGameClockSubsystem> BoundGameClock;
};
