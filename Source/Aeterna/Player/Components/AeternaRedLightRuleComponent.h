// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaRedLightRuleComponent.generated.h"

class UScenarioManagerSubsystem;
class USoundBase;

UENUM(BlueprintType)
enum class EAeternaRedLightState : uint8
{
	/** 아직 발동하지 않았습니다. */
	Waiting,
	/** 빨간 불이 켜졌고 경비실까지 가야 합니다. */
	Fleeing,
	/** 이번 밤의 이벤트가 끝났습니다. */
	Done
};

/**
 *  수첩 규칙 "빨간색 불이 켜지는 걸 목격하셨다면 경비실로 도망치십시오" 판정입니다.
 *
 *  밤 시작 시 발동 시각을 추첨해 밤당 정확히 한 번 켭니다. 목격 판정은 하지
 *  않습니다 — M-05의 센서가 감지한 것이 곧 목격이라는 팀 결정 (SPEC_NIGHT3 §5).
 *
 *  켜지면 시계 UI가 남은 시간으로 바뀌고, 제한 시간 안에 `SafeZone` 태그가
 *  붙은 액터 안으로 들어가지 못하면 위반입니다.
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaRedLightRuleComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaRedLightRuleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 빨간 불 액터를 표시하는 태그입니다. */
	static const TCHAR* GetRedLightTag() { return TEXT("RedLight"); }

	/** 경비실을 표시하는 태그입니다. 이 액터의 경계 안이 안전합니다. */
	static const TCHAR* GetSafeZoneTag() { return TEXT("SafeZone"); }

	UFUNCTION(BlueprintPure, Category="Red Light Rule")
	EAeternaRedLightState GetRedLightState() const { return RedLightState; }

	UFUNCTION(BlueprintPure, Category="Red Light Rule")
	float GetFleeRemainingSeconds() const { return FleeRemainingSeconds; }

	/** 빨간 불을 끄고 이번 밤의 이벤트를 처음으로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category="Red Light Rule")
	void ResetRedLightRule();

	/** 개발 테스트용으로 지금 즉시 발동시킵니다. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Red Light Rule|Debug")
	void TriggerRedLightNow();

protected:
	/** 이 규칙이 도는 시나리오입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule")
	FName RuleScenarioId = TEXT("S03_ForbiddenLight");

	/**
	 *  빨간 불이 켜지는 게임 시각들입니다. 01:00이 60분이므로
	 *  01:32=92 / 02:12=132 / 03:00=180 / 04:30=270 입니다.
	 *  한 밤에 이 횟수만큼 발동하며, 매번 경비실까지 가야 합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule")
	TArray<int32> TriggerClockMinutesSchedule = { 92, 132, 180, 270 };

	/** 경비실까지 가야 하는 실시간 제한입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule", meta=(ClampMin="1.0", Units="s"))
	float FleeTimeLimitSeconds = 30.0f;

	/** 경비실에 닿은 뒤 빨간 불이 꺼지기까지의 여유입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule", meta=(ClampMin="0.0", Units="s"))
	float SafeHoldSeconds = 2.0f;

	/**
	 *  빨간 불이 켜져 있는 동안 흐르는 BGM입니다. Looping이 켜진 에셋을 넣으십시오.
	 *  비워두면 배경음을 바꾸지 않습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule|Audio")
	TObjectPtr<USoundBase> RedLightBgm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule|Audio", meta=(ClampMin="0.0"))
	float RedLightBgmVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule|Audio", meta=(ClampMin="0.0", Units="s"))
	float RedLightBgmFadeSeconds = 0.5f;

	/** SafeZone 액터의 경계에 이만큼 여유를 줍니다. 문턱에서 아슬아슬하게 죽는 것을 막습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Red Light Rule", meta=(ClampMin="0.0", Units="cm"))
	float SafeZoneTolerance = 60.0f;

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	void CacheLevelActors();
	void ArmTriggerSchedule();
	int32 GetNextTriggerClockMinutes() const;
	void SkipElapsedTriggers(int32 CurrentClockMinutes);
	void BeginFlee();
	void SurviveFlee();
	void FailFlee();

	void SetRedLightsVisible(bool bVisible);

	/** 액터를 숨기고 안에 있는 라이트 컴포넌트도 직접 끕니다. */
	static void SetRedLightVisible(AActor* RedLightActor, bool bVisible);
	bool IsPlayerInSafeZone() const;
	void PushCountdownToHud(float RemainingSeconds);
	void ClearCountdownFromHud();

	void StartRedLightBgm();
	void RestorePreviousBgm();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> RedLightActors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SafeZoneActors;

	UPROPERTY(Transient)
	TObjectPtr<AActor> ActiveRedLightActor;

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;

	/** 빨간 불이 켜지기 직전에 흐르던 곡입니다. 살아남으면 여기로 돌아갑니다. */
	UPROPERTY(Transient)
	TObjectPtr<USoundBase> BgmBeforeRedLight;

	EAeternaRedLightState RedLightState = EAeternaRedLightState::Waiting;

	/** 스케줄에서 다음에 볼 항목입니다. */
	int32 NextTriggerIndex = 0;
	float FleeRemainingSeconds = 0.0f;
	float SafeHoldRemainingSeconds = 0.0f;

	bool bRuleActive = false;
};
