// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaRadarHudComponent.h"

#include "Blueprint/UserWidget.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/AeternaInteractableActor.h"
#include "Player/AeternaCharacter.h"
#include "Player/Components/AeternaScanProgressComponent.h"
#include "Player/UI/AeternaRadarHudWidget.h"

UAeternaRadarHudComponent::UAeternaRadarHudComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;

	RadarWidgetClass = UAeternaRadarHudWidget::StaticClass();
}

void UAeternaRadarHudComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetAeternaCharacter())
	{
		if (AAeternaCharacter* OwnerCharacter = Cast<AAeternaCharacter>(GetOwner()))
		{
			InitializePlayerComponent(OwnerCharacter);
		}
	}

	RefreshRadarTargets();
	CreateRadarWidget();
	UpdateRadarState();
}

void UAeternaRadarHudComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);
	CreateRadarWidget();
}

void UAeternaRadarHudComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SweepAngleDegrees = FMath::Fmod(SweepAngleDegrees + SweepSpeedDegreesPerSecond * DeltaTime, 360.0f);
	TargetRefreshAccumulatorSeconds += DeltaTime;
	if (TargetRefreshAccumulatorSeconds >= TargetRefreshIntervalSeconds)
	{
		TargetRefreshAccumulatorSeconds = 0.0f;
		RefreshRadarTargets();
	}

	if (!RadarHudWidget)
	{
		CreateRadarWidget();
	}

	UpdateRadarWidgetPosition();
	UpdateRadarState();
}

void UAeternaRadarHudComponent::RefreshRadarTargets()
{
	RadarTargets.Reset();

	UWorld* World = GetWorld();
	if (!World || QuestTargetTag.IsNone())
	{
		return;
	}

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (ShouldTrackActor(Actor))
		{
			RadarTargets.Add(Actor);
		}
	}
}

void UAeternaRadarHudComponent::CreateRadarWidget()
{
	if (RadarHudWidget || !RadarWidgetClass)
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

	RadarHudWidget = CreateWidget<UUserWidget>(PlayerController, RadarWidgetClass);
	if (RadarHudWidget)
	{
		RadarHudWidget->AddToViewport(ViewportZOrder);
		RadarHudWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
		RadarHudWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		UpdateRadarWidgetPosition();
	}
}

void UAeternaRadarHudComponent::UpdateRadarWidgetPosition()
{
	if (!RadarHudWidget)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return;
	}

	const FVector2D WidgetPosition(
		ViewportAnchorNormalized.X >= 0.5f
			? static_cast<float>(ViewportSizeX) - RadarWidgetSize.X - ViewportOffset.X
			: ViewportOffset.X,
		ViewportAnchorNormalized.Y >= 0.5f
			? static_cast<float>(ViewportSizeY) - RadarWidgetSize.Y - ViewportOffset.Y
			: ViewportOffset.Y);

	RadarHudWidget->SetDesiredSizeInViewport(RadarWidgetSize);
	RadarHudWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
	RadarHudWidget->SetPositionInViewport(WidgetPosition, true);
}

void UAeternaRadarHudComponent::UpdateRadarState()
{
	UAeternaRadarHudWidget* RadarWidget = Cast<UAeternaRadarHudWidget>(RadarHudWidget);
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!RadarWidget || !AeternaCharacter)
	{
		return;
	}

	if (RadarTargets.Num() == 0)
	{
		RefreshRadarTargets();
	}

	TArray<FAeternaRadarBlip> QuestBlips;
	const FVector PlayerLocation = AeternaCharacter->GetActorLocation();
	const FRotator PlayerRotation = AeternaCharacter->GetControlRotation();
	const float YawRadians = FMath::DegreesToRadians(PlayerRotation.Yaw);

	for (int32 Index = RadarTargets.Num() - 1; Index >= 0; --Index)
	{
		AActor* TargetActor = RadarTargets[Index].Get();
		if (!ShouldTrackActor(TargetActor))
		{
			if (!TargetActor)
			{
				RadarTargets.RemoveAtSwap(Index);
			}
			continue;
		}

		const FVector OffsetWorld = TargetActor->GetActorLocation() - PlayerLocation;
		const FVector2D Offset2D(OffsetWorld.X, OffsetWorld.Y);
		const float Distance = Offset2D.Size();
		if (Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const FVector2D WorldDirection = Offset2D / Distance;
		const float CosYaw = FMath::Cos(-YawRadians);
		const float SinYaw = FMath::Sin(-YawRadians);
		FVector2D LocalDirection(
			WorldDirection.X * CosYaw - WorldDirection.Y * SinYaw,
			WorldDirection.X * SinYaw + WorldDirection.Y * CosYaw);

		FAeternaRadarBlip Blip;
		const float NormalizedDistance = FMath::Clamp(Distance / RadarRange, 0.0f, 1.0f);
		Blip.bClampedToEdge = Distance > RadarRange;
		Blip.Position = FVector2D(LocalDirection.Y, -LocalDirection.X) * NormalizedDistance;
		QuestBlips.Add(Blip);
	}

	RadarWidget->SetRadarState(SweepAngleDegrees, QuestBlips);
}

bool UAeternaRadarHudComponent::ShouldTrackActor(AActor* Actor) const
{
	if (!Actor || Actor == GetOwner() || Actor->IsHidden())
	{
		return false;
	}

	if (!QuestTargetTag.IsNone() && Actor->ActorHasTag(QuestTargetTag))
	{
		return true;
	}

	if (!bTrackInteractableObjectives)
	{
		return bTrackCarryObjectives && IsCarryObjectiveActor(Actor);
	}

	const AAeternaInteractableActor* InteractableActor = Cast<AAeternaInteractableActor>(Actor);
	if (!InteractableActor || InteractableActor->IsInteractionCompleted())
	{
		return bTrackCarryObjectives && IsCarryObjectiveActor(Actor);
	}

	if (!InteractableActor->IsActiveForScenario(GetCurrentScenarioId()))
	{
		return false;
	}

	const EAeternaInteractionType InteractionType = InteractableActor->GetInteractionType();
	return InteractionType == EAeternaInteractionType::Scan
		|| InteractionType == EAeternaInteractionType::Charge
		|| InteractionType == EAeternaInteractionType::Install
		|| InteractionType == EAeternaInteractionType::Pickup;
}

bool UAeternaRadarHudComponent::IsCarryObjectiveActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	bool bCarryTagged = false;
	for (const FName& Tag : Actor->Tags)
	{
		const FString TagString = Tag.ToString();
		if (Tag == TEXT("Carry") || TagString.StartsWith(TEXT("Carry_"), ESearchCase::CaseSensitive))
		{
			bCarryTagged = true;
			break;
		}
	}

	if (!bCarryTagged)
	{
		return false;
	}

	if (GetCurrentScenarioId() != TEXT("S02_GrandHallFossil"))
	{
		return false;
	}

	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	const UAeternaScanProgressComponent* ScanProgressComponent = AeternaCharacter
		? AeternaCharacter->FindComponentByClass<UAeternaScanProgressComponent>()
		: nullptr;
	return !ScanProgressComponent || !ScanProgressComponent->HasScannedPoint(Actor->GetFName());
}

FName UAeternaRadarHudComponent::GetCurrentScenarioId() const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	return ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;
}
