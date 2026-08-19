// Copyright Epic Games, Inc. All Rights Reserved.

#include "AeternaScanProgressComponent.h"

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
