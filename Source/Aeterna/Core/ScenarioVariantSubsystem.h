// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScenarioVariantSubsystem.generated.h"

class UScenarioManagerSubsystem;

/**
 *  액터 태그만으로 밤별 존재 여부를 처리합니다.
 *  `Night_<시나리오 ID>` 태그가 하나라도 붙은 액터는 이 시스템이 관리하며,
 *  현재 밤의 태그를 가진 밤에만 보이고 나머지 밤에는 숨겨집니다.
 */
UCLASS()
class AETERNA_API UScenarioVariantSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 밤별 태그 접두어입니다. */
	static const TCHAR* GetScenarioTagPrefix() { return TEXT("Night_"); }

	/** 시나리오 ID를 태그 이름으로 바꿉니다. */
	UFUNCTION(BlueprintPure, Category="Scenario Variant")
	static FName MakeScenarioTag(FName ScenarioId);

	/** 태그가 붙은 모든 액터에 해당 밤의 상태를 적용합니다. */
	UFUNCTION(BlueprintCallable, Category="Scenario Variant")
	void ApplyScenarioToTaggedActors(FName ScenarioId);

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;
};
