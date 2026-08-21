// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaClockComponent.h"

#include "Aeterna.h"
#include "Player/AeternaCharacter.h"
#include "Player/UI/AeternaClockHudWidget.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

UAeternaClockComponent::UAeternaClockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAeternaClockComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentClockMinutes = FMath::Clamp(StartClockMinutes, 0, MaxClockMinutes);

	if (!GetAeternaCharacter())
	{
		if (AAeternaCharacter* OwnerCharacter = Cast<AAeternaCharacter>(GetOwner()))
		{
			InitializePlayerComponent(OwnerCharacter);
		}
	}

	CreateClockHudWidget();
	ApplyClockHud();
}

void UAeternaClockComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);
	CreateClockHudWidget();
}

void UAeternaClockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ClockHudWidget)
	{
		CreateClockHudWidget();
	}

	if (ClockHudWidget)
	{
		UpdateClockHudPosition();
	}
}

void UAeternaClockComponent::AdvanceClockMinutes(int32 MinutesToAdvance)
{
	if (MinutesToAdvance <= 0)
	{
		return;
	}

	const int32 PreviousClockMinutes = CurrentClockMinutes;
	CurrentClockMinutes = FMath::Clamp(CurrentClockMinutes + MinutesToAdvance, 0, MaxClockMinutes);
	if (!ApplyClockHud())
	{
		SetComponentTickEnabled(true);
	}

	if (PreviousClockMinutes != CurrentClockMinutes)
	{
		UE_LOG(LogAeterna, Log, TEXT("Clock advanced: %02d:%02d"),
			CurrentClockMinutes / 60,
			CurrentClockMinutes % 60);
	}
}

void UAeternaClockComponent::AdvanceDebugClockStep()
{
	AdvanceClockMinutes(DebugAdvanceMinutes);
}

void UAeternaClockComponent::CreateClockHudWidget()
{
	if (ClockHudWidget)
	{
		return;
	}

	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	APlayerController* PlayerController = AeternaCharacter ? Cast<APlayerController>(AeternaCharacter->GetController()) : nullptr;
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	if (!PlayerController)
	{
		SetComponentTickEnabled(true);
		return;
	}

	ClockHudWidget = CreateWidget<UAeternaClockHudWidget>(PlayerController, UAeternaClockHudWidget::StaticClass());
	if (ClockHudWidget)
	{
		ClockHudWidget->AddToViewport(ViewportZOrder);
		ClockHudWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.0f));
		ClockHudWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ClockHudWidget->ForceLayoutPrepass();
		UpdateClockHudPosition();
	}
}

bool UAeternaClockComponent::ApplyClockHud()
{
	if (UAeternaClockHudWidget* NativeClockHudWidget = Cast<UAeternaClockHudWidget>(ClockHudWidget))
	{
		NativeClockHudWidget->SetClockMinutes(CurrentClockMinutes);
		return UpdateClockHudPosition();
	}

	return false;
}

bool UAeternaClockComponent::UpdateClockHudPosition()
{
	if (!ClockHudWidget)
	{
		return false;
	}

	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	APlayerController* PlayerController = AeternaCharacter ? Cast<APlayerController>(AeternaCharacter->GetController()) : nullptr;
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	if (!PlayerController)
	{
		return false;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return false;
	}

	ClockHudWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.0f));
	ClockHudWidget->SetPositionInViewport(
		FVector2D((static_cast<float>(ViewportSizeX) * 0.5f) + TopCenterOffset.X, TopCenterOffset.Y),
		true);
	return true;
}
