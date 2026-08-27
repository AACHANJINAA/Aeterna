// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaClockComponent.h"

#include "Aeterna.h"
#include "Core/GameClockSubsystem.h"
#include "Player/AeternaCharacter.h"
#include "Player/UI/AeternaHudLayoutUtils.h"
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

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		CurrentClockMinutes = GameClockSubsystem->GetClockMinutes();
		GameClockSubsystem->OnClockMinuteChanged.AddUniqueDynamic(this, &UAeternaClockComponent::HandleGameClockMinuteChanged);
		ApplyClockHud();
	}
}

void UAeternaClockComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->OnClockMinuteChanged.RemoveDynamic(this, &UAeternaClockComponent::HandleGameClockMinuteChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UAeternaClockComponent::SetCountdownSeconds(float RemainingSeconds)
{
	if (UAeternaClockHudWidget* ClockWidget = Cast<UAeternaClockHudWidget>(ClockHudWidget))
	{
		ClockWidget->SetCountdownSeconds(RemainingSeconds);
	}
}

void UAeternaClockComponent::ClearCountdown()
{
	if (UAeternaClockHudWidget* ClockWidget = Cast<UAeternaClockHudWidget>(ClockHudWidget))
	{
		ClockWidget->ClearCountdown();
	}
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

	if (UGameClockSubsystem* GameClockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		GameClockSubsystem->AdvanceClockMinutes(MinutesToAdvance);
		return;
	}

	CurrentClockMinutes = FMath::Clamp(CurrentClockMinutes + MinutesToAdvance, 0, MaxClockMinutes);
	if (!ApplyClockHud())
	{
		SetComponentTickEnabled(true);
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
		ClockHudWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
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

	const FVector2D ViewportSize(static_cast<float>(ViewportSizeX), static_cast<float>(ViewportSizeY));
	const float ViewportScale = AeternaHudLayout::GetViewportScale(ViewportSize, ReferenceViewportSize);
	const FVector2D WidgetPosition = AeternaHudLayout::GetTopCenterPosition(ViewportSize, TopCenterOffset, HudWidgetSize, ViewportScale);
	AeternaHudLayout::ApplyScaledViewportLayout(ClockHudWidget, WidgetPosition, HudWidgetSize, ViewportScale);
	return true;
}

void UAeternaClockComponent::HandleGameClockMinuteChanged(int32 InClockMinutes)
{
	CurrentClockMinutes = InClockMinutes;
	ApplyClockHud();
}
