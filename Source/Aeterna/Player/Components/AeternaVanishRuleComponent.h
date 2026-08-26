// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaVanishRuleComponent.generated.h"

class UScenarioManagerSubsystem;

UENUM(BlueprintType)
enum class EAeternaVanishState : uint8
{
	/** 발동 대기 중입니다. */
	Idle,
	/** 화석이 사라졌고 움직이면 안 되는 구간입니다. */
	Frozen,
	/** 발동 직후 재발동이 막힌 구간입니다. */
	Cooldown,
	/** 위반해서 낙하 연출이 진행 중입니다. */
	Falling
};

/**
 *  수첩 규칙 "화석이 눈앞에서 사라졌다면 움직이지 마십시오" 판정입니다.
 *
 *  `Vanish` 태그가 붙은 액터가 대상입니다. 플레이어가 반경 안으로 들어오고
 *  그것이 시야에 보이는 상태면 즉시 사라지고, FreezeSeconds 동안 이동 입력이
 *  들어오면 위반입니다. 시점 회전은 허용합니다.
 *
 *  버티면 화석이 제자리에 다시 나타납니다. 기본은 한 밤에 한 번만
 *  발동하며(bTriggerOnce), 밤이 시작되거나 재시작될 때 다시 무장합니다.
 *  위반하면 카메라가 바닥으로 쓰러지고 시나리오 실패로 넘깁니다.
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaVanishRuleComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaVanishRuleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 사라질 액터를 표시하는 태그입니다. */
	static const TCHAR* GetVanishTag() { return TEXT("Vanish"); }

	UFUNCTION(BlueprintPure, Category="Vanish Rule")
	EAeternaVanishState GetVanishState() const { return VanishState; }

	/** 정지 구간에서 남은 시간입니다. */
	UFUNCTION(BlueprintPure, Category="Vanish Rule")
	float GetFreezeRemainingSeconds() const { return FreezeRemainingSeconds; }

	/** 화석을 되돌리고 상태를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category="Vanish Rule")
	void ResetVanishRule();

protected:
	/** 이 규칙이 도는 시나리오입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule")
	FName RuleScenarioId = TEXT("S02_GrandHallFossil");

	/** 이 거리 안으로 들어오면 발동 후보가 됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule", meta=(ClampMin="0.0", Units="cm"))
	float TriggerRadius = 1400.0f;

	/** 화면 안에 있다고 보는 반각입니다. 이 안에 들어와 있어야 발동합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule", meta=(ClampMin="1.0", ClampMax="90.0", Units="deg"))
	float TriggerViewHalfAngle = 45.0f;

	/** 사라진 직후 이동 판정을 하지 않는 유예입니다. 걸어오던 중에 발동해도 멈출 시간을 줍니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule", meta=(ClampMin="0.0", Units="s"))
	float FreezeGraceSeconds = 0.5f;

	/** 유예가 끝난 뒤 움직이면 안 되는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule", meta=(ClampMin="0.0", Units="s"))
	float FreezeSeconds = 2.0f;

	/**
	 *  한 밤에 한 번만 발동시킵니다. 밤이 시작되거나 재시작될 때 다시 무장합니다.
	 *  끄면 CooldownSeconds 간격으로 계속 반복됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule")
	bool bTriggerOnce = true;

	/** bTriggerOnce가 꺼져 있을 때, 한 번 발동한 뒤 다시 발동하기까지의 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule", meta=(ClampMin="0.0", Units="s", EditCondition="!bTriggerOnce"))
	float CooldownSeconds = 45.0f;

	/** 이 크기를 넘는 이동 입력이 들어오면 움직인 것으로 봅니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vanish Rule", meta=(ClampMin="0.0"))
	float MoveInputThreshold = 0.1f;

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	void CacheVanishActors();
	void SetVanishActorsHidden(bool bHidden);
	bool ShouldTriggerVanish() const;
	bool IsVanishActorVisible(const AActor* VanishActor) const;
	bool HasMovementInput() const;

	void BeginFreeze();
	void SurviveFreeze();
	void TriggerViolation();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> VanishActors;

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;

	EAeternaVanishState VanishState = EAeternaVanishState::Idle;
	float FreezeRemainingSeconds = 0.0f;
	float FreezeGraceRemainingSeconds = 0.0f;
	float CooldownRemainingSeconds = 0.0f;

	bool bRuleActive = false;

	/** 이번 밤에 이미 발동했는지. bTriggerOnce일 때 재발동을 막습니다. */
	bool bAlreadyTriggered = false;
};
