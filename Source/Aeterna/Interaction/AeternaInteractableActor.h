// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "AeternaInteractableActor.generated.h"

class AAeternaCharacter;

UCLASS()
class AETERNA_API AAeternaInteractableActor : public AActor, public IAeternaInteractableInterface
{
	GENERATED_BODY()

public:
	AAeternaInteractableActor();

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

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FAeternaInteractionInfo GetInteractionInfo_Implementation(AActor* Interactor) const override;

	/** C++ 기본 상호작용 처리입니다. BP는 추가 연출만 BP_Interacted에 연결합니다. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool PerformInteraction(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category="Scan")
	bool RegisterScanProgress(AAeternaCharacter* AeternaCharacter);

	/** BP에서 추가 연출이나 로그를 연결합니다. */
	UFUNCTION(BlueprintImplementableEvent, Category="Interaction", meta=(DisplayName="Interacted"))
	void BP_Interacted(AActor* Interactor);

	/** 현재 완료 여부를 반환합니다. */
	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsInteractionCompleted() const { return bInteractionCompleted; }
};
