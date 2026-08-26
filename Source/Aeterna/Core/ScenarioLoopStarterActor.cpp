// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioLoopStarterActor.h"

#include "Core/BgmSubsystem.h"
#include "Core/GameClockSubsystem.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Core/ScenarioVariantSubsystem.h"
#include "Core/ScreenFadeSubsystem.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AeternaCharacter.h"
#include "Player/Components/AeternaObjectiveHudComponent.h"
#include "Player/Components/AeternaCarryComponent.h"
#include "Player/Components/AeternaScanProgressComponent.h"
#include "Sound/SoundBase.h"

AScenarioLoopStarterActor::AScenarioLoopStarterActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AScenarioLoopStarterActor::BeginPlay()
{
	Super::BeginPlay();

	BindScenarioCompletion();

	if (bStartOnBeginPlay)
	{
		StartScenarioLoop();
	}
}

void AScenarioLoopStarterActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(StartDayCardTimerHandle);
	UnbindScanCompletion();
	UnbindScenarioCompletion();
	UnbindFadeFinished();
	Super::EndPlay(EndPlayReason);
}

void AScenarioLoopStarterActor::StartScenarioLoop()
{
	// Day 카드보다 먼저 켭니다. 카드가 뜨는 동안 이미 음악이 흐르고 있어야 합니다.
	PlayScenarioBgm();

	if (PlayStartDayCard())
	{
		bStartScenarioAfterDayCardPending = true;
		return;
	}

	StartScenarioLoopInternal();
}

void AScenarioLoopStarterActor::StartScenarioLoopInternal()
{
	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->StartScenarioLoop(ScenarioId, StartClockMinutes, EndClockMinutes, GameMinutesPerRealSecond, ClockEvents);
	}

	ConfigurePlayerScanProgress();

	// 배터리는 이어지므로, 램프가 켜진 채 재기동되면 암전 구간 내내 잔량이 샙니다.
	if (bTurnOffHeadlampOnStart)
	{
		if (AAeternaCharacter* AeternaCharacter = Cast<AAeternaCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			AeternaCharacter->SetHeadlampOn(false);
		}
	}

	if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		if (UAeternaObjectiveHudComponent* ObjectiveHudComponent = PlayerCharacter->FindComponentByClass<UAeternaObjectiveHudComponent>())
		{
			ObjectiveHudComponent->RefreshObjectiveHud();
		}
	}
}

void AScenarioLoopStarterActor::CompleteScenario()
{
	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->CompleteCurrentScenario();
	}
}

void AScenarioLoopStarterActor::FailScenarioByRuleViolation()
{
	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}
}

void AScenarioLoopStarterActor::HandleRequiredScanCountReached(int32 CurrentCount, int32 RequiredCount)
{
	(void)CurrentCount;
	(void)RequiredCount;

	if (bCompleteScenarioOnRequiredScans)
	{
		CompleteScenario();
	}
}

void AScenarioLoopStarterActor::HandleScenarioCompleted(FName CompletedScenarioId)
{
	if (!bFadeOutOnScenarioComplete || CompletedScenarioId != ScenarioId || bTransitionPending)
	{
		return;
	}

	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return;
	}

	bTransitionPending = true;
	bRestartPending = false;
	SetPlayerInputLocked(true);

	BindFadeFinished();
	ScreenFadeSubsystem->SetFadeColor(ScenarioCompleteFadeColor);
	ScreenFadeSubsystem->StartFadeOut(ScenarioCompleteFadeDurationSeconds, ScenarioCompleteFadeDelaySeconds);
}

void AScenarioLoopStarterActor::HandleScenarioFailed(FName FailedScenarioId, EScenarioFailureReason FailureReason)
{
	(void)FailureReason;

	if (!bRestartOnRuleViolation || FailedScenarioId != ScenarioId || bTransitionPending)
	{
		return;
	}

	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return;
	}

	bTransitionPending = true;
	bRestartPending = true;
	SetPlayerInputLocked(true);

	// 유예 시간 동안 위반 연출(눈구멍 등장 등)이 진행되고, 그 뒤 페이드아웃이 시작됩니다.
	BindFadeFinished();
	ScreenFadeSubsystem->SetFadeColor(ScenarioCompleteFadeColor);
	ScreenFadeSubsystem->StartFadeOut(ScenarioCompleteFadeDurationSeconds, FailureFadeDelaySeconds);
}

