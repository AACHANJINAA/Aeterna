// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaRadarHudComponent.generated.h"

class UAeternaRadarHudWidget;
class UUserWidget;

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaRadarHudComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaRadarHudComponent();

	virtual void BeginPlay() override;
	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Radar")
	void RefreshRadarTargets();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar")
	TSubclassOf<UAeternaRadarHudWidget> RadarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar")
	FName QuestTargetTag = TEXT("MinimapQuest");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar")
	bool bTrackInteractableObjectives = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar")
	bool bTrackCarryObjectives = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar", meta=(ClampMin="100.0", Units="cm"))
	float RadarRange = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar", meta=(ClampMin="0.0", Units="deg/s"))
	float SweepSpeedDegreesPerSecond = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar", meta=(ClampMin="0.05", Units="s"))
	float TargetRefreshIntervalSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Position", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D ViewportAnchorNormalized = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Position")
	FVector2D ViewportOffset = FVector2D(46.0f, 46.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Position")
	FVector2D RadarWidgetSize = FVector2D(184.0f, 184.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar")
	int32 ViewportZOrder = 6;

private:
	void CreateRadarWidget();
	void UpdateRadarWidgetPosition();
	void UpdateRadarState();
	bool ShouldTrackActor(AActor* Actor) const;
	bool IsCarryObjectiveActor(const AActor* Actor) const;
	FName GetCurrentScenarioId() const;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> RadarHudWidget;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> RadarTargets;

	float SweepAngleDegrees = 0.0f;
	float TargetRefreshAccumulatorSeconds = 0.0f;
};
