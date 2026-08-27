// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "AeternaInteractableActor.generated.h"

class AAeternaCharacter;
class USoundBase;
class UScenarioManagerSubsystem;

UCLASS()
class AETERNA_API AAeternaInteractableActor : public AActor, public IAeternaInteractableInterface
{
	GENERATED_BODY()

public:
	AAeternaInteractableActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 이 Actor가 수행하는 상호작용 종류입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	EAeternaInteractionType InteractionType = EAeternaInteractionType::Scan;

	/** 조준 프롬프트에 표시할 문구입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	FText InteractionPromptText;

	/** false면 한 번 상호작용한 뒤 다시 상호작용되지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bRepeatable = false;

	/**
	 *  밤이 시작되거나 재시작될 때 다시 상호작용할 수 있게 되돌립니다.
	 *  끄면 한 번 쓴 뒤 그 세션 내내 잠긴 채로 남습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bResetOnScenarioStart = true;

	/** 이미 상호작용 완료됐는지 여부입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	bool bInteractionCompleted = false;

	/** Scan 타입일 때 진행도에 기록할 ID입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scan")
	FName ScanPointId;

	/** Scan 타입이 아니어도 이 상호작용을 스캔 진행도에 함께 기록합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scan")
	bool bCountsAsScanPoint = false;

	/** Charge 타입일 때 회복할 배터리 양입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery", meta=(ClampMin="0.0"))
	float BatteryChargeAmount = 35.0f;

	/**
	 *  상호작용이 실제로 처리됐을 때 이 자리에서 재생할 소리입니다.
	 *  스캔·충전·전등 버튼 등 종류를 가리지 않고 여기 하나로 처리합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction")
	TObjectPtr<USoundBase> InteractionSound;

	/** 스캔·배터리 원본이 피크 -16dBFS 언저리라 그대로 쓰면 작습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction", meta=(ClampMin="0.0"))
	float InteractionSoundVolume = 2.6f;

	/** 비어 있으면 모든 시나리오에서 활성화됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scenario")
	TArray<FName> ActiveScenarioIds;

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FAeternaInteractionInfo GetInteractionInfo_Implementation(AActor* Interactor) const override;

	/** C++ 기본 상호작용 처리입니다. BP는 추가 연출만 BP_Interacted에 연결합니다. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool PerformInteraction(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category="Scan")
	bool RegisterScanProgress(AAeternaCharacter* AeternaCharacter);

	/** InteractionSound를 이 액터 위치에서 재생합니다. 비어 있으면 아무것도 하지 않습니다. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void PlayInteractionSound();

	/** 상호작용을 아직 쓰지 않은 상태로 되돌립니다. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	virtual void ResetInteraction();

	/** 되돌릴 때 BP에서 숨겨둔 메시를 다시 보이게 하는 등 연출을 복구합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction", meta=(DisplayName="Interaction Reset"))
	void BP_InteractionReset();

	/** 상호작용이 실제로 처리된 직후 자식 클래스가 끼어드는 자리입니다. */
	virtual void OnInteractionPerformed(AActor* Interactor) {}

	/** BP에서 추가 연출이나 로그를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction", meta=(DisplayName="Interacted"))
	void BP_Interacted(AActor* Interactor);

	/** 현재 완료 여부를 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsInteractionCompleted() const { return bInteractionCompleted; }

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void MarkInteractionCompleted() { bInteractionCompleted = true; }

	UFUNCTION(BlueprintPure, Category="Interaction")
	EAeternaInteractionType GetInteractionType() const { return InteractionType; }

	UFUNCTION(BlueprintPure, Category="Scan")
	FName GetResolvedScanPointId() const { return ScanPointId.IsNone() ? GetFName() : ScanPointId; }

	UFUNCTION(BlueprintPure, Category="Scan")
	bool CountsAsScanPoint() const { return bCountsAsScanPoint; }

	UFUNCTION(BlueprintPure, Category="Scenario")
	bool IsActiveForScenario(FName ScenarioId) const { return ActiveScenarioIds.Num() == 0 || ActiveScenarioIds.Contains(ScenarioId); }

	UFUNCTION(BlueprintPure, Category="Scenario")
	bool HasActiveScenarioRestrictions() const { return ActiveScenarioIds.Num() > 0; }

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;
};
