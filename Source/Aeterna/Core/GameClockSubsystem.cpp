// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/GameClockSubsystem.h"

#include "Aeterna.h"

void UGameClockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetClock();
}

void UGameClockSubsystem::Tick(float DeltaTime)
{
	if (!bClockRunning || bFinished || GameMinutesPerRealSecond <= 0.0f)
	{
		return;
	}

	MinuteAccumulator += DeltaTime * GameMinutesPerRealSecond;
	const int32 WholeMinutesToAdvance = FMath::FloorToInt(MinuteAccumulator);
	if (WholeMinutesToAdvance <= 0)
	{
		return;
	}

	MinuteAccumulator -= static_cast<float>(WholeMinutesToAdvance);
	AdvanceClockMinutes(WholeMinutesToAdvance);
}

TStatId UGameClockSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGameClockSubsystem, STATGROUP_Tickables);
}

bool UGameClockSubsystem::IsTickable() const
{
	return !IsTemplate();
}

void UGameClockSubsystem::ConfigureClock(int32 InStartClockMinutes, int32 InEndClockMinutes, float InGameMinutesPerRealSecond)
{
	const bool bWasClockRunning = bClockRunning;
	StartClockMinutes = FMath::Max(0, InStartClockMinutes);
	EndClockMinutes = FMath::Max(StartClockMinutes, InEndClockMinutes);
	GameMinutesPerRealSecond = FMath::Max(0.0f, InGameMinutesPerRealSecond);
	ResetClock();
	if (bWasClockRunning)
	{
		StartClock();
	}
}

void UGameClockSubsystem::SetClockEvents(const TArray<FGameClockEventDefinition>& InClockEvents)
{
	ClockEvents = InClockEvents;
	ClockEvents.Sort([](const FGameClockEventDefinition& Left, const FGameClockEventDefinition& Right)
	{
		return Left.ClockMinutes < Right.ClockMinutes;
	});

	ReachedEventIds.Reset();
}

void UGameClockSubsystem::StartClock()
{
	bClockRunning = !bFinished;
}

void UGameClockSubsystem::PauseClock()
{
	bClockRunning = false;
}

void UGameClockSubsystem::ResetClock()
{
	CurrentClockMinutes = StartClockMinutes;
	MinuteAccumulator = 0.0f;
	bClockRunning = false;
	bFinished = false;
	ReachedEventIds.Reset();
	OnClockMinuteChanged.Broadcast(CurrentClockMinutes);
}

void UGameClockSubsystem::AdvanceClockMinutes(int32 MinutesToAdvance)
{
	if (MinutesToAdvance <= 0 || bFinished)
	{
		return;
	}

	SetClockMinutesInternal(CurrentClockMinutes + MinutesToAdvance);
}

void UGameClockSubsystem::SetClockMinutes(int32 NewClockMinutes)
{
	if (bFinished)
	{
		return;
	}

	SetClockMinutesInternal(NewClockMinutes);
}

void UGameClockSubsystem::SetClockMinutesInternal(int32 NewClockMinutes)
{
	const int32 PreviousClockMinutes = CurrentClockMinutes;
	CurrentClockMinutes = FMath::Clamp(NewClockMinutes, StartClockMinutes, EndClockMinutes);
	if (PreviousClockMinutes == CurrentClockMinutes)
	{
		return;
	}

	OnClockMinuteChanged.Broadcast(CurrentClockMinutes);
	BroadcastReachedEvents(PreviousClockMinutes, CurrentClockMinutes);

	if (CurrentClockMinutes >= EndClockMinutes && !bFinished)
	{
		bFinished = true;
		bClockRunning = false;
		OnClockFinished.Broadcast(CurrentClockMinutes);
	}
}

void UGameClockSubsystem::BroadcastReachedEvents(int32 PreviousClockMinutes, int32 NewClockMinutes)
{
	for (const FGameClockEventDefinition& ClockEvent : ClockEvents)
	{
		if (ClockEvent.EventId.IsNone() || ReachedEventIds.Contains(ClockEvent.EventId))
		{
			continue;
		}

		if (ClockEvent.ClockMinutes > PreviousClockMinutes && ClockEvent.ClockMinutes <= NewClockMinutes)
		{
			ReachedEventIds.Add(ClockEvent.EventId);
			OnClockEventReached.Broadcast(ClockEvent.EventId, ClockEvent.ClockMinutes);
		}
	}
}
