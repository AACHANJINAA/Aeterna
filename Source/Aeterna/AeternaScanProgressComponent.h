// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AeternaPlayerComponent.h"
#include "AeternaScanProgressComponent.generated.h"

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaScanProgressComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaScanProgressComponent();

	UFUNCTION(BlueprintCallable, Category="Scan")
	bool RegisterScanPoint(FName ScanPointId);

	UFUNCTION(BlueprintPure, Category="Scan")
	bool HasScannedPoint(FName ScanPointId) const;

	UFUNCTION(BlueprintPure, Category="Scan")
	int32 GetCompletedScanCount() const { return ScannedPointIds.Num(); }

	UFUNCTION(BlueprintPure, Category="Scan")
	int32 GetRequiredScanCount() const { return RequiredScanCount; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scan", meta=(ClampMin="0"))
	int32 RequiredScanCount = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scan")
	TSet<FName> ScannedPointIds;
};
