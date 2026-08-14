// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AeternaInteractableInterface.generated.h"

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
};
