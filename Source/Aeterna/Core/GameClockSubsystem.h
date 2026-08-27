// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameClockSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FGameClockEventDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clock")
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clock", meta=(ClampMin="0"))
	int32 ClockMinutes = 60;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameClockMinuteChangedSignature, int32, ClockMinutes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameClockEventReachedSignature, FName, EventId, int32, ClockMinutes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameClockFinishedSignature, int32, ClockMinutes);

UCLASS(BlueprintType)
class AETERNA_API UGameClockSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	UFUNCTION(BlueprintCallable, Category="Clock")
	void ConfigureClock(int32 InStartClockMinutes, int32 InEndClockMinutes, float InGameMinutesPerRealSecond);

	UFUNCTION(BlueprintCallable, Category="Clock")
	void SetClockEvents(const TArray<FGameClockEventDefinition>& InClockEvents);

	UFUNCTION(BlueprintCallable, Category="Clock")
	void StartClock();

	UFUNCTION(BlueprintCallable, Category="Clock")
	void PauseClock();

	UFUNCTION(BlueprintCallable, Category="Clock")
	void ResetClock();

	UFUNCTION(BlueprintCallable, Category="Clock")
	void AdvanceClockMinutes(int32 MinutesToAdvance);

	UFUNCTION(BlueprintCallable, Category="Clock")
	void SetClockMinutes(int32 NewClockMinutes);

	UFUNCTION(BlueprintPure, Category="Clock")
	int32 GetClockMinutes() const { return CurrentClockMinutes; }

	UFUNCTION(BlueprintPure, Category="Clock")
	int32 GetEndClockMinutes() const { return EndClockMinutes; }

	UFUNCTION(BlueprintPure, Category="Clock")
	bool IsClockRunning() const { return bClockRunning; }

	UPROPERTY(BlueprintAssignable, Category="Clock")
	FGameClockMinuteChangedSignature OnClockMinuteChanged;

	UPROPERTY(BlueprintAssignable, Category="Clock")
	FGameClockEventReachedSignature OnClockEventReached;

	UPROPERTY(BlueprintAssignable, Category="Clock")
	FGameClockFinishedSignature OnClockFinished;

private:
	void SetClockMinutesInternal(int32 NewClockMinutes);
	void BroadcastReachedEvents(int32 PreviousClockMinutes, int32 NewClockMinutes);

	UPROPERTY(Transient)
	TArray<FGameClockEventDefinition> ClockEvents;

	UPROPERTY(Transient)
	TSet<FName> ReachedEventIds;

	int32 StartClockMinutes = 60;
	int32 EndClockMinutes = 300;
	int32 CurrentClockMinutes = 60;
	float GameMinutesPerRealSecond = 1.0f;
	float MinuteAccumulator = 0.0f;
	bool bClockRunning = false;
	bool bFinished = false;
};
