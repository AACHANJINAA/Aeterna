// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AeternaPlayerComponent.h"
#include "AeternaNotebookHudComponent.generated.h"

class UTexture2D;
class UTextBlock;
class UWidget;
class UUserWidget;

UENUM(BlueprintType)
enum class EAeternaNotebookPage : uint8
{
	Todo,
	Warnings,
	Controls
};

USTRUCT(BlueprintType)
struct FAeternaNotebookTextSlot
{
	GENERATED_BODY()

	FAeternaNotebookTextSlot() = default;

	FAeternaNotebookTextSlot(EAeternaNotebookPage InPage, FName InTextBlockName, const FText& InScenario1Text, const FText& InScenario2Text, const FText& InScenario3Text)
		: Page(InPage)
		, TextBlockName(InTextBlockName)
		, Scenario1Text(InScenario1Text)
		, Scenario2Text(InScenario2Text)
		, Scenario3Text(InScenario3Text)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notebook")
	EAeternaNotebookPage Page = EAeternaNotebookPage::Todo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notebook")
	FName TextBlockName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notebook", meta=(MultiLine="true"))
	FText Scenario1Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notebook", meta=(MultiLine="true"))
	FText Scenario2Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Notebook", meta=(MultiLine="true"))
	FText Scenario3Text;
};

UCLASS(ClassGroup=(Aeterna), meta=(BlueprintSpawnableComponent))
class AETERNA_API UAeternaNotebookHudComponent : public UAeternaPlayerComponent
{
	GENERATED_BODY()

public:
	UAeternaNotebookHudComponent();

	virtual void BeginPlay() override;
	virtual void InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void UpdateNotebookHud();

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void SetNotebookOpen(bool bOpen);

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void SetNotebookPage(EAeternaNotebookPage NewPage);

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void ShowPreviousNotebookPage();

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void ShowNextNotebookPage();

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void ShowPreviousNotebookDay();

	UFUNCTION(BlueprintCallable, Category="Notebook|HUD")
	void ShowNextNotebookDay();

	UFUNCTION(BlueprintPure, Category="Notebook|HUD")
	EAeternaNotebookPage GetNotebookPage() const { return CurrentNotebookPage; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD")
	TObjectPtr<UTexture2D> NotebookIconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD")
	TSubclassOf<UUserWidget> NotebookJournalWidgetClass;

	/** WB_QuestJournal_UI가 아직 깨졌을 때만 임시 C++ 두루마리 화면을 씁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	bool bUseNativeJournalFallback = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	TObjectPtr<UTexture2D> NotebookJournalBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Position", meta=(ClampMin="0.0", ClampMax="1.0"))
	FVector2D ViewportAnchorNormalized = FVector2D(0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Position")
	FVector2D ViewportOffset = FVector2D(32.0f, 32.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Position")
	FVector2D HudWidgetSize = FVector2D(76.0f, 76.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD|Position")
	FVector2D ReferenceViewportSize = FVector2D(1920.0f, 1080.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|HUD")
	int32 ViewportZOrder = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	FText Scenario1NotebookText = FText::FromString(TEXT("Day 1"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	FText Scenario2NotebookText = FText::FromString(TEXT("Day 2"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	FText Scenario3NotebookText = FText::FromString(TEXT("Day 3"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JournalViewportWidthRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JournalViewportHeightRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal", meta=(ClampMin="0.0", ClampMax="1.0"))
	float JournalRenderOpacity = 1.0f;

	/** Designer에 배치된 TextBlock 이름별로, 노트가 열릴 때 C++이 넣어줄 문구입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	TArray<FAeternaNotebookTextSlot> JournalTextSlots;

	/** 위 슬롯에 이름이 없을 때만 쓰는 단일 본문 TextBlock 후보입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Notebook|Journal")
	TArray<FName> JournalTextBlockNames;

private:
	void CreateNotebookHudWidget();
	void CreateNotebookJournalWidget();
	void UpdateNotebookHudPosition();
	void UpdateNotebookJournalPosition();
	void ApplyNotebookJournalText();
	void ApplyNotebookNavigationText();
	void ApplyNotebookSectionVisibility();
	void ProcessNotebookPageInput();
	bool SetNamedTextBlock(FName TextBlockName, const FText& Text);
	void SetFirstMatchingTextBlock(const TArray<FName>& TextBlockNames, const FText& Text);
	bool SetNamedWidgetVisibility(FName WidgetName, ESlateVisibility Visibility);
	UTextBlock* FindTextBlockWithinWidget(UWidget* RootWidget) const;
	int32 GetCurrentScenarioNotebookDayIndex() const;
	int32 GetUnlockedNotebookDayIndex() const;
	bool IsNotebookDayLocked() const;
	FText ResolveNotebookSlotText(const FAeternaNotebookTextSlot& TextSlot) const;
	FText ResolveNotebookText() const;
	UTexture2D* ResolveNotebookIconTexture() const;
	UTexture2D* ResolveNotebookJournalBackgroundTexture() const;
	TSubclassOf<UUserWidget> ResolveNotebookJournalWidgetClass() const;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> NotebookHudWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> NotebookJournalWidget;

	EAeternaNotebookPage CurrentNotebookPage = EAeternaNotebookPage::Todo;
	int32 CurrentNotebookDayIndex = 0;
	bool bNotebookPageInputHeld = false;
	bool bNotebookDayInputHeld = false;
};
