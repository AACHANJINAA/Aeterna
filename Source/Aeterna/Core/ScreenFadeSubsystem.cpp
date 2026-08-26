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
	if (!bFadeActive && !bTitleFadeActive)
	{
		return;
	}

	float FadeDeltaTime = DeltaTime;
	if (bFadeActive && RemainingDelaySeconds > 0.0f)
	{
		RemainingDelaySeconds -= FadeDeltaTime;
		if (RemainingDelaySeconds > 0.0f)
		{
			// 대기 중에도 위젯을 미리 붙여 첫 프레임 끊김을 막습니다.
			ApplyFadeAlpha(CurrentAlpha);
			FadeDeltaTime = 0.0f;
		}
		else
		{
			// 대기가 끝나고 남은 시간은 이번 프레임의 진행분으로 씁니다.
			FadeDeltaTime = -RemainingDelaySeconds;
			RemainingDelaySeconds = 0.0f;
		}
	}

	if (bFadeActive && FadeDeltaTime > 0.0f)
	{
		ElapsedSeconds += FadeDeltaTime;

		const float Progress = (FadeDuration > KINDA_SMALL_NUMBER)
			? FMath::Clamp(ElapsedSeconds / FadeDuration, 0.0f, 1.0f)
			: 1.0f;

		ApplyFadeAlpha(FMath::Lerp(StartAlpha, TargetAlpha, Progress));

		if (Progress >= 1.0f)
		{
			bFadeActive = false;
			if (TargetAlpha <= 0.0f && CurrentTitleAlpha <= 0.0f)
			{
				RemoveFadeWidget();
			}

			OnFadeFinished.Broadcast(TargetAlpha);
		}
	}

	float TitleDeltaTime = DeltaTime;
	if (bTitleFadeActive && TitleRemainingDelaySeconds > 0.0f)
	{
		TitleRemainingDelaySeconds -= TitleDeltaTime;
		if (TitleRemainingDelaySeconds > 0.0f)
		{
			ApplyTitleAlpha(CurrentTitleAlpha);
			TitleDeltaTime = 0.0f;
		}
		else
		{
			TitleDeltaTime = -TitleRemainingDelaySeconds;
			TitleRemainingDelaySeconds = 0.0f;
		}
	}

	if (bTitleFadeActive && TitleDeltaTime > 0.0f)
	{
		TitleElapsedSeconds += TitleDeltaTime;

		const float TitleProgress = (TitleFadeDuration > KINDA_SMALL_NUMBER)
			? FMath::Clamp(TitleElapsedSeconds / TitleFadeDuration, 0.0f, 1.0f)
			: 1.0f;

		ApplyTitleAlpha(FMath::Lerp(StartTitleAlpha, TargetTitleAlpha, TitleProgress));

		if (TitleProgress >= 1.0f)
		{
			bTitleFadeActive = false;
		}
	}
}

TStatId UScreenFadeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UScreenFadeSubsystem, STATGROUP_Tickables);
}

bool UScreenFadeSubsystem::IsTickable() const
{
	return !IsTemplate() && (bFadeActive || bTitleFadeActive);
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

	if (ClampedAlpha <= 0.0f && CurrentTitleAlpha <= 0.0f)
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

void UScreenFadeSubsystem::SetTitleText(FText InTitleText)
{
	TitleText = InTitleText;
	if (UScreenFadeWidget* Widget = EnsureFadeWidget())
	{
		Widget->SetTitleText(TitleText);
	}
}

void UScreenFadeSubsystem::SetTitleTexture(UTexture2D* InTitleTexture)
{
	TitleTexture = InTitleTexture;
	if (UScreenFadeWidget* Widget = EnsureFadeWidget())
	{
		Widget->SetTitleTexture(TitleTexture);
	}
}

void UScreenFadeSubsystem::SetTitleAlphaImmediate(float InTitleAlpha)
{
	bTitleFadeActive = false;
	TitleRemainingDelaySeconds = 0.0f;
	TitleElapsedSeconds = 0.0f;

	const float ClampedAlpha = FMath::Clamp(InTitleAlpha, 0.0f, 1.0f);
	StartTitleAlpha = ClampedAlpha;
	TargetTitleAlpha = ClampedAlpha;
	ApplyTitleAlpha(ClampedAlpha);

	if (CurrentAlpha <= 0.0f && ClampedAlpha <= 0.0f)
	{
		RemoveFadeWidget();
	}
}

void UScreenFadeSubsystem::StartTitleFadeIn(float DurationSeconds, float DelaySeconds)
{
	StartTitleFade(1.0f, DurationSeconds, DelaySeconds);
}

void UScreenFadeSubsystem::StartTitleFadeOut(float DurationSeconds, float DelaySeconds)
{
	StartTitleFade(0.0f, DurationSeconds, DelaySeconds);
}

void UScreenFadeSubsystem::ClearTitleText()
{
	SetTitleText(FText::GetEmpty());
	SetTitleTexture(nullptr);
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

void UScreenFadeSubsystem::StartTitleFade(float InTargetAlpha, float DurationSeconds, float DelaySeconds)
{
	const float ClampedTarget = FMath::Clamp(InTargetAlpha, 0.0f, 1.0f);
	if (bTitleFadeActive && FMath::IsNearlyEqual(TargetTitleAlpha, ClampedTarget))
	{
		return;
	}

	if (!bTitleFadeActive && FMath::IsNearlyEqual(CurrentTitleAlpha, ClampedTarget))
	{
		return;
	}

	StartTitleAlpha = CurrentTitleAlpha;
	TargetTitleAlpha = ClampedTarget;
	TitleFadeDuration = FMath::Max(DurationSeconds, 0.0f);
	TitleRemainingDelaySeconds = FMath::Max(DelaySeconds, 0.0f);
	TitleElapsedSeconds = 0.0f;
	bTitleFadeActive = true;
	ApplyTitleAlpha(CurrentTitleAlpha);
}

void UScreenFadeSubsystem::ApplyFadeAlpha(float InFadeAlpha)
{
	CurrentAlpha = FMath::Clamp(InFadeAlpha, 0.0f, 1.0f);

	if (UScreenFadeWidget* Widget = EnsureFadeWidget())
	{
		Widget->SetFadeAlpha(CurrentAlpha);
	}
}

void UScreenFadeSubsystem::ApplyTitleAlpha(float InTitleAlpha)
{
	CurrentTitleAlpha = FMath::Clamp(InTitleAlpha, 0.0f, 1.0f);

	if (UScreenFadeWidget* Widget = EnsureFadeWidget())
	{
		Widget->SetTitleAlpha(CurrentTitleAlpha);
	}
}

UScreenFadeWidget* UScreenFadeSubsystem::EnsureFadeWidget()
{
	if (FadeWidget)
	{
		return FadeWidget;
	}

	// 알파가 0이면 굳이 위젯을 만들지 않습니다.
	if (CurrentAlpha <= 0.0f && TargetAlpha <= 0.0f && CurrentTitleAlpha <= 0.0f && TargetTitleAlpha <= 0.0f)
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
	FadeWidget->SetTitleText(TitleText);
	FadeWidget->SetTitleTexture(TitleTexture);
	FadeWidget->SetTitleAlpha(CurrentTitleAlpha);
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
