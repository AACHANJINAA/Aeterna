// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/S03ScenarioStarterActor.h"

AS03ScenarioStarterActor::AS03ScenarioStarterActor()
{
	ScenarioId = TEXT("S03_ForbiddenLight");

	// 밤2가 끝난 뒤 S02 스타터가 시작시킵니다.
	bStartOnBeginPlay = false;

	// SPEC_NIGHT3 §0: 01:00~05:00
	StartClockMinutes = 60;
	EndClockMinutes = 300;
	GameMinutesPerRealSecond = 1.0f;

	// 밤3의 목표는 켜진 불을 전부 끄는 것입니다. 조명 하나가 목표 1개이므로
	// 배치한 AS03LightActor 수에 맞춰 조정하십시오.
	bConfigurePlayerScanProgress = true;
	RequiredScanCount = 5;
	bResetScanProgressOnStart = true;
	bCompleteScenarioOnRequiredScans = true;
}
