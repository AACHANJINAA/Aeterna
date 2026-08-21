// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaBatteryHudComponent.generated.h"

class UUserWidget;

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaBatteryHudComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaBatteryHudComponent();

	virtual void BeginPlay() override;
	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Battery|HUD")
	void UpdateBatteryHud(float Current, float Max, float Normalized);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD")
	FText BatteryLabelText = FText::FromString(TEXT("BATTERY"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Position", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D ViewportAnchorNormalized = FVector2D(0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Position")
	FVector2D ViewportOffset = FVector2D(32.0f, 32.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD|Position")
	FVector2D HudWidgetSize = FVector2D(420.0f, 82.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Battery|HUD")
	int32 ViewportZOrder = 5;

private:
	void CreateBatteryHudWidget();
	void ApplyBatteryHud();
	void UpdateBatteryHudPosition();

	float LastCurrentBattery = 1.0f;
	float LastMaxBattery = 1.0f;
	float LastBatteryNormalized = 1.0f;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> BatteryHudWidget;
};
