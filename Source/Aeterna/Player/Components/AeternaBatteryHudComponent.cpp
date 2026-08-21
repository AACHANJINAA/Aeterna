// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaBatteryHudComponent.h"

#include "Player/AeternaCharacter.h"
#include "Player/UI/AeternaBatteryHudWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/PlayerController.h"

UAeternaBatteryHudComponent::UAeternaBatteryHudComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAeternaBatteryHudComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetAeternaCharacter())
	{
		if (AAeternaCharacter* OwnerCharacter = Cast<AAeternaCharacter>(GetOwner()))
		{
			InitializePlayerComponent(OwnerCharacter);
		}
	}

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		UpdateBatteryHud(AeternaCharacter->GetCurrentBattery(), 100.0f, AeternaCharacter->GetBatteryNormalized());
	}
	else
	{
		SetComponentTickEnabled(true);
	}
}

void UAeternaBatteryHudComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);
	CreateBatteryHudWidget();
}

void UAeternaBatteryHudComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!BatteryHudWidget)
	{
		CreateBatteryHudWidget();
	}

	if (BatteryHudWidget)
	{
		ApplyBatteryHud();
		SetComponentTickEnabled(false);
	}
}

void UAeternaBatteryHudComponent::UpdateBatteryHud(float Current, float Max, float Normalized)
{
	LastCurrentBattery = Current;
	LastMaxBattery = Max;
	LastBatteryNormalized = Normalized;

	CreateBatteryHudWidget();
	if (!BatteryHudWidget)
	{
		SetComponentTickEnabled(true);
		return;
	}

	ApplyBatteryHud();
}

void UAeternaBatteryHudComponent::ApplyBatteryHud()
{
	if (UAeternaBatteryHudWidget* NativeBatteryHudWidget = Cast<UAeternaBatteryHudWidget>(BatteryHudWidget))
	{
		const float BatteryPercent = FMath::Clamp(LastBatteryNormalized, 0.0f, 1.0f);
		NativeBatteryHudWidget->SetBatteryLabel(BatteryLabelText);
		NativeBatteryHudWidget->SetBattery(LastCurrentBattery, LastMaxBattery, BatteryPercent);
		UpdateBatteryHudPosition();
	}
}

void UAeternaBatteryHudComponent::CreateBatteryHudWidget()
{
	if (BatteryHudWidget)
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
		return;
	}

	BatteryHudWidget = CreateWidget<UAeternaBatteryHudWidget>(PlayerController, UAeternaBatteryHudWidget::StaticClass());
	if (BatteryHudWidget)
	{
		BatteryHudWidget->AddToViewport(ViewportZOrder);
		BatteryHudWidget->SetAnchorsInViewport(FAnchors(ViewportAnchorNormalized.X, ViewportAnchorNormalized.Y));
		BatteryHudWidget->SetAlignmentInViewport(WidgetAlignment);
		BatteryHudWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		BatteryHudWidget->ForceLayoutPrepass();
		UpdateBatteryHudPosition();
	}
}

void UAeternaBatteryHudComponent::UpdateBatteryHudPosition()
{
	if (!BatteryHudWidget)
	{
		return;
	}

	BatteryHudWidget->SetAnchorsInViewport(FAnchors(ViewportAnchorNormalized.X, ViewportAnchorNormalized.Y));
	BatteryHudWidget->SetAlignmentInViewport(WidgetAlignment);
	BatteryHudWidget->SetPositionInViewport(ViewportOffset, false);
}
