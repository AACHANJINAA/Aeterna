// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "AeternaNotebookJournalWidget.generated.h"

class SBox;
class STextBlock;
class SWidget;
class UTexture2D;

UCLASS()
class AETERNA_API UAeternaNotebookJournalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category="Notebook|Journal")
	void SetNotebookText(const FText& InNotebookText);

	UFUNCTION(BlueprintCallable, Category="Notebook|Journal")
	void SetBackgroundTexture(UTexture2D* InBackgroundTexture);

	UFUNCTION(BlueprintCallable, Category="Notebook|Journal")
	void SetWidgetSize(FVector2D InWidgetSize);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal|Layout")
	FVector2D WidgetSize = FVector2D(960.0f, 540.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal|Color")
	FLinearColor PanelColor = FLinearColor(0.74f, 0.56f, 0.32f, 0.52f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal|Color")
	FLinearColor TextColor = FLinearColor(0.08f, 0.055f, 0.035f, 0.96f);

private:
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> BackgroundTexture;

	FText NotebookText;
	FSlateBrush BackgroundBrush;
	TSharedPtr<SBox> RootSizeBox;
	TSharedPtr<STextBlock> NotebookTextWidget;
};

