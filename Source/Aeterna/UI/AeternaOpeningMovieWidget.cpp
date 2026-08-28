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

void UAeternaOpeningMovieWidget::SetOpeningMovieSource(
	UMediaSource* InMediaSource,
	const FString& InVideoContentPath,
	float InSkipHoldSeconds,
	float InMovieVolumeMultiplier,
	float InMovieFadeInSeconds,
	float InMovieFadeOutSeconds)
{
	OpeningMediaSource = InMediaSource;
	OpeningVideoContentPath = InVideoContentPath;
	SkipHoldSeconds = FMath::Max(0.1f, InSkipHoldSeconds);
	MovieVolumeMultiplier = FMath::Max(0.0f, InMovieVolumeMultiplier);
	MovieFadeInSeconds = FMath::Max(0.0f, InMovieFadeInSeconds);
	MovieFadeOutSeconds = FMath::Max(0.0f, InMovieFadeOutSeconds);
}

void UAeternaOpeningMovieWidget::PlayOpeningMovie()
{
	bMovieFinishing = false;
	bMovieFadeOutActive = false;
	bMovieFadeInActive = MovieFadeInSeconds > 0.0f;
	MovieFadeElapsedSeconds = 0.0f;
	SetVideoAlpha(bMovieFadeInActive ? 0.0f : 1.0f);
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
	// Route movie audio through a MediaSoundComponent, matching UE's CommonUI
	// video player path. Marking it as UI sound keeps it audible during the
	// input-locked opening/ending sequence.
	MediaPlayer->NativeAudioOut = false;
	MediaPlayer->SetNativeVolume(MovieVolumeMultiplier);
	MediaPlayer->PlayOnOpen = false;
	MediaPlayer->OnMediaOpened.AddUniqueDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaOpened);
	MediaPlayer->OnMediaOpenFailed.AddUniqueDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaOpenFailed);
	MediaPlayer->OnEndReached.AddUniqueDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaEndReached);

	MediaSoundComponent = NewObject<UMediaSoundComponent>(this);
	if (MediaSoundComponent)
	{
		MediaSoundComponent->Channels = EMediaSoundChannels::Stereo;
		MediaSoundComponent->bIsUISound = true;
		MediaSoundComponent->SetMediaPlayer(MediaPlayer);
		MediaSoundComponent->SetVolumeMultiplier(MovieVolumeMultiplier);
		MediaSoundComponent->RegisterComponentWithWorld(GetWorld());
		MediaSoundComponent->Initialize();
		MediaSoundComponent->UpdatePlayer();
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
	if (!OpeningVideoContentPath.IsEmpty())
	{
		const FString AbsoluteVideoPath = BuildContentVideoPath();
		if (!FPaths::FileExists(AbsoluteVideoPath))
		{
			UE_LOG(LogAeterna, Warning, TEXT("[Opening] 영상 파일이 없습니다: %s"), *AbsoluteVideoPath);
		}
		else
		{
			UE_LOG(LogAeterna, Log, TEXT("[Opening] 영상 파일을 엽니다: %s"), *AbsoluteVideoPath);
			bOpened = MediaPlayer->OpenFile(AbsoluteVideoPath);
		}
	}

	if (!bOpened && OpeningMediaSource)
	{
		bOpened = MediaPlayer->OpenSource(OpeningMediaSource);
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
	bMovieFadeInActive = false;

	if (MovieFadeOutSeconds > 0.0f)
	{
		bMovieFadeOutActive = true;
		MovieFadeElapsedSeconds = 0.0f;
		return;
	}

	CompleteOpeningMovie();
}

void UAeternaOpeningMovieWidget::CompleteOpeningMovie()
{
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
					.Text(NSLOCTEXT("Aeterna", "OpeningHoldQToSkip", "Hold Q to skip"))
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
		if (bMovieFadeOutActive)
		{
			MovieFadeElapsedSeconds += InDeltaTime;
			const float Progress = FMath::Clamp(MovieFadeElapsedSeconds / MovieFadeOutSeconds, 0.0f, 1.0f);
			SetVideoAlpha(FMath::Lerp(1.0f, 0.0f, Progress));

			if (Progress >= 1.0f)
			{
				bMovieFadeOutActive = false;
				CompleteOpeningMovie();
			}
		}
		return;
	}

	if (bMovieFadeInActive)
	{
		MovieFadeElapsedSeconds += InDeltaTime;
		const float Progress = FMath::Clamp(MovieFadeElapsedSeconds / MovieFadeInSeconds, 0.0f, 1.0f);
		SetVideoAlpha(Progress);

		if (Progress >= 1.0f)
		{
			bMovieFadeInActive = false;
		}
	}

	APlayerController* PlayerController = GetOwningPlayer();
	const bool bHoldingSkip = PlayerController && PlayerController->IsInputKeyDown(EKeys::Q);
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

void UAeternaOpeningMovieWidget::HandleMediaOpened(FString OpenedUrl)
{
	UE_LOG(LogAeterna, Log, TEXT("[Opening] 영상 열기 성공: %s"), *OpenedUrl);

	if (MediaPlayer)
	{
		if (MediaSoundComponent)
		{
			MediaSoundComponent->SetMediaPlayer(MediaPlayer);
			MediaSoundComponent->SetVolumeMultiplier(MovieVolumeMultiplier);
			MediaSoundComponent->UpdatePlayer();
			MediaSoundComponent->Activate(true);
		}

		MediaPlayer->Rewind();
		MediaPlayer->Play();
	}
}

void UAeternaOpeningMovieWidget::HandleMediaOpenFailed(FString FailedUrl)
{
	UE_LOG(LogAeterna, Warning, TEXT("[Opening] 영상 열기 실패: %s"), *FailedUrl);
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
	if (MediaSoundComponent)
	{
		MediaSoundComponent->SetMediaPlayer(nullptr);
		MediaSoundComponent->Deactivate();
		MediaSoundComponent->UnregisterComponent();
		MediaSoundComponent = nullptr;
	}

	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.RemoveDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaOpened);
		MediaPlayer->OnMediaOpenFailed.RemoveDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaOpenFailed);
		MediaPlayer->OnEndReached.RemoveDynamic(this, &UAeternaOpeningMovieWidget::HandleMediaEndReached);
		MediaPlayer->Close();
		MediaPlayer = nullptr;
	}

	MediaTexture = nullptr;
}

void UAeternaOpeningMovieWidget::RefreshVideoBrush()
{
	VideoBrush = FSlateBrush();
	VideoBrush.SetResourceObject(MediaTexture);
	VideoBrush.DrawAs = ESlateBrushDrawType::Image;
	VideoBrush.Tiling = ESlateBrushTileType::NoTile;
	VideoBrush.Mirroring = ESlateBrushMirrorType::NoMirror;
	VideoBrush.ImageSize = FVector2D(1920.0f, 1080.0f);
	VideoBrush.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, VideoAlpha));
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

void UAeternaOpeningMovieWidget::SetVideoAlpha(float InAlpha)
{
	VideoAlpha = FMath::Clamp(InAlpha, 0.0f, 1.0f);
	VideoBrush.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, VideoAlpha));

	if (VideoImage.IsValid())
	{
		VideoImage->SetImage(&VideoBrush);
	}
}
