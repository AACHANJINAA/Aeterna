// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaBatteryComponent.h"

#include "Aeterna.h"

#include "Components/SpotLightComponent.h"

UAeternaBatteryComponent::UAeternaBatteryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAeternaBatteryComponent::InitializeBattery(USpotLightComponent* InHeadlampComponent)
{
	HeadlampComponent = InHeadlampComponent;
	CurrentBattery = FMath::Clamp(CurrentBattery, 0.0f, MaxBattery);
	UpdateHeadlampBrightness();
	UpdateBatteryDebugString(false);
}

bool UAeternaBatteryComponent::TickBattery(float DeltaSeconds, bool bHeadlampOn)
{
	if (!bHeadlampOn || HeadlampBatteryDrainPerSecond <= 0.0f)
	{
		return false;
	}

	const float PreviousBattery = CurrentBattery;
	CurrentBattery = FMath::Max(CurrentBattery - HeadlampBatteryDrainPerSecond * DeltaSeconds, 0.0f);
	UpdateHeadlampBrightness();
	UpdateBatteryDebugString(bHeadlampOn);

	return !FMath::IsNearlyEqual(PreviousBattery, CurrentBattery);
}

void UAeternaBatteryComponent::AddBattery(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	const float PreviousBattery = CurrentBattery;
	CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBattery);
	LastBatteryChargeAmount = CurrentBattery - PreviousBattery;
	UpdateHeadlampBrightness();
	UpdateBatteryDebugString(false);
}

void UAeternaBatteryComponent::ResetBatteryToFull()
{
	CurrentBattery = MaxBattery;
	LastBatteryChargeAmount = 0.0f;
	UpdateHeadlampBrightness();
	UpdateBatteryDebugString(false);
}

void UAeternaBatteryComponent::UpdateHeadlampBrightness()
{
	if (!HeadlampComponent)
	{
		return;
	}

	const float BatteryAlpha = FMath::Clamp(GetBatteryNormalized(), 0.0f, 1.0f);
	const float WeightedAlpha = FMath::Pow(BatteryAlpha, BatteryBrightnessExponent);

	if (BatteryAlpha <= 0.0f)
	{
		HeadlampComponent->SetVisibility(false);
		HeadlampComponent->SetIntensity(0.0f);
		return;
	}

	HeadlampComponent->SetIntensity(FMath::Lerp(LowBatteryLightIntensity, FullBatteryLightIntensity, WeightedAlpha));
	HeadlampComponent->SetAttenuationRadius(FMath::Lerp(LowBatteryAttenuationRadius, FullBatteryAttenuationRadius, WeightedAlpha));
	HeadlampComponent->SetLightColor(FMath::Lerp(LowBatteryLightColor, FullBatteryLightColor, WeightedAlpha));
	HeadlampComponent->SetTemperature(FMath::Lerp(LowBatteryTemperature, FullBatteryTemperature, WeightedAlpha));

	const float InnerConeAngle = FMath::Lerp(LowBatteryInnerConeAngle, FullBatteryInnerConeAngle, WeightedAlpha);
	const float OuterConeAngle = FMath::Max(FMath::Lerp(LowBatteryOuterConeAngle, FullBatteryOuterConeAngle, WeightedAlpha), InnerConeAngle);
	HeadlampComponent->SetInnerConeAngle(InnerConeAngle);
	HeadlampComponent->SetOuterConeAngle(OuterConeAngle);
}

void UAeternaBatteryComponent::UpdateBatteryDebugString(bool bHeadlampOn)
{
	BatteryDebugString = FString::Printf(
		TEXT("Battery %.1f / %.1f (%.0f%%) | Headlamp %s | Drain %.1f/s"),
		CurrentBattery,
		MaxBattery,
		GetBatteryNormalized() * 100.0f,
		bHeadlampOn ? TEXT("ON") : TEXT("OFF"),
		bHeadlampOn ? HeadlampBatteryDrainPerSecond : 0.0f);
}

float UAeternaBatteryComponent::GetBatteryNormalized() const
{
	return MaxBattery > 0.0f ? CurrentBattery / MaxBattery : 0.0f;
}
