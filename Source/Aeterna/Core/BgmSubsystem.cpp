// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/BgmSubsystem.h"

#include "Aeterna.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UBgmSubsystem::Deinitialize()
{
	StopChannelImmediately(BgmComponent, CurrentBgm);
	StopChannelImmediately(AmbienceComponent, CurrentAmbience);

	Super::Deinitialize();
}

bool UBgmSubsystem::IsPlayingBgm() const
{
	return BgmComponent != nullptr && BgmComponent->IsPlaying();
}

bool UBgmSubsystem::IsPlayingAmbience() const
{
	return AmbienceComponent != nullptr && AmbienceComponent->IsPlaying();
}

void UBgmSubsystem::PlayBgm(USoundBase* Bgm, float Volume, float FadeInSeconds, float CrossFadeOutSeconds)
{
	PlayOnChannel(BgmComponent, CurrentBgm, Bgm, Volume, FadeInSeconds, CrossFadeOutSeconds, TEXT("BGM"));
}

void UBgmSubsystem::StopBgm(float FadeOutSeconds)
{
	StopChannel(BgmComponent, CurrentBgm, FadeOutSeconds);
}

void UBgmSubsystem::PlayAmbience(USoundBase* Ambience, float Volume, float FadeInSeconds, float CrossFadeOutSeconds)
{
	PlayOnChannel(AmbienceComponent, CurrentAmbience, Ambience, Volume, FadeInSeconds, CrossFadeOutSeconds, TEXT("Ambience"));
}

void UBgmSubsystem::StopAmbience(float FadeOutSeconds)
{
	StopChannel(AmbienceComponent, CurrentAmbience, FadeOutSeconds);
}

void UBgmSubsystem::PlayOnChannel(TObjectPtr<UAudioComponent>& Component, TObjectPtr<USoundBase>& Current, USoundBase* Sound, float Volume, float FadeInSeconds, float CrossFadeOutSeconds, const TCHAR* ChannelTag)
{
	if (!Sound)
	{
		StopChannel(Component, Current, CrossFadeOutSeconds);
		return;
	}

	// 같은 음원이면 손대지 않습니다. 밤을 재시작해도 소리가 이어지는 지점입니다.
	if (Current == Sound && Component != nullptr && Component->IsPlaying())
	{
		return;
	}

	if (!Sound->IsLooping())
	{
		UE_LOG(LogAeterna, Warning,
			TEXT("[%s] %s 에 Looping이 꺼져 있어 한 번만 재생됩니다. 에셋을 열어 Looping을 켜십시오."),
			ChannelTag, *Sound->GetName());
	}

	// 이전 음원은 페이드아웃에 맡기고 소유권을 놓습니다. 다 사라지면 스스로 정리됩니다.
	if (Component)
	{
		Component->bAutoDestroy = true;
		Component->FadeOut(FMath::Max(0.0f, CrossFadeOutSeconds), 0.0f);
		Component = nullptr;
	}

	UAudioComponent* NewComponent = UGameplayStatics::CreateSound2D(this, Sound, FMath::Max(0.0f, Volume), 1.0f, 0.0f, nullptr, false, false);
	if (!NewComponent)
	{
		UE_LOG(LogAeterna, Warning, TEXT("[%s] %s 재생용 오디오 컴포넌트를 만들지 못했습니다."), ChannelTag, *Sound->GetName());
		Current = nullptr;
		return;
	}

	NewComponent->bAutoDestroy = false;

	if (FadeInSeconds > 0.0f)
	{
		NewComponent->FadeIn(FadeInSeconds);
	}
	else
	{
		NewComponent->Play();
	}

	Component = NewComponent;
	Current = Sound;
}

void UBgmSubsystem::StopChannel(TObjectPtr<UAudioComponent>& Component, TObjectPtr<USoundBase>& Current, float FadeOutSeconds)
{
	if (Component)
	{
		Component->bAutoDestroy = true;
		Component->FadeOut(FMath::Max(0.0f, FadeOutSeconds), 0.0f);
		Component = nullptr;
	}

	Current = nullptr;
}

void UBgmSubsystem::StopChannelImmediately(TObjectPtr<UAudioComponent>& Component, TObjectPtr<USoundBase>& Current)
{
	if (Component)
	{
		Component->Stop();
		Component->DestroyComponent();
		Component = nullptr;
	}

	Current = nullptr;
}
