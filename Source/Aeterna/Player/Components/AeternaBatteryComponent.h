// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaBatteryComponent.generated.h"

class USpotLightComponent;

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaBatteryComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaBatteryComponent();

	void InitializeBattery(USpotLightComponent* InHeadlampComponent);

	UFUNCTION(BlueprintCallable, Category="Battery")
	bool TickBattery(float DeltaSeconds, bool bHeadlampOn);

	UFUNCTION(BlueprintCallable, Category="Battery")
	void AddBattery(float Amount);

	UFUNCTION(BlueprintCallable, Category="Battery")
	void ResetBatteryToFull();

	UFUNCTION(BlueprintCallable, Category="Headlamp")
	void UpdateHeadlampBrightness();

	UFUNCTION(BlueprintCallable, Category="Battery|Debug")
	void UpdateBatteryDebugString(bool bHeadlampOn);

	UFUNCTION(BlueprintPure, Category="Battery")
	float GetCurrentBattery() const { return CurrentBattery; }

	UFUNCTION(BlueprintPure, Category="Battery")
	float GetMaxBattery() const { return MaxBattery; }

	UFUNCTION(BlueprintPure, Category="Battery")
	float GetLastBatteryChargeAmount() const { return LastBatteryChargeAmount; }

	UFUNCTION(BlueprintPure, Category="Battery")
	float GetBatteryNormalized() const;

	UFUNCTION(BlueprintPure, Category="Battery|Debug")
	FString GetBatteryDebugString() const { return BatteryDebugString; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery", meta=(ClampMin="1.0"))
	float MaxBattery = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Battery")
	float CurrentBattery = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Battery")
	float LastBatteryChargeAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery", meta=(ClampMin="0.0"))
	float HeadlampBatteryDrainPerSecond = 1.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Battery|Debug")
	FString BatteryDebugString;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="0.0"))
	float FullBatteryLightIntensity = 12300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="0.0"))
	float LowBatteryLightIntensity = 4300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="0.0", Units="cm"))
	float FullBatteryAttenuationRadius = 2575.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="0.0", Units="cm"))
	float LowBatteryAttenuationRadius = 975.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Color")
	FLinearColor FullBatteryLightColor = FLinearColor(1.0f, 0.95f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Color")
	FLinearColor LowBatteryLightColor = FLinearColor(1.0f, 0.45f, 0.22f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="1000.0", Units="K"))
	float FullBatteryTemperature = 6500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="1000.0", Units="K"))
	float LowBatteryTemperature = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Shape", meta=(ClampMin="0.0", ClampMax="80.0", Units="deg"))
	float FullBatteryInnerConeAngle = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Shape", meta=(ClampMin="0.0", ClampMax="80.0", Units="deg"))
	float LowBatteryInnerConeAngle = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Shape", meta=(ClampMin="1.0", ClampMax="80.0", Units="deg"))
	float FullBatteryOuterConeAngle = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Shape", meta=(ClampMin="1.0", ClampMax="80.0", Units="deg"))
	float LowBatteryOuterConeAngle = 29.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Headlamp|Brightness", meta=(ClampMin="0.1"))
	float BatteryBrightnessExponent = 1.8f;

private:
	TObjectPtr<USpotLightComponent> HeadlampComponent;
};
