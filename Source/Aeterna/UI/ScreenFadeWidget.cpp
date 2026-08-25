// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ScreenFadeWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

TSharedRef<SWidget> UScreenFadeWidget::RebuildWidget()
{
	FLinearColor CurrentColor = FadeColor;
	CurrentColor.A = FadeAlpha;

	SAssignNew(FadeBorder, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FSlateColor(CurrentColor));

	return FadeBorder.ToSharedRef();
}

void UScreenFadeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 페이드 중에도 아래 UI가 입력을 받을 수 있게 히트 테스트에서 제외합니다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
	ApplyFadeColor();
}

void UScreenFadeWidget::SetFadeColor(const FLinearColor& InFadeColor)
{
	FadeColor = InFadeColor;
	ApplyFadeColor();
}

void UScreenFadeWidget::SetFadeAlpha(float InFadeAlpha)
{
	FadeAlpha = FMath::Clamp(InFadeAlpha, 0.0f, 1.0f);
	ApplyFadeColor();
}

void UScreenFadeWidget::ApplyFadeColor()
{
	if (!FadeBorder.IsValid())
	{
		return;
	}

	FLinearColor CurrentColor = FadeColor;
	CurrentColor.A = FadeAlpha;
	FadeBorder->SetBorderBackgroundColor(FSlateColor(CurrentColor));
}
