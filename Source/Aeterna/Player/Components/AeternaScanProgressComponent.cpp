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
	const int32 CurrentCount = GetCompletedScanCount();
	OnScanPointRegistered.Broadcast(ScanPointId, CurrentCount);
	OnScanProgressChanged.Broadcast(CurrentCount, RequiredScanCount);
	if (HasCompletedRequiredScans())
	{
		OnRequiredScanCountReached.Broadcast(CurrentCount, RequiredScanCount);
	}

	return true;
}

bool UAeternaScanProgressComponent::HasScannedPoint(FName ScanPointId) const
{
	return !ScanPointId.IsNone() && ScannedPointIds.Contains(ScanPointId);
}

bool UAeternaScanProgressComponent::HasCompletedRequiredScans() const
{
	return RequiredScanCount > 0 && GetCompletedScanCount() >= RequiredScanCount;
}

void UAeternaScanProgressComponent::SetRequiredScanCount(int32 InRequiredScanCount)
{
	RequiredScanCount = FMath::Max(0, InRequiredScanCount);
	const int32 CurrentCount = GetCompletedScanCount();
	OnScanProgressChanged.Broadcast(CurrentCount, RequiredScanCount);
	if (HasCompletedRequiredScans())
	{
		OnRequiredScanCountReached.Broadcast(CurrentCount, RequiredScanCount);
	}
}

void UAeternaScanProgressComponent::ResetScanProgress()
{
	ScannedPointIds.Reset();
	OnScanProgressChanged.Broadcast(0, RequiredScanCount);
}
