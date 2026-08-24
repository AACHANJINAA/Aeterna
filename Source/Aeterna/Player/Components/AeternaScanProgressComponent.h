// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaScanProgressComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScanProgressChangedSignature, int32, CurrentCount, int32, RequiredCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FScanPointRegisteredSignature, FName, ScanPointId, int32, CurrentCount);

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

	UFUNCTION(BlueprintPure, Category="Scan")
	bool HasCompletedRequiredScans() const;

	UFUNCTION(BlueprintCallable, Category="Scan")
	void SetRequiredScanCount(int32 InRequiredScanCount);

	UFUNCTION(BlueprintCallable, Category="Scan")
	void ResetScanProgress();

	UPROPERTY(BlueprintAssignable, Category="Scan")
	FScanPointRegisteredSignature OnScanPointRegistered;

	UPROPERTY(BlueprintAssignable, Category="Scan")
	FScanProgressChangedSignature OnScanProgressChanged;

	UPROPERTY(BlueprintAssignable, Category="Scan")
	FScanProgressChangedSignature OnRequiredScanCountReached;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scan", meta=(ClampMin="0"))
	int32 RequiredScanCount = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scan")
	TSet<FName> ScannedPointIds;
};
