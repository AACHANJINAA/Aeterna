// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaClockFreezeRuleComponent.generated.h"

class UGameClockSubsystem;
class UScenarioManagerSubsystem;

/**
 *  수첩 규칙 "매시 22분에는 움직이지 마십시오" 판정입니다 (SPEC_NIGHT2 §6-3).
 *
 *  게임 내 시계가 매시 FreezeMinuteOfHour분에 들어서면 창이 열리고,
 *  창이 닫힐 때까지 이동 입력이 들어오면 위반입니다. 시점 회전은 허용합니다.
 *
 *  규칙 3(화석 소실)과 달리 예고 신호가 없습니다. 신호는 화면의 시계뿐이고,
 *  미리 멈추는 것이 이 규칙의 내용입니다. 그래서 유예 시간도 두지 않습니다 —
 *  유예를 주면 "시계를 본다"는 행위 자체가 무의미해집니다.
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaClockFreezeRuleComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaClockFreezeRuleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 지금 정지 창이 열려 있는지 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Clock Freeze Rule")
	bool IsFreezeWindowOpen() const { return bFreezeWindowOpen; }

	/** 판정 상태를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category="Clock Freeze Rule")
	void ResetClockFreezeRule();

protected:
	/** 이 규칙이 도는 시나리오입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock Freeze Rule")
	FName RuleScenarioId = TEXT("S02_GrandHallFossil");

	/** 매시 몇 분에 창이 열리는지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock Freeze Rule", meta=(ClampMin="0", ClampMax="59"))
	int32 FreezeMinuteOfHour = 22;

	/** 창이 몇 게임분 동안 유지되는지입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock Freeze Rule", meta=(ClampMin="1", ClampMax="59"))
	int32 FreezeWindowMinutes = 1;

	/** 이 크기를 넘는 이동 입력이 들어오면 움직인 것으로 봅니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock Freeze Rule", meta=(ClampMin="0.0"))
	float MoveInputThreshold = 0.1f;

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	UFUNCTION()
	void HandleClockMinuteChanged(int32 ClockMinutes);

	bool IsFreezeMinute(int32 ClockMinutes) const;
	bool HasMovementInput() const;
	void TriggerViolation();

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;
	TWeakObjectPtr<UGameClockSubsystem> BoundGameClock;

	bool bRuleActive = false;
	bool bFreezeWindowOpen = false;
};
