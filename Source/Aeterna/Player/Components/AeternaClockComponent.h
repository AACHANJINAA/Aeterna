// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaClockComponent.generated.h"

class UUserWidget;

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaClockComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaClockComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Clock")
	void AdvanceClockMinutes(int32 MinutesToAdvance);

	UFUNCTION(BlueprintCallable, Category="Clock|Debug")
	void AdvanceDebugClockStep();

	UFUNCTION(BlueprintPure, Category="Clock")
	int32 GetClockMinutes() const { return CurrentClockMinutes; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock", meta=(ClampMin="0"))
	int32 StartClockMinutes = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock", meta=(ClampMin="0"))
	int32 MaxClockMinutes = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|Debug", meta=(ClampMin="1"))
	int32 DebugAdvanceMinutes = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Position")
	FVector2D TopCenterOffset = FVector2D(0.0f, 32.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Position")
	FVector2D HudWidgetSize = FVector2D(338.0f, 110.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD|Position")
	FVector2D ReferenceViewportSize = FVector2D(1920.0f, 1080.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Clock|HUD")
	int32 ViewportZOrder = 6;

private:
	void CreateClockHudWidget();
	bool ApplyClockHud();
	bool UpdateClockHudPosition();

	UFUNCTION()
	void HandleGameClockMinuteChanged(int32 InClockMinutes);

	int32 CurrentClockMinutes = 60;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ClockHudWidget;
};
