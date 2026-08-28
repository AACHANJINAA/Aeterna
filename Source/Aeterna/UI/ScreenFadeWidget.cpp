// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ScreenFadeWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
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
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
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
	UpdateTitleImageSize();
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
		UpdateTitleImageSize();
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
		UpdateTitleImageSize();
		ApplyTitleTexture();
	}
}

void UScreenFadeWidget::SetTitleTextureFillScreen(bool bInFillScreen)
{
	bTitleTextureFillsScreen = bInFillScreen;
	if (TitleTexture)
	{
		UpdateTitleImageSize();
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

void UScreenFadeWidget::UpdateTitleImageSize()
{
	if (!TitleTexture)
	{
		TitleImageBrush.ImageSize = FVector2D::ZeroVector;
		return;
	}

	const FVector2D NativeSize(TitleTexture->GetSizeX(), TitleTexture->GetSizeY());
	if (NativeSize.X <= 0.0f || NativeSize.Y <= 0.0f)
	{
		TitleImageBrush.ImageSize = FVector2D::ZeroVector;
		return;
	}

	// 위젯이 아직 뷰포트에 없으면 1920x1080을 기준으로 잡고, NativeConstruct에서 다시 계산합니다.
	FVector2D AvailableSize(1920.0f, 1080.0f);
	if (GetWorld())
	{
		const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
		const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
		if (ViewportSize.X > 0.0f && ViewportSize.Y > 0.0f && ViewportScale > 0.0f)
		{
			AvailableSize = ViewportSize / ViewportScale;
		}
	}

	// Day 카드는 화면을 꽉 채웁니다. 비율은 깨지지만 여백 없이 한 장으로 덮는 연출입니다.
	if (bTitleTextureFillsScreen)
	{
		TitleImageBrush.ImageSize = AvailableSize;
		return;
	}

	// 가로·세로 중 더 빡빡한 쪽에 맞춰 줄입니다. 비율이 유지되므로 남는 쪽은 검게 비어둡니다.
	const float FitScale = FMath::Min(AvailableSize.X / NativeSize.X, AvailableSize.Y / NativeSize.Y);
	const float FinalScale = FitScale * FMath::Clamp(TitleViewportCoverage, 0.05f, 1.0f) * TitleTextureScale;
	TitleImageBrush.ImageSize = NativeSize * FinalScale;
}

void UScreenFadeWidget::ApplyTitleAlpha()
{
	ApplyFadeColor();
}
