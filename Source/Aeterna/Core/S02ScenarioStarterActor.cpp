// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/S02ScenarioStarterActor.h"

AS02ScenarioStarterActor::AS02ScenarioStarterActor()
{
	ScenarioId = TEXT("S02_GrandHallFossil");

	// 밤1이 끝난 뒤 S01 스타터가 시작시킵니다.
	bStartOnBeginPlay = false;

	// SPEC_NIGHT2 5: 01:00~05:00, 1 게임분 = 실시간 5초
	StartClockMinutes = 60;
	EndClockMinutes = 300;
	GameMinutesPerRealSecond = 0.2f;

	// 밤2의 목표는 뼈 3개 설치입니다 (SPEC_NIGHT2 §0). 진행도 컴포넌트를 그대로 씁니다.
	bConfigurePlayerScanProgress = true;
	RequiredScanCount = 3;
	bResetScanProgressOnStart = true;
	bCompleteScenarioOnRequiredScans = true;
}
