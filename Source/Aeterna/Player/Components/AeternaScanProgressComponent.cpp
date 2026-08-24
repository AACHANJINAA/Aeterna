// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaScanProgressComponent.h"

UAeternaScanProgressComponent::UAeternaScanProgressComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAeternaScanProgressComponent::RegisterScanPoint(FName ScanPointId)
{
	if (ScanPointId.IsNone() || ScannedPointIds.Contains(ScanPointId))
	{
		return false;
	}

	ScannedPointIds.Add(ScanPointId);
	return true;
}

bool UAeternaScanProgressComponent::HasScannedPoint(FName ScanPointId) const
{
	return !ScanPointId.IsNone() && ScannedPointIds.Contains(ScanPointId);
}

void UAeternaScanProgressComponent::SetRequiredScanCount(int32 InRequiredScanCount)
{
	RequiredScanCount = FMath::Max(0, InRequiredScanCount);
}

void UAeternaScanProgressComponent::ResetScanProgress()
{
	ScannedPointIds.Reset();
}
