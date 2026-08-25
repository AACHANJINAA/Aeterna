// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaGazeRuleComponent.generated.h"

class UScenarioManagerSubsystem;

/**
 *  수첩 규칙 "화석의 눈구멍은 비추지 마십시오" 판정입니다 (SPEC_NIGHT2 §6-1).
 *
 *  `Eye` 태그가 붙은 액터를 눈구멍으로 봅니다. 그 액터는 평소 숨겨져 있다가
 *  위반 시 천천히 커지며 나타납니다 — 눈알 에셋으로 바꿀 자리입니다.
 *
 *  위반 조건: 헤드램프 ON + 눈구멍이 시야 중앙 콘 안 + 사거리 안 + 가림 없음,
 *  이 상태가 연속 GazeDurationSeconds 유지. 헤드램프를 끄면 비위반입니다
 *  (빛이 곧 시선 — SPEC 확정).
 *
 *  판정만 여기서 하고, 실패 처리는 ScenarioManager에 위임합니다.
 */
UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaGazeRuleComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaGazeRuleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 눈구멍 액터를 표시하는 태그입니다. */
	static const TCHAR* GetEyeTag() { return TEXT("Eye"); }

	/** 눈구멍을 모두 숨기고 원래 크기로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category="Gaze Rule")
	void ResetEyeActors();

	UFUNCTION(BlueprintPure, Category="Gaze Rule")
	bool IsRuleActive() const { return bRuleActive; }

	/** 현재 응시가 얼마나 누적됐는지 0~1로 반환합니다 (디버그용). */
	UFUNCTION(BlueprintPure, Category="Gaze Rule")
	float GetGazeProgress() const;

protected:
	/** 이 규칙이 도는 시나리오입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gaze Rule")
	FName RuleScenarioId = TEXT("S02_GrandHallFossil");

	/** 시야 중앙으로 인정하는 반각입니다. 좁을수록 "들여다봐야" 걸립니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gaze Rule", meta=(ClampMin="0.5", ClampMax="90.0", Units="deg"))
	float CenterConeHalfAngle = 6.0f;

	/** 이 시간만큼 연속으로 비추면 위반입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gaze Rule", meta=(ClampMin="0.0", Units="s"))
	float GazeDurationSeconds = 0.3f;

	/** 헤드램프 빛이 닿는다고 보는 최대 거리입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gaze Rule", meta=(ClampMin="0.0", Units="cm"))
	float MaxGazeDistance = 1200.0f;

	/** 눈구멍이 나타나는 데 걸리는 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gaze Rule|Reveal", meta=(ClampMin="0.0", Units="s"))
	float RevealSeconds = 1.5f;

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	void CacheEyeActors();
	AActor* FindGazedEyeActor() const;
	bool IsGazingAtEye(const AActor* EyeActor) const;
	void TriggerViolation(AActor* EyeActor);
	void UpdateReveal(float DeltaTime);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> EyeActors;

	UPROPERTY(Transient)
	TObjectPtr<AActor> RevealingEyeActor;

	UPROPERTY(Transient)
	TObjectPtr<AActor> GazedEyeActor;

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;

	/** 눈구멍 액터의 원래 크기입니다. 나타날 때 이 값까지 커집니다. */
	TArray<FVector> EyeAuthoredScales;

	float GazeSeconds = 0.0f;
	float RevealElapsedSeconds = 0.0f;
	bool bRuleActive = false;
};
