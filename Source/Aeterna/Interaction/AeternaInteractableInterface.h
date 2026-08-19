// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AeternaInteractableInterface.generated.h"

UENUM(BlueprintType)
enum class EAeternaInteractionType : uint8
{
	None,
	Scan,
	Read,
	Pickup,
	Install,
	Charge,
	UseTerminal,
	Custom
};

USTRUCT(BlueprintType)
struct FAeternaInteractionInfo
{
	GENERATED_BODY()

	/** 상호작용의 성격입니다. 프롬프트와 처리 분기에 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	EAeternaInteractionType Type = EAeternaInteractionType::None;

	/** UI에 표시할 짧은 문구입니다. 실제 문구는 이후 String Table 연동 대상으로 둡니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText PromptText;
};

UINTERFACE(Blueprintable)
class UAeternaInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class AETERNA_API IAeternaInteractableInterface
{
	GENERATED_BODY()

public:
	/** 플레이어가 E 입력으로 대상과 상호작용할 때 호출됩니다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);

	/** 상호작용 가능한 상태인지 확인합니다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanInteract(AActor* Interactor) const;

	/** 조준 프롬프트와 상호작용 분기에 필요한 표시 정보를 반환합니다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FAeternaInteractionInfo GetInteractionInfo(AActor* Interactor) const;
};
