// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ScreenFadeWidget.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UScreenFadeWidget::RebuildWidget()
{
	FLinearColor CurrentColor = FadeColor;
	CurrentColor.A = FadeAlpha;

	TSharedRef<SOverlay> RootOverlay = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(FadeBorder, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FSlateColor(CurrentColor))
		]
		+ SOverlay::Slot()
		[
			SAssignNew(TitleImage, SImage)
			.Image(&TitleImageBrush)
			.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, TitleAlpha * FadeAlpha))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(TitleTextBlock, STextBlock)
			.Text(TitleText)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FSlateColor(TitleColor))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", TitleFontSize))
		];

	return RootOverlay;
}

void UScreenFadeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 페이드 중에도 아래 UI가 입력을 받을 수 있게 히트 테스트에서 제외합니다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
	ApplyFadeColor();
	ApplyTitleTexture();
	ApplyTitleText();
	ApplyTitleAlpha();
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

void UScreenFadeWidget::SetTitleText(const FText& InTitleText)
{
	TitleText = InTitleText;
	ApplyTitleText();
}

void UScreenFadeWidget::SetTitleTexture(UTexture2D* InTitleTexture)
{
	TitleTexture = InTitleTexture;

	if (TitleTexture)
	{
		TitleImageBrush.DrawAs = ESlateBrushDrawType::Image;
		TitleImageBrush.SetResourceObject(TitleTexture);
		TitleImageBrush.ImageSize = FVector2D(TitleTexture->GetSizeX(), TitleTexture->GetSizeY()) * TitleTextureScale;
	}
	else
	{
		TitleImageBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
		TitleImageBrush.SetResourceObject(nullptr);
		TitleImageBrush.ImageSize = FVector2D::ZeroVector;
	}

	ApplyTitleTexture();
}

void UScreenFadeWidget::SetTitleTextureScale(float InTitleTextureScale)
{
	TitleTextureScale = FMath::Max(0.01f, InTitleTextureScale);
	if (TitleTexture)
	{
		TitleImageBrush.ImageSize = FVector2D(TitleTexture->GetSizeX(), TitleTexture->GetSizeY()) * TitleTextureScale;
		ApplyTitleTexture();
	}
}

void UScreenFadeWidget::SetTitleAlpha(float InTitleAlpha)
{
	TitleAlpha = FMath::Clamp(InTitleAlpha, 0.0f, 1.0f);
	ApplyTitleAlpha();
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

	if (TitleTextBlock.IsValid())
	{
		FLinearColor CurrentTitleColor = TitleColor;
		CurrentTitleColor.A *= TitleAlpha * FadeAlpha;
		TitleTextBlock->SetColorAndOpacity(FSlateColor(CurrentTitleColor));
	}

	if (TitleImage.IsValid())
	{
		TitleImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, TitleAlpha * FadeAlpha));
	}
}

void UScreenFadeWidget::ApplyTitleText()
{
	if (!TitleTextBlock.IsValid())
	{
		return;
	}

	TitleTextBlock->SetText(TitleText);
}

void UScreenFadeWidget::ApplyTitleTexture()
{
	if (!TitleImage.IsValid())
	{
		return;
	}

	TitleImage->SetImage(&TitleImageBrush);
}

void UScreenFadeWidget::ApplyTitleAlpha()
{
	ApplyFadeColor();
}
