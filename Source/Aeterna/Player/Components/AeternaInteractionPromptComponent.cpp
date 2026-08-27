// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaInteractionPromptComponent.h"

#include "Player/AeternaCharacter.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const FName InteractionBoundsComponentName(TEXT("InteractionBounds"));
	const FName InteractComponentName(TEXT("Interact"));
	constexpr float PromptTopOffsetCm = 0.0f;

	bool IsPromptAnchorComponent(const UActorComponent* Component)
	{
		const FString ComponentName = Component ? Component->GetName() : FString();
		return Component
			&& (Component->GetFName() == InteractionBoundsComponentName
				|| Component->GetFName() == InteractComponentName
				|| ComponentName.Contains(TEXT("InteractionBounds"))
				|| ComponentName.Contains(TEXT("Interact")));
	}

	bool TryGetPromptAnchorLocation(const AActor* PromptActor, FVector& OutPromptWorldLocation)
	{
		if (!PromptActor)
		{
			return false;
		}

		TArray<UBoxComponent*> BoxComponents;
		PromptActor->GetComponents<UBoxComponent>(BoxComponents);
		for (const UBoxComponent* BoxComponent : BoxComponents)
		{
			if (!IsPromptAnchorComponent(BoxComponent))
			{
				continue;
			}

			OutPromptWorldLocation = BoxComponent->GetComponentLocation()
				+ BoxComponent->GetUpVector() * (BoxComponent->GetScaledBoxExtent().Z + PromptTopOffsetCm);
			return true;
		}

		TArray<USceneComponent*> SceneComponents;
		PromptActor->GetComponents<USceneComponent>(SceneComponents);
		for (const USceneComponent* SceneComponent : SceneComponents)
		{
			if (!IsPromptAnchorComponent(SceneComponent))
			{
				continue;
			}

			if (const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(SceneComponent))
			{
				OutPromptWorldLocation = PrimitiveComponent->Bounds.Origin
					+ FVector(0.0f, 0.0f, PrimitiveComponent->Bounds.BoxExtent.Z + PromptTopOffsetCm);
				return true;
			}

			OutPromptWorldLocation = SceneComponent->GetComponentLocation()
				+ SceneComponent->GetUpVector() * PromptTopOffsetCm;
			return true;
		}

		return false;
	}
}

UAeternaInteractionPromptComponent::UAeternaInteractionPromptComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAeternaInteractionPromptComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdatePromptScreenPosition();
}

void UAeternaInteractionPromptComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);
	CreatePromptWidget();
	HidePrompt();
}

void UAeternaInteractionPromptComponent::ShowPrompt(AActor* PromptActor, const FAeternaInteractionInfo& InteractionInfo)
{
	CreatePromptWidget();
	if (!PromptWidget || !PromptActor)
	{
		return;
	}

	CurrentPromptActor = PromptActor;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HidePromptTimerHandle);
		World->GetTimerManager().ClearTimer(SuccessFeedbackTimerHandle);
	}

	bShowingSuccessFeedback = false;
	const FText PromptText = InteractionInfo.PromptText.IsEmpty()
		? FText::FromString(TEXT("Interact"))
		: InteractionInfo.PromptText;
	SetPromptText(ActionText, PromptText);
	if (bDefaultPromptTextColorCached)
	{
		SetPromptTextColor(DefaultPromptTextColor);
	}

	PromptWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetComponentTickEnabled(bProjectPromptToWorldLocation);
	if (bProjectPromptToWorldLocation)
	{
		UpdatePromptScreenPosition();
	}
}

void UAeternaInteractionPromptComponent::HidePrompt()
{
	if (!PromptWidget)
	{
		return;
	}

	if (PromptHideDelay <= 0.0f)
	{
		HidePromptImmediately();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HidePromptTimerHandle, this, &UAeternaInteractionPromptComponent::HidePromptImmediately, PromptHideDelay, false);
		return;
	}

	HidePromptImmediately();
}

void UAeternaInteractionPromptComponent::ShowSuccessFeedback(AActor* PromptActor)
{
	CreatePromptWidget();
	if (!PromptWidget || !PromptActor)
	{
		return;
	}

	CurrentPromptActor = PromptActor;
	bShowingSuccessFeedback = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HidePromptTimerHandle);
		World->GetTimerManager().ClearTimer(SuccessFeedbackTimerHandle);
		if (SuccessFeedbackDuration > 0.0f)
		{
			World->GetTimerManager().SetTimer(SuccessFeedbackTimerHandle, this, &UAeternaInteractionPromptComponent::HidePromptImmediately, SuccessFeedbackDuration, false);
		}
	}

	SetPromptText(FText::GetEmpty(), SuccessText);
	SetPromptTextColor(SuccessTextColor);
	PromptWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetComponentTickEnabled(bProjectPromptToWorldLocation);
	if (bProjectPromptToWorldLocation)
	{
		UpdatePromptScreenPosition();
	}

	if (SuccessFeedbackDuration <= 0.0f)
	{
		HidePromptImmediately();
	}
}

