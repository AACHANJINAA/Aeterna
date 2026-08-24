// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableActor.h"
#include "S01ScanPointActor.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class ES01ScanPointKind : uint8
{
	DisplayCaseFossil,
	Battery,
	TRexInfoSign
};

UCLASS()
class AETERNA_API AS01ScanPointActor : public AAeternaInteractableActor
{
	GENERATED_BODY()

public:
	AS01ScanPointActor();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S01")
	ES01ScanPointKind ScanPointKind = ES01ScanPointKind::DisplayCaseFossil;

private:
	void ApplyScanPointDefaults();
};