void AScenarioLoopStarterActor::HandleFadeFinished(float TargetAlpha)
{
	// 화면이 완전히 덮인 순간에만 다음 밤으로 넘어갑니다. 페이드인 종료는 무시합니다.
	if (!bTransitionPending || TargetAlpha < 1.0f)
	{
		return;
	}

	bTransitionPending = false;
	UnbindFadeFinished();

	if (bRestartPending)
	{
		bRestartPending = false;
		RestartScenario();
		return;
	}

	if (!bStartNextScenarioAfterFadeOut || !NextScenarioStarter)
	{
		// 이어질 밤이 없으면 검은 화면을 유지하되 입력만 돌려줍니다.
		SetPlayerInputLocked(false);
		return;
	}

	StartNextScenario();
}

void AScenarioLoopStarterActor::RestartScenario()
{
	MovePlayerTo(RestartPlayerStart);

	// 플레이어가 옮긴 뼈를 배치 위치로 되돌립니다. 시나리오 시작에서도 한 번 더 적용됩니다.
	if (UScenarioVariantSubsystem* ScenarioVariantSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScenarioVariantSubsystem>() : nullptr)
	{
		ScenarioVariantSubsystem->RestoreAuthoredTransforms();
	}

	// 같은 시나리오를 처음부터 다시 시작합니다. 클럭·스캔 진행도·운반 기록이 초기화됩니다.
	StartScenarioLoop();

	if (!bPlayedStartDayCardLastStart)
	{
		SetPlayerInputLocked(false);

		if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
		{
			ScreenFadeSubsystem->StartFadeIn(NextScenarioFadeInDurationSeconds, NextScenarioFadeInDelaySeconds);
		}
	}
}

void AScenarioLoopStarterActor::StartNextScenario()
{
	MovePlayerToNextScenarioStart();

	// StartScenarioLoop이 OnScenarioStarted를 브로드캐스트하고,
	// UScenarioVariantSubsystem이 Night_ 태그 액터들을 그 밤의 상태로 갈아끼웁니다.
	NextScenarioStarter->StartScenarioLoop();

	if (!NextScenarioStarter->bPlayedStartDayCardLastStart)
	{
		SetPlayerInputLocked(false);

		if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
		{
			ScreenFadeSubsystem->StartFadeIn(NextScenarioFadeInDurationSeconds, NextScenarioFadeInDelaySeconds);
		}
	}
}

void AScenarioLoopStarterActor::MovePlayerToNextScenarioStart()
{
	MovePlayerTo(NextScenarioPlayerStart);
}

void AScenarioLoopStarterActor::MovePlayerTo(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FRotator TargetRotation = TargetActor->GetActorRotation();

	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
	{
		if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}

	PlayerPawn->TeleportTo(TargetLocation, TargetRotation);

	if (AController* PlayerController = PlayerPawn->GetController())
	{
		PlayerController->SetControlRotation(TargetRotation);
	}
}

void AScenarioLoopStarterActor::SetPlayerInputLocked(bool bLocked)
{
	if (!bLockInputDuringTransition)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerPawn || !PlayerController)
	{
		return;
	}

	if (bLocked)
	{
		PlayerPawn->DisableInput(PlayerController);
	}
	else
	{
		PlayerPawn->EnableInput(PlayerController);
	}
}

