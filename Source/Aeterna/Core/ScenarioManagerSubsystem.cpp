// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioManagerSubsystem.h"

#include "Core/GameClockSubsystem.h"
#include "Engine/Engine.h"

namespace
{
	const uint64 ScenarioLoopDebugMessageKey = 0xA37E0001;
	const uint64 ScenarioLoopTimeDebugMessageKey = 0xA37E0002;
	constexpr float ScenarioLoopDebugMessageDuration = 6.0f;
	constexpr float ScenarioLoopTimeDebugMessageDuration = 0.05f;
}

void UScenarioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->OnClockFinished.AddUniqueDynamic(this, &UScenarioManagerSubsystem::HandleClockFinished);
	}
}

void UScenarioManagerSubsystem::Deinitialize()
{
	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->OnClockFinished.RemoveDynamic(this, &UScenarioManagerSubsystem::HandleClockFinished);
	}

	if (GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(ScenarioLoopDebugMessageKey);
		GEngine->RemoveOnScreenDebugMessage(ScenarioLoopTimeDebugMessageKey);
	}

	Super::Deinitialize();
}

void UScenarioManagerSubsystem::Tick(float DeltaTime)
{
	(void)DeltaTime;
	UpdateLoopTimeDebugLog();
}

TStatId UScenarioManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UScenarioManagerSubsystem, STATGROUP_Tickables);
}

bool UScenarioManagerSubsystem::IsTickable() const
{
	return !IsTemplate() && bShowLoopDebugLog && HasCurrentScenario();
}

void UScenarioManagerSubsystem::StartScenario(FName ScenarioId)
{
	if (ScenarioId.IsNone())
	{
		return;
	}

	CurrentScenarioId = ScenarioId;
	LastFailureReason = EScenarioFailureReason::None;
	SetRunState(EScenarioRunState::Starting);
	OnScenarioStarted.Broadcast(CurrentScenarioId);
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::StartScenarioLoop(FName ScenarioId, int32 StartClockMinutes, int32 EndClockMinutes, float GameMinutesPerRealSecond, const TArray<FGameClockEventDefinition>& ClockEvents)
{
	StartScenario(ScenarioId);
	if (!HasCurrentScenario())
	{
		return;
	}

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->ConfigureClock(StartClockMinutes, EndClockMinutes, GameMinutesPerRealSecond);
		GameClockSubsystem->SetClockEvents(ClockEvents);
		GameClockSubsystem->StartClock();
	}

	MarkScenarioRunning();
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::MarkScenarioRunning()
{
	if (!HasCurrentScenario())
	{
		return;
	}

	SetRunState(EScenarioRunState::Running);
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::RequestRestartCurrentScenario()
{
	if (!HasCurrentScenario())
	{
		return;
	}

	SetRunState(EScenarioRunState::Restarting);
	LastFailureReason = EScenarioFailureReason::None;
	OnScenarioRestartRequested.Broadcast(CurrentScenarioId);
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::FailCurrentScenarioByRuleViolation()
{
	FailCurrentScenario(EScenarioFailureReason::RuleViolation);
}

void UScenarioManagerSubsystem::FailCurrentScenarioByTimeExpired()
{
	FailCurrentScenario(EScenarioFailureReason::TimeExpired);
}

void UScenarioManagerSubsystem::FailCurrentScenarioByBatteryDepleted()
{
	FailCurrentScenario(EScenarioFailureReason::BatteryDepleted);
}

void UScenarioManagerSubsystem::CompleteCurrentScenario()
{
	if (!HasCurrentScenario())
	{
		return;
	}

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->PauseClock();
	}

	SetRunState(EScenarioRunState::Completed);
	OnScenarioCompleted.Broadcast(CurrentScenarioId);
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::ClearCurrentScenario()
{
	CurrentScenarioId = NAME_None;
	LastFailureReason = EScenarioFailureReason::None;
	SetRunState(EScenarioRunState::None);
}

void UScenarioManagerSubsystem::SetLoopDebugLogVisible(bool bVisible)
{
	// Scenario loop screen logging was useful during loop bring-up, but it overlaps
	// player-facing HUD now. Keep the Blueprint API as a harmless cleanup hook.
	bShowLoopDebugLog = false;
	if (!bShowLoopDebugLog && GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(ScenarioLoopDebugMessageKey);
		GEngine->RemoveOnScreenDebugMessage(ScenarioLoopTimeDebugMessageKey);
		return;
	}

	ShowLoopDebugLog();
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

void UScenarioManagerSubsystem::FailCurrentScenario(EScenarioFailureReason FailureReason)
{
	if (!HasCurrentScenario()
		|| FailureReason == EScenarioFailureReason::None
		|| RunState == EScenarioRunState::Completed
		|| RunState == EScenarioRunState::Failed
		|| RunState == EScenarioRunState::Restarting)
	{
		return;
	}

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->PauseClock();
	}

	LastFailureReason = FailureReason;
	SetRunState(EScenarioRunState::Failed);
	OnScenarioFailed.Broadcast(CurrentScenarioId, LastFailureReason);
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::HandleClockFinished(int32 ClockMinutes)
{
	(void)ClockMinutes;

	if (!HasCurrentScenario() || RunState == EScenarioRunState::Completed || RunState == EScenarioRunState::Failed)
	{
		return;
	}

	OnScenarioTimeExpired.Broadcast(CurrentScenarioId);
	ShowLoopDebugLog();
}

void UScenarioManagerSubsystem::ShowLoopDebugLog() const
{
	if (!bShowLoopDebugLog || !GEngine)
	{
		return;
	}

	GEngine->RemoveOnScreenDebugMessage(ScenarioLoopDebugMessageKey);
	GEngine->AddOnScreenDebugMessage(
		ScenarioLoopDebugMessageKey,
		ScenarioLoopDebugMessageDuration,
		FColor::Cyan,
		BuildLoopDebugLogText());
}

void UScenarioManagerSubsystem::UpdateLoopTimeDebugLog() const
{
	if (!bShowLoopDebugLog || !GEngine)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(
		ScenarioLoopTimeDebugMessageKey,
		ScenarioLoopTimeDebugMessageDuration,
		FColor::Cyan,
		BuildLoopTimeDebugLogText());
}

FString UScenarioManagerSubsystem::BuildLoopDebugLogText() const
{
	const UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr;
	const int32 ClockMinutes = GameClockSubsystem ? GameClockSubsystem->GetClockMinutes() : 0;

	return FString::Printf(
		TEXT("[ScenarioLoop] Scenario=%s State=%s Time=%02d:%02d Failure=%s"),
		*CurrentScenarioId.ToString(),
		*UEnum::GetValueAsString(RunState),
		ClockMinutes / 60,
		ClockMinutes % 60,
		*UEnum::GetValueAsString(LastFailureReason));
}

FString UScenarioManagerSubsystem::BuildLoopTimeDebugLogText() const
{
	const UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr;
	const int32 ClockMinutes = GameClockSubsystem ? GameClockSubsystem->GetClockMinutes() : 0;

	return FString::Printf(
		TEXT("Time=%02d:%02d"),
		ClockMinutes / 60,
		ClockMinutes % 60);
}
