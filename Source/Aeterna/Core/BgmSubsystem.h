// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BgmSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

/**
 *  밤별 배경음의 재생 상태를 관리합니다.
 *  어느 밤에 어느 곡을 트는지는 AScenarioLoopStarterActor가 정하고,
 *  여기서는 지금 무엇이 흐르는지와 곡 교체만 책임집니다.
 */
UCLASS(BlueprintType)
class AETERNA_API UBgmSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	/**
	 *  배경음을 재생합니다. 곡이 바뀌면 이전 곡을 페이드아웃하며 겹칩니다.
	 *  이미 같은 곡이 흐르고 있으면 아무것도 하지 않습니다 — 밤을 재시작해도 음악이 끊기지 않습니다.
	 */
	UFUNCTION(BlueprintCallable, Category="BGM")
	void PlayBgm(USoundBase* Bgm, float FadeInSeconds = 2.0f, float CrossFadeOutSeconds = 2.0f);

	/** 흐르던 배경음을 서서히 걷어냅니다. */
	UFUNCTION(BlueprintCallable, Category="BGM")
	void StopBgm(float FadeOutSeconds = 2.0f);

	UFUNCTION(BlueprintPure, Category="BGM")
	USoundBase* GetCurrentBgm() const { return CurrentBgm; }

	UFUNCTION(BlueprintPure, Category="BGM")
	bool IsPlayingBgm() const;

private:
	/** 지금 흐르는 곡. 페이드아웃에 넘긴 이전 곡은 여기 남지 않습니다. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BgmComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentBgm;
};
