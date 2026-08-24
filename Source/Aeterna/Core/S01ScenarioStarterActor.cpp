// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/S01ScenarioStarterActor.h"

AS01ScenarioStarterActor::AS01ScenarioStarterActor()
{
	ScenarioId = TEXT("S01_Handover");
	StartClockMinutes = 60;
	EndClockMinutes = 300;
	GameMinutesPerRealSecond = 1.0f;
	bConfigurePlayerScanProgress = true;
	RequiredScanCount = 3;
	bResetScanProgressOnStart = true;
	bCompleteScenarioOnRequiredScans = true;
}