void UAeternaInteractionPromptComponent::CreatePromptWidget()
{
	if (PromptWidget || !PromptWidgetClass)
	{
		return;
	}

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		PromptWidget = CreateWidget<UUserWidget>(AeternaCharacter->GetWorld(), PromptWidgetClass);
		if (PromptWidget)
		{
			PromptWidget->AddToViewport(ViewportZOrder);
			PromptWidget->SetDesiredSizeInViewport(PromptViewportSize);
			PromptWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
			CacheDefaultPromptTextColor();
		}
	}
}

UTextBlock* UAeternaInteractionPromptComponent::FindTextBlock(FName TextBlockName) const
{
	if (!PromptWidget || TextBlockName.IsNone())
	{
		return nullptr;
	}

	return Cast<UTextBlock>(PromptWidget->GetWidgetFromName(TextBlockName));
}

void UAeternaInteractionPromptComponent::SetPromptText(const FText& InActionText, const FText& InPromptText)
{
	if (UTextBlock* ActionTextBlock = FindTextBlock(ActionTextBlockName))
	{
		ActionTextBlock->SetText(InActionText);
		ActionTextBlock->SetJustification(ETextJustify::Center);
	}

	if (UTextBlock* PromptTextBlock = FindTextBlock(PromptTextBlockName))
	{
		PromptTextBlock->SetText(InPromptText);
		PromptTextBlock->SetJustification(ETextJustify::Center);
	}
}

void UAeternaInteractionPromptComponent::SetPromptTextColor(const FSlateColor& InTextColor)
{
	if (UTextBlock* PromptTextBlock = FindTextBlock(PromptTextBlockName))
	{
		PromptTextBlock->SetColorAndOpacity(InTextColor);
	}
}

void UAeternaInteractionPromptComponent::CacheDefaultPromptTextColor()
{
	if (bDefaultPromptTextColorCached)
	{
		return;
	}

	if (UTextBlock* PromptTextBlock = FindTextBlock(PromptTextBlockName))
	{
		DefaultPromptTextColor = PromptTextBlock->GetColorAndOpacity();
		bDefaultPromptTextColorCached = true;
	}
}

FVector2D UAeternaInteractionPromptComponent::GetPromptContentDesiredSize() const
{
	FVector2D ContentSize = FVector2D::ZeroVector;

	if (const UTextBlock* PromptTextBlock = FindTextBlock(PromptTextBlockName))
	{
		const FVector2D PromptTextSize = PromptTextBlock->GetDesiredSize();
		ContentSize.X = FMath::Max(ContentSize.X, PromptTextSize.X);
		ContentSize.Y += PromptTextSize.Y;
	}

	if (const UTextBlock* ActionTextBlock = FindTextBlock(ActionTextBlockName))
	{
		const FVector2D ActionTextSize = ActionTextBlock->GetDesiredSize();
		ContentSize.X = FMath::Max(ContentSize.X, ActionTextSize.X);
		ContentSize.Y += ActionTextSize.Y;
	}

	if (ContentSize.X <= 1.0f || ContentSize.Y <= 1.0f)
	{
		return PromptViewportSize;
	}

	constexpr float ContentPaddingX = 24.0f;
	constexpr float ContentPaddingY = 12.0f;
	return ContentSize + FVector2D(ContentPaddingX, ContentPaddingY);
}

void UAeternaInteractionPromptComponent::UpdatePromptScreenPosition()
{
	if (!PromptWidget || !CurrentPromptActor)
	{
		return;
	}

	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	APlayerController* PlayerController = AeternaCharacter ? Cast<APlayerController>(AeternaCharacter->GetController()) : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FVector PromptWorldLocation = CurrentPromptActor->GetActorLocation();
	if (!TryGetPromptAnchorLocation(CurrentPromptActor, PromptWorldLocation))
	{
		FVector Origin = CurrentPromptActor->GetActorLocation();
		FVector Extent = FVector::ZeroVector;
		CurrentPromptActor->GetActorBounds(false, Origin, Extent);
		PromptWorldLocation = Origin + FVector(0.0f, 0.0f, Extent.Z + PromptTopOffsetCm);
	}

	FVector2D ScreenPosition;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, PromptWorldLocation, ScreenPosition, true))
	{
		PromptWidget->ForceLayoutPrepass();
		const FVector2D PromptContentSize = GetPromptContentDesiredSize();
		PromptWidget->SetDesiredSizeInViewport(PromptContentSize);
		PromptWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		const FVector2D TextCenterOffset(PromptContentSize.X * 0.5f, 0.0f);
		PromptWidget->SetPositionInViewport(ScreenPosition + TextCenterOffset + PromptScreenOffset, true);
	}
}

void UAeternaInteractionPromptComponent::HidePromptImmediately()
{
	CurrentPromptActor = nullptr;
	bShowingSuccessFeedback = false;
	SetComponentTickEnabled(false);
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
