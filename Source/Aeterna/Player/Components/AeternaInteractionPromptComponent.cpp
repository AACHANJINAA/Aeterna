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
	SetComponentTickEnabled(true);
	UpdatePromptScreenPosition();

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
	}

	if (UTextBlock* PromptTextBlock = FindTextBlock(PromptTextBlockName))
	{
		PromptTextBlock->SetText(InPromptText);
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
	bShowingSuccessFeedback = false;
	SetComponentTickEnabled(false);
	if (PromptWidget)
	{
		PromptWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
