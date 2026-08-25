// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScreenFadeSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UI/ScreenFadeWidget.h"

namespace
{
	constexpr int32 ScreenFadeWidgetZOrder = 1000;
}

void UScreenFadeSubsystem::Deinitialize()
{
	RemoveFadeWidget();
	Super::Deinitialize();
}

void UScreenFadeSubsystem::Tick(float DeltaTime)
{
	if (!bFadeActive)
	{
		return;
	}

	if (RemainingDelaySeconds > 0.0f)
	{
		RemainingDelaySeconds -= DeltaTime;
		if (RemainingDelaySeconds > 0.0f)
		{
			// 대기 중에도 위젯을 미리 붙여 첫 프레임 끊김을 막습니다.
			ApplyFadeAlpha(CurrentAlpha);
			return;
		}

		// 대기가 끝나고 남은 시간은 이번 프레임의 진행분으로 씁니다.
		DeltaTime = -RemainingDelaySeconds;
		RemainingDelaySeconds = 0.0f;
	}

	ElapsedSeconds += DeltaTime;

	const float Progress = (FadeDuration > KINDA_SMALL_NUMBER)
		? FMath::Clamp(ElapsedSeconds / FadeDuration, 0.0f, 1.0f)
		: 1.0f;

	ApplyFadeAlpha(FMath::Lerp(StartAlpha, TargetAlpha, Progress));

	if (Progress >= 1.0f)
	{
		bFadeActive = false;
		if (TargetAlpha <= 0.0f)
		{
			RemoveFadeWidget();
		}

		OnFadeFinished.Broadcast(TargetAlpha);
	}
}

TStatId UScreenFadeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UScreenFadeSubsystem, STATGROUP_Tickables);
}

bool UScreenFadeSubsystem::IsTickable() const
{
	return !IsTemplate() && bFadeActive;
}

void UScreenFadeSubsystem::StartFadeOut(float DurationSeconds, float DelaySeconds)
{
	StartFade(1.0f, DurationSeconds, DelaySeconds);
}

void UScreenFadeSubsystem::StartFadeIn(float DurationSeconds, float DelaySeconds)
{
	StartFade(0.0f, DurationSeconds, DelaySeconds);
}

void UScreenFadeSubsystem::SetFadeAlphaImmediate(float InFadeAlpha)
{
	bFadeActive = false;
	RemainingDelaySeconds = 0.0f;
	ElapsedSeconds = 0.0f;

	const float ClampedAlpha = FMath::Clamp(InFadeAlpha, 0.0f, 1.0f);
	StartAlpha = ClampedAlpha;
	TargetAlpha = ClampedAlpha;
	ApplyFadeAlpha(ClampedAlpha);

	if (ClampedAlpha <= 0.0f)
	{
		RemoveFadeWidget();
	}
}

void UScreenFadeSubsystem::ClearFade()
{
	SetFadeAlphaImmediate(0.0f);
}

void UScreenFadeSubsystem::SetFadeColor(FLinearColor InFadeColor)
{
	FadeColor = InFadeColor;
	if (FadeWidget)
	{
		FadeWidget->SetFadeColor(FadeColor);
	}
}

void UScreenFadeSubsystem::StartFade(float InTargetAlpha, float DurationSeconds, float DelaySeconds)
{
	const float ClampedTarget = FMath::Clamp(InTargetAlpha, 0.0f, 1.0f);

	// 같은 목표로 이미 진행 중이거나 이미 도달한 상태면 다시 시작하지 않습니다.
	if (bFadeActive && FMath::IsNearlyEqual(TargetAlpha, ClampedTarget))
	{
		return;
	}

	if (!bFadeActive && FMath::IsNearlyEqual(CurrentAlpha, ClampedTarget))
	{
		return;
	}

	StartAlpha = CurrentAlpha;
	TargetAlpha = ClampedTarget;
	FadeDuration = FMath::Max(DurationSeconds, 0.0f);
	RemainingDelaySeconds = FMath::Max(DelaySeconds, 0.0f);
	ElapsedSeconds = 0.0f;
	bFadeActive = true;

	// 대기 시간이 0이어도 첫 프레임부터 시작 알파가 보이도록 미리 반영합니다.
	ApplyFadeAlpha(CurrentAlpha);
}

void UScreenFadeSubsystem::ApplyFadeAlpha(float InFadeAlpha)
{
	CurrentAlpha = FMath::Clamp(InFadeAlpha, 0.0f, 1.0f);

	if (UScreenFadeWidget* Widget = EnsureFadeWidget())
	{
		Widget->SetFadeAlpha(CurrentAlpha);
	}
}

UScreenFadeWidget* UScreenFadeSubsystem::EnsureFadeWidget()
{
	if (FadeWidget)
	{
		return FadeWidget;
	}

	// 알파가 0이면 굳이 위젯을 만들지 않습니다.
	if (CurrentAlpha <= 0.0f && TargetAlpha <= 0.0f)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return nullptr;
	}

	FadeWidget = CreateWidget<UScreenFadeWidget>(PlayerController, UScreenFadeWidget::StaticClass());
	if (!FadeWidget)
	{
		return nullptr;
	}

	FadeWidget->SetFadeColor(FadeColor);
	FadeWidget->SetFadeAlpha(CurrentAlpha);
	FadeWidget->AddToViewport(ScreenFadeWidgetZOrder);

	return FadeWidget;
}

void UScreenFadeSubsystem::RemoveFadeWidget()
{
	if (!FadeWidget)
	{
		return;
	}

	FadeWidget->RemoveFromParent();
	FadeWidget = nullptr;
}
