// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/AeternaInteractableInterface.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "Styling/SlateColor.h"
#include "TimerManager.h"
#include "AeternaInteractionPromptComponent.generated.h"

class UTextBlock;
class UUserWidget;

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaInteractionPromptComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaInteractionPromptComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;

	UFUNCTION(BlueprintCallable, Category="Interaction|Prompt")
	void ShowPrompt(AActor* PromptActor, const FAeternaInteractionInfo& InteractionInfo);

	UFUNCTION(BlueprintCallable, Category="Interaction|Prompt")
	void HidePrompt();

	UFUNCTION(BlueprintCallable, Category="Interaction|Prompt")
	void ShowSuccessFeedback(AActor* PromptActor);

	UFUNCTION(BlueprintPure, Category="Interaction|Prompt")
	bool IsShowingSuccessFeedback() const { return bShowingSuccessFeedback; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	TSubclassOf<UUserWidget> PromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FName ActionTextBlockName = TEXT("InteractText");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FName PromptTextBlockName = TEXT("Name");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FText ActionText = FText::FromString(TEXT("[E]"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FText SuccessText = FText::FromString(TEXT("Success"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FSlateColor SuccessTextColor = FSlateColor(FLinearColor(0.1f, 1.0f, 0.35f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	int32 ViewportZOrder = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	FVector2D PromptViewportSize = FVector2D(420.0f, 120.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt", meta=(ClampMin="0.0", Units="s"))
	float PromptHideDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt", meta=(Units="cm"))
	float PromptWorldVerticalOffset = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	bool bProjectPromptToWorldLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction|Prompt", meta=(ClampMin="0.0", Units="s"))
	float SuccessFeedbackDuration = 0.7f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|Prompt")
	bool bShowingSuccessFeedback = false;

private:
	void CreatePromptWidget();
	UTextBlock* FindTextBlock(FName TextBlockName) const;
	void SetPromptText(const FText& InActionText, const FText& InPromptText);
	void SetPromptTextColor(const FSlateColor& InTextColor);
	void CacheDefaultPromptTextColor();
	void UpdatePromptScreenPosition();
	void HidePromptImmediately();

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PromptWidget;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentPromptActor;

	FSlateColor DefaultPromptTextColor;
	bool bDefaultPromptTextColorCached = false;

	FTimerHandle HidePromptTimerHandle;
	FTimerHandle SuccessFeedbackTimerHandle;
};
