// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "AeternaNotebookHudWidget.generated.h"

class SBox;
class SWidget;
class UTexture2D;

UCLASS()
class AETERNA_API UAeternaNotebookHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void SetNotebookIconTexture(UTexture2D* InIconTexture);

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void SetWidgetSize(FVector2D InWidgetSize);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Layout")
	FVector2D WidgetSize = FVector2D(76.0f, 76.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Color")
	FLinearColor PanelColor = FLinearColor(0.015f, 0.06f, 0.055f, 0.42f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Color")
	FLinearColor BorderColor = FLinearColor(0.15f, 1.0f, 0.78f, 0.86f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Color")
	FLinearColor IconTint = FLinearColor(0.82f, 1.0f, 0.94f, 0.96f);

private:
	TSharedRef<SWidget> BuildFallbackIcon() const;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconTexture;

	FSlateBrush IconBrush;
	TSharedPtr<SBox> RootSizeBox;
};