bool AScenarioLoopStarterActor::PlayStartDayCard()
{
	bPlayedStartDayCardLastStart = false;

	if (!bShowDayCardOnStart)
	{
		return false;
	}

	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return false;
	}

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->PauseClock();
	}

	GetWorldTimerManager().ClearTimer(StartDayCardTimerHandle);
	SetPlayerInputLocked(true);
	ScreenFadeSubsystem->SetFadeColor(StartDayCardFadeColor);
	ScreenFadeSubsystem->SetFadeAlphaImmediate(1.0f);
	UTexture2D* DayCardTexture = ResolveStartDayCardTexture();
	ScreenFadeSubsystem->SetTitleText(FText::GetEmpty());
	ScreenFadeSubsystem->SetTitleTexture(DayCardTexture);
	ScreenFadeSubsystem->SetTitleAlphaImmediate(0.0f);
	ScreenFadeSubsystem->StartTitleFadeIn(StartDayCardTextureFadeInSeconds, StartDayCardTextureDelaySeconds);

	const float FadeInDelay = StartDayCardTextureDelaySeconds + StartDayCardTextureFadeInSeconds + StartDayCardHoldSeconds;
	ScreenFadeSubsystem->StartFadeIn(StartDayCardFadeInSeconds, FadeInDelay);

	const float UnlockDelay = FMath::Max(0.0f, FadeInDelay + StartDayCardFadeInSeconds);
	GetWorldTimerManager().SetTimer(StartDayCardTimerHandle, this, &AScenarioLoopStarterActor::FinishStartDayCard, UnlockDelay, false);

	bPlayedStartDayCardLastStart = true;
	return true;
}

void AScenarioLoopStarterActor::FinishStartDayCard()
{
	if (UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr)
	{
		ScreenFadeSubsystem->SetTitleAlphaImmediate(0.0f);
		ScreenFadeSubsystem->ClearTitleText();
	}

	if (bStartScenarioAfterDayCardPending)
	{
		bStartScenarioAfterDayCardPending = false;
		StartScenarioLoopInternal();
	}

	SetPlayerInputLocked(false);
}

FText AScenarioLoopStarterActor::BuildStartDayCardText() const
{
	if (!StartDayCardText.IsEmpty())
	{
		return StartDayCardText;
	}

	if (ScenarioId == TEXT("S01_Handover") || ScenarioId == TEXT("S01"))
	{
		return NSLOCTEXT("Aeterna", "StartDayCardS01", "Day 1");
	}

	if (ScenarioId == TEXT("S02_GrandHallFossil"))
	{
		return NSLOCTEXT("Aeterna", "StartDayCardS02", "Day 2");
	}

	if (ScenarioId == TEXT("S03_ForbiddenLight"))
	{
		return NSLOCTEXT("Aeterna", "StartDayCardS03", "Day 3");
	}

	return FText::FromName(ScenarioId);
}

UTexture2D* AScenarioLoopStarterActor::ResolveStartDayCardTexture() const
{
	if (StartDayCardTexture)
	{
		return StartDayCardTexture;
	}

	TArray<const TCHAR*> TexturePaths;
	if (ScenarioId == TEXT("S01_Handover") || ScenarioId == TEXT("S01"))
	{
		TexturePaths.Add(TEXT("/Game/Resource/Texture/Day1.Day1"));
		TexturePaths.Add(TEXT("/Game/Resource/Texture/Day1"));
	}
	else if (ScenarioId == TEXT("S02_GrandHallFossil"))
	{
		TexturePaths.Add(TEXT("/Game/Resource/Texture/Day2.Day2"));
		TexturePaths.Add(TEXT("/Game/Resource/Texture/Day2"));
	}
	else if (ScenarioId == TEXT("S03_ForbiddenLight"))
	{
		TexturePaths.Add(TEXT("/Game/Resource/Texture/Day3.Day3"));
		TexturePaths.Add(TEXT("/Game/Resource/Texture/Day3"));
	}

	for (const TCHAR* TexturePath : TexturePaths)
	{
		if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TexturePath))
		{
			return Texture;
		}
	}

	return nullptr;
}

void AScenarioLoopStarterActor::PlayScenarioBgm()
{
	if (!bPlayBgmOnStart)
	{
		return;
	}

	UBgmSubsystem* BgmSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UBgmSubsystem>() : nullptr;
	if (!BgmSubsystem)
	{
		return;
	}

	// 같은 곡이면 UBgmSubsystem이 알아서 무시하므로, 재시작에서도 그냥 다시 부릅니다.
	if (USoundBase* Bgm = ResolveScenarioBgm())
	{
		BgmSubsystem->PlayBgm(Bgm, BgmVolume, BgmFadeInSeconds, BgmCrossFadeOutSeconds);
	}
}

