// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaObjectiveHudComponent.generated.h"

class UAeternaScanProgressComponent;
class UAeternaObjectiveHudUserWidget;
class UUserWidget;

USTRUCT(BlueprintType)
struct FAeternaScenarioObjectiveHudItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective|HUD")
	FText ItemText;

	/** 비워두면 현재 완료 개수 순서로 0/1이 채워집니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective|HUD")
	FName ProgressId;
};

USTRUCT(BlueprintType)
struct FAeternaScenarioObjectiveHudDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective|HUD")
	FName ScenarioId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Objective|HUD")
	TArray<FAeternaScenarioObjectiveHudItem> Items;
};

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaObjectiveHudComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaObjectiveHudComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Objective|HUD")
	void RefreshObjectiveHud();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD")
	TArray<FAeternaScenarioObjectiveHudDefinition> ScenarioObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD")
	TSubclassOf<UAeternaObjectiveHudUserWidget> ObjectiveWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Position", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D ViewportAnchorNormalized = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Position")
	FVector2D ViewportOffset = FVector2D(112.0f, 64.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD|Position")
	FVector2D HudWidgetSize = FVector2D(300.0f, 98.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Objective|HUD")
	int32 ViewportZOrder = 7;

private:
	UFUNCTION()
	void HandleScenarioStarted(FName ScenarioId);

	UFUNCTION()
	void HandleScenarioStateChanged(FName ScenarioId, EScenarioRunState RunState);

	UFUNCTION()
	void HandleScanProgressChanged(int32 CurrentCount, int32 RequiredCount);

	void BindScenarioManager();
	void UnbindScenarioManager();
	void BindScanProgress();
	void UnbindScanProgress();
	void CreateObjectiveHudWidget();
	void ApplyObjectiveHud();
	void UpdateObjectiveHudPosition();
	void UpdateObjectiveHudVisibility();
	const FAeternaScenarioObjectiveHudDefinition* FindObjectiveDefinition(FName ScenarioId) const;
	void BuildCurrentObjectiveItems();

	TWeakObjectPtr<UScenarioManagerSubsystem> BoundScenarioManager;
	TWeakObjectPtr<UAeternaScanProgressComponent> BoundScanProgressComponent;

	FName CurrentScenarioId;
	TArray<FText> CurrentItemTexts;
	TArray<bool> CurrentItemCompleted;
	int32 CurrentCount = 0;
	int32 RequiredCount = 0;
	EScenarioRunState CurrentRunState = EScenarioRunState::None;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ObjectiveHudWidget;
};
