// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/BgmSubsystem.h"

#include "Aeterna.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UBgmSubsystem::Deinitialize()
{
	if (BgmComponent)
	{
		BgmComponent->Stop();
		BgmComponent->DestroyComponent();
		BgmComponent = nullptr;
	}

	CurrentBgm = nullptr;

	Super::Deinitialize();
}

bool UBgmSubsystem::IsPlayingBgm() const
{
	return BgmComponent != nullptr && BgmComponent->IsPlaying();
}

void UBgmSubsystem::PlayBgm(USoundBase* Bgm, float FadeInSeconds, float CrossFadeOutSeconds)
{
	if (!Bgm)
	{
		StopBgm(CrossFadeOutSeconds);
		return;
	}

	// 같은 곡이면 손대지 않습니다. 밤을 재시작해도 음악이 이어지는 지점입니다.
	if (CurrentBgm == Bgm && IsPlayingBgm())
	{
		return;
	}

	if (!Bgm->IsLooping())
	{
		UE_LOG(LogAeterna, Warning,
			TEXT("[BGM] %s 에 Looping이 꺼져 있어 한 번만 재생됩니다. 에셋을 열어 Looping을 켜십시오."),
			*Bgm->GetName());
	}

	// 이전 곡은 페이드아웃에 맡기고 소유권을 놓습니다. 다 사라지면 스스로 정리됩니다.
	if (BgmComponent)
	{
		BgmComponent->bAutoDestroy = true;
		BgmComponent->FadeOut(FMath::Max(0.0f, CrossFadeOutSeconds), 0.0f);
		BgmComponent = nullptr;
	}

	UAudioComponent* NewBgmComponent = UGameplayStatics::CreateSound2D(this, Bgm, 1.0f, 1.0f, 0.0f, nullptr, false, false);
	if (!NewBgmComponent)
	{
		UE_LOG(LogAeterna, Warning, TEXT("[BGM] %s 재생용 오디오 컴포넌트를 만들지 못했습니다."), *Bgm->GetName());
		CurrentBgm = nullptr;
		return;
	}

	NewBgmComponent->bAutoDestroy = false;

	if (FadeInSeconds > 0.0f)
	{
		NewBgmComponent->FadeIn(FadeInSeconds);
	}
	else
	{
		NewBgmComponent->Play();
	}

	BgmComponent = NewBgmComponent;
	CurrentBgm = Bgm;
}

void UBgmSubsystem::StopBgm(float FadeOutSeconds)
{
	if (BgmComponent)
	{
		BgmComponent->bAutoDestroy = true;
		BgmComponent->FadeOut(FMath::Max(0.0f, FadeOutSeconds), 0.0f);
		BgmComponent = nullptr;
	}

	CurrentBgm = nullptr;
}
