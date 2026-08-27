// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/AeternaOpeningMovieWidget.h"

#include "Aeterna.h"
#include "Core/AeternaOpeningSequenceActor.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void UAeternaOpeningMovieWidget::SetOpeningSequenceActor(AAeternaOpeningSequenceActor* InOpeningSequenceActor)
{
	OpeningSequenceActor = InOpeningSequenceActor;
}

void UAeternaOpeningMovieWidget::SetOpeningMovieSource(UMediaSource* InMediaSource, const FString& InVideoContentPath, float InSkipHoldSeconds, float InMovieVolumeMultiplier)
{
	OpeningMediaSource = InMediaSource;
	OpeningVideoContentPath = InVideoContentPath;
	SkipHoldSeconds = FMath::Max(0.1f, InSkipHoldSeconds);
	MovieVolumeMultiplier = FMath::Max(0.0f, InMovieVolumeMultiplier);
}

void UAeternaOpeningMovieWidget::PlayOpeningMovie()
{
	bMovieFinishing = false;
	CurrentSkipHoldSeconds = 0.0f;
	RefreshSkipPrompt();

	if (!OpeningMediaSource && OpeningVideoContentPath.IsEmpty())
	{
		UE_LOG(LogAeterna, Warning, TEXT("[Opening] OpeningMediaSource와 OpeningVideoContentPath가 모두 비어 있습니다."));
		FinishOpeningMovie();
		return;
	}

	MediaPlayer = NewObject<UMediaPlayer>(this);
	MediaTexture = NewObject<UMediaTexture>(this);
	if (!MediaPlayer || !MediaTexture)
	{
		FinishOpeningMovie();
		return;
	}

	MediaPlayer->SetLooping(false);
	MediaPlayer->NativeAudioOut = false;
	MediaPlayer->SetNativeVolume(MovieVolumeMultiplier);
	MediaPlayer->PlayOnOpen = true;
	MediaPlayer->OnEndReached.AddUniqueDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaEndReached);

	MediaSoundComponent = NewObject<UMediaSoundComponent>(this);
	if (MediaSoundComponent)
	{
		MediaSoundComponent->SetMediaPlayer(MediaPlayer);
		MediaSoundComponent->SetVolumeMultiplier(MovieVolumeMultiplier);
		MediaSoundComponent->RegisterComponentWithWorld(GetWorld());
		MediaSoundComponent->Activate(true);
	}

	MediaTexture->SetMediaPlayer(MediaPlayer);
	MediaTexture->UpdateResource();
	RefreshVideoBrush();

	if (VideoImage.IsValid())
	{
		VideoImage->SetImage(&VideoBrush);
	}

	bool bOpened = false;
	if (OpeningMediaSource)
	{
		bOpened = MediaPlayer->OpenSource(OpeningMediaSource);
	}
	else
	{
		const FString AbsoluteVideoPath = BuildContentVideoPath();
		bOpened = FPaths::FileExists(AbsoluteVideoPath) && MediaPlayer->OpenFile(AbsoluteVideoPath);
	}

	if (!bOpened)
	{
		UE_LOG(LogAeterna, Warning, TEXT("[Opening] 오프닝 영상을 열지 못했습니다."));
		FinishOpeningMovie();
	}
}

void UAeternaOpeningMovieWidget::FinishOpeningMovie()
{
	if (bMovieFinishing)
	{
		return;
	}

	bMovieFinishing = true;
	StopMovie();
	OnMovieFinished.Broadcast();

	if (AAeternaOpeningSequenceActor* Owner = OpeningSequenceActor.Get())
	{
		Owner->FinishOpeningSequence();
	}
}

TSharedRef<SWidget> UAeternaOpeningMovieWidget::RebuildWidget()
{
	RefreshVideoBrush();

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
		]
		+ SOverlay::Slot()
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SAssignNew(VideoImage, SImage)
				.Image(&VideoBrush)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 72.0f, 64.0f))
		[
			SNew(SBox)
			.WidthOverride(260.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
				[
					SAssignNew(SkipTextBlock, STextBlock)
					.Text(NSLOCTEXT("Aeterna", "OpeningHoldEscToSkip", "Hold ESC to skip"))
					.Justification(ETextJustify::Right)
					.ColorAndOpacity(FLinearColor(0.92f, 0.98f, 1.0f, 0.72f))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(SkipProgressBar, SProgressBar)
					.Percent(0.0f)
					.FillColorAndOpacity(FLinearColor(0.55f, 1.0f, 0.9f, 0.95f))
				]
			]
		];
}

void UAeternaOpeningMovieWidget::NativeDestruct()
{
	StopMovie();
	Super::NativeDestruct();
}

void UAeternaOpeningMovieWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!MediaPlayer || bMovieFinishing)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	const bool bHoldingSkip = PlayerController && PlayerController->IsInputKeyDown(EKeys::Escape);
	CurrentSkipHoldSeconds = bHoldingSkip ? CurrentSkipHoldSeconds + InDeltaTime : 0.0f;
	RefreshSkipPrompt();

	if (CurrentSkipHoldSeconds >= SkipHoldSeconds)
	{
		FinishOpeningMovie();
	}
}

void UAeternaOpeningMovieWidget::HandleMediaEndReached()
{
	FinishOpeningMovie();
}

FString UAeternaOpeningMovieWidget::BuildContentVideoPath() const
{
	FString NormalizedPath = OpeningVideoContentPath;
	NormalizedPath.ReplaceInline(TEXT("\\"), TEXT("/"));
	NormalizedPath.RemoveFromStart(TEXT("/"));
	NormalizedPath.RemoveFromStart(TEXT("Content/"));

	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / NormalizedPath);
}

void UAeternaOpeningMovieWidget::StopMovie()
{
	if (MediaPlayer)
	{
		MediaPlayer->OnEndReached.RemoveDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaEndReached);
		MediaPlayer->Close();
		MediaPlayer = nullptr;
	}

	MediaTexture = nullptr;

	if (MediaSoundComponent)
	{
		MediaSoundComponent->SetMediaPlayer(nullptr);
		MediaSoundComponent->Deactivate();
		MediaSoundComponent->UnregisterComponent();
		MediaSoundComponent = nullptr;
	}
}

void UAeternaOpeningMovieWidget::RefreshVideoBrush()
{
	VideoBrush = FSlateBrush();
	VideoBrush.SetResourceObject(MediaTexture);
	VideoBrush.DrawAs = ESlateBrushDrawType::Image;
	VideoBrush.Tiling = ESlateBrushTileType::NoTile;
	VideoBrush.Mirroring = ESlateBrushMirrorType::NoMirror;
	VideoBrush.ImageSize = FVector2D(1920.0f, 1080.0f);
	VideoBrush.TintColor = FSlateColor(FLinearColor::White);
}

void UAeternaOpeningMovieWidget::RefreshSkipPrompt()
{
	const float SkipProgress = SkipHoldSeconds > 0.0f ? FMath::Clamp(CurrentSkipHoldSeconds / SkipHoldSeconds, 0.0f, 1.0f) : 1.0f;

	if (SkipProgressBar.IsValid())
	{
		SkipProgressBar->SetPercent(SkipProgress);
	}

	if (SkipTextBlock.IsValid())
	{
		const float Alpha = SkipProgress > 0.0f ? 1.0f : 0.72f;
		SkipTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.98f, 1.0f, Alpha)));
	}
}