USoundBase* AScenarioLoopStarterActor::ResolveScenarioBgm() const
{
	if (ScenarioBgm)
	{
		return ScenarioBgm;
	}

	TArray<const TCHAR*> BgmPaths;
	if (ScenarioId == TEXT("S01_Handover") || ScenarioId == TEXT("S01"))
	{
		BgmPaths.Add(TEXT("/Game/Resource/Audio/BGM3.BGM3"));
		BgmPaths.Add(TEXT("/Game/Resource/Audio/BGM3"));
	}
	else if (ScenarioId == TEXT("S02_GrandHallFossil"))
	{
		BgmPaths.Add(TEXT("/Game/Resource/Audio/BGM2.BGM2"));
		BgmPaths.Add(TEXT("/Game/Resource/Audio/BGM2"));
	}
	else if (ScenarioId == TEXT("S03_ForbiddenLight"))
	{
		BgmPaths.Add(TEXT("/Game/Resource/Audio/BGM1.BGM1"));
		BgmPaths.Add(TEXT("/Game/Resource/Audio/BGM1"));
	}

	for (const TCHAR* BgmPath : BgmPaths)
	{
		if (USoundBase* Bgm = LoadObject<USoundBase>(nullptr, BgmPath))
		{
			return Bgm;
		}
	}

	return nullptr;
}

void AScenarioLoopStarterActor::ConfigurePlayerScanProgress()
{
	if (!bConfigurePlayerScanProgress)
	{
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	UAeternaScanProgressComponent* ScanProgressComponent = PlayerCharacter
		? PlayerCharacter->FindComponentByClass<UAeternaScanProgressComponent>()
		: nullptr;
	if (!ScanProgressComponent)
	{
		return;
	}

	UnbindScanCompletion();
	BoundScanProgressComponent = ScanProgressComponent;

	if (bResetScanProgressOnStart)
	{
		ScanProgressComponent->ResetScanProgress();

		if (UAeternaCarryComponent* CarryComponent = PlayerCharacter->FindComponentByClass<UAeternaCarryComponent>())
		{
			CarryComponent->ResetInstallProgress();
		}
	}

	ScanProgressComponent->SetRequiredScanCount(RequiredScanCount);
	BindScanCompletion();

	if (bCompleteScenarioOnRequiredScans && ScanProgressComponent->HasCompletedRequiredScans())
	{
		CompleteScenario();
	}
}

void AScenarioLoopStarterActor::BindScanCompletion()
{
	if (UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get())
	{
		ScanProgressComponent->OnRequiredScanCountReached.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleRequiredScanCountReached);
	}
}

void AScenarioLoopStarterActor::UnbindScanCompletion()
{
	if (UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get())
	{
		ScanProgressComponent->OnRequiredScanCountReached.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleRequiredScanCountReached);
	}
	BoundScanProgressComponent.Reset();
}

void AScenarioLoopStarterActor::BindScenarioCompletion()
{
	UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	if (!ScenarioManager)
	{
		return;
	}

	BoundScenarioManager = ScenarioManager;
	ScenarioManager->OnScenarioCompleted.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleScenarioCompleted);
	ScenarioManager->OnScenarioFailed.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleScenarioFailed);
}

void AScenarioLoopStarterActor::UnbindScenarioCompletion()
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioCompleted.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleScenarioCompleted);
		ScenarioManager->OnScenarioFailed.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleScenarioFailed);
	}
	BoundScenarioManager.Reset();
}

void AScenarioLoopStarterActor::BindFadeFinished()
{
	UScreenFadeSubsystem* ScreenFadeSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UScreenFadeSubsystem>() : nullptr;
	if (!ScreenFadeSubsystem)
	{
		return;
	}

	BoundScreenFadeSubsystem = ScreenFadeSubsystem;
	ScreenFadeSubsystem->OnFadeFinished.AddUniqueDynamic(this, &AScenarioLoopStarterActor::HandleFadeFinished);
}

void AScenarioLoopStarterActor::UnbindFadeFinished()
{
	if (UScreenFadeSubsystem* ScreenFadeSubsystem = BoundScreenFadeSubsystem.Get())
	{
		ScreenFadeSubsystem->OnFadeFinished.RemoveDynamic(this, &AScenarioLoopStarterActor::HandleFadeFinished);
	}
	BoundScreenFadeSubsystem.Reset();
}
