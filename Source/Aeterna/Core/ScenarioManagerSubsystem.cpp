// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioManagerSubsystem.h"

void UScenarioManagerSubsystem::StartScenario(FName ScenarioId)
{
	if (ScenarioId.IsNone())
	{
		return;
	}

	CurrentScenarioId = ScenarioId;
	SetRunState(EScenarioRunState::Starting);
	OnScenarioStarted.Broadcast(CurrentScenarioId);
}

void UScenarioManagerSubsystem::MarkScenarioRunning()
{
	if (!HasCurrentScenario())
	{
		return;
	}

	SetRunState(EScenarioRunState::Running);
}

void UScenarioManagerSubsystem::RequestRestartCurrentScenario()
{
	if (!HasCurrentScenario())
	{
		return;
	}

	SetRunState(EScenarioRunState::Restarting);
	OnScenarioRestartRequested.Broadcast(CurrentScenarioId);
}

void UScenarioManagerSubsystem::CompleteCurrentScenario()
{
	if (!HasCurrentScenario())
	{
		return;
	}

	SetRunState(EScenarioRunState::Completed);
	OnScenarioCompleted.Broadcast(CurrentScenarioId);
}

void UScenarioManagerSubsystem::ClearCurrentScenario()
{
	CurrentScenarioId = NAME_None;
	SetRunState(EScenarioRunState::None);
}

void UScenarioManagerSubsystem::SetRunState(EScenarioRunState NewRunState)
{
	if (RunState == NewRunState)
	{
		return;
	}

	RunState = NewRunState;
	OnScenarioStateChanged.Broadcast(CurrentScenarioId, RunState);
}
