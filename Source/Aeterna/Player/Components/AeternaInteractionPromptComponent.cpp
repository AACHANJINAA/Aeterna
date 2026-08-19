// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaInteractionPromptComponent.h"

#include "Player/AeternaCharacter.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

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
	}

	if (UTextBlock* ActionTextBlock = FindTextBlock(ActionTextBlockName))
	{
		ActionTextBlock->SetText(ActionText);
	}

	if (UTextBlock* PromptTextBlock = FindTextBlock(PromptTextBlockName))
	{
		const FText PromptText = InteractionInfo.PromptText.IsEmpty()
			? FText::FromString(TEXT("Interact"))
			: InteractionInfo.PromptText;
		PromptTextBlock->SetText(PromptText);
	}

	PromptWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetComponentTickEnabled(true);
	UpdatePromptScreenPosition();
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

	FVector Origin = CurrentPromptActor->GetActorLocation();
	FVector Extent = FVector::ZeroVector;
	CurrentPromptActor->GetActorBounds(false, Origin, Extent);

	const FVector PromptWorldLocation = Origin + FVector(0.0f, 0.0f, Extent.Z + PromptWorldVerticalOffset);
	FVector2D ScreenPosition;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController, PromptWorldLocation, ScreenPosition, true))
	{
		PromptWidget->SetPositionInViewport(ScreenPosition, true);
	}
}

void UAeternaInteractionPromptComponent::HidePromptImmediately()
{
	CurrentPromptActor = nullptr;
	SetComponentTickEnabled(false);
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
