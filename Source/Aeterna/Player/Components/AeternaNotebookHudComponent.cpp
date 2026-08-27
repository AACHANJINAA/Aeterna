// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaNotebookHudComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Player/AeternaCharacter.h"
#include "Player/UI/AeternaHudLayoutUtils.h"
#include "Player/UI/AeternaNotebookJournalWidget.h"
#include "Player/UI/AeternaNotebookHudWidget.h"
#include "Blueprint/WidgetTree.h"

UAeternaNotebookHudComponent::UAeternaNotebookHudComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;

	JournalTextSlots =
	{
		{ EAeternaNotebookPage::Todo, TEXT("NotebookDate"), FText::FromString(TEXT("NIGHT 01 / TO DO")), FText::FromString(TEXT("NIGHT 02 / TO DO")), FText::FromString(TEXT("NIGHT 03 / TO DO")) },
		{ EAeternaNotebookPage::Warnings, TEXT("NotebookDate"), FText::FromString(TEXT("NIGHT 01 / WARNINGS")), FText::FromString(TEXT("NIGHT 02 / WARNINGS")), FText::FromString(TEXT("NIGHT 03 / WARNINGS")) },
		{ EAeternaNotebookPage::Controls, TEXT("NotebookDate"), FText::FromString(TEXT("NIGHT 01 / CONTROLS")), FText::FromString(TEXT("NIGHT 02 / CONTROLS")), FText::FromString(TEXT("NIGHT 03 / CONTROLS")) },
		{ EAeternaNotebookPage::Todo, TEXT("QuestName"), FText::FromString(TEXT("M-05 MAINTENANCE LOG / TO DO")), FText::FromString(TEXT("M-05 MAINTENANCE LOG / TO DO")), FText::FromString(TEXT("M-05 MAINTENANCE LOG / TO DO")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestName"), FText::FromString(TEXT("M-05 MAINTENANCE LOG / WARNINGS")), FText::FromString(TEXT("M-05 MAINTENANCE LOG / WARNINGS")), FText::FromString(TEXT("M-05 MAINTENANCE LOG / WARNINGS")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestName"), FText::FromString(TEXT("M-05 MAINTENANCE LOG / CONTROLS")), FText::FromString(TEXT("M-05 MAINTENANCE LOG / CONTROLS")), FText::FromString(TEXT("M-05 MAINTENANCE LOG / CONTROLS")) },
		{ EAeternaNotebookPage::Todo, TEXT("QuestCategory"), FText::FromString(TEXT("TO DO")), FText::FromString(TEXT("TO DO")), FText::FromString(TEXT("TO DO")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestCategory"), FText::FromString(TEXT("WARNINGS")), FText::FromString(TEXT("WARNINGS")), FText::FromString(TEXT("WARNINGS")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestCategory"), FText::FromString(TEXT("CONTROLS")), FText::FromString(TEXT("CONTROLS")), FText::FromString(TEXT("CONTROLS")) },
		{ EAeternaNotebookPage::Todo, TEXT("QuestRegion"), FText::FromString(TEXT("NIGHT 01")), FText::FromString(TEXT("NIGHT 02")), FText::FromString(TEXT("NIGHT 03")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestRegion"), FText::FromString(TEXT("NIGHT 01")), FText::FromString(TEXT("NIGHT 02")), FText::FromString(TEXT("NIGHT 03")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestRegion"), FText::FromString(TEXT("NIGHT 01")), FText::FromString(TEXT("NIGHT 02")), FText::FromString(TEXT("NIGHT 03")) },
		{ EAeternaNotebookPage::Todo, TEXT("TXT_SelectGuide"), FText::FromString(TEXT("Select a log category with W/S.")), FText::FromString(TEXT("Select a log category with W/S.")), FText::FromString(TEXT("Select a log category with W/S.")) },
		{ EAeternaNotebookPage::Warnings, TEXT("TXT_SelectGuide"), FText::FromString(TEXT("Select a log category with W/S.")), FText::FromString(TEXT("Select a log category with W/S.")), FText::FromString(TEXT("Select a log category with W/S.")) },
		{ EAeternaNotebookPage::Controls, TEXT("TXT_SelectGuide"), FText::FromString(TEXT("Select a log category with W/S.")), FText::FromString(TEXT("Select a log category with W/S.")), FText::FromString(TEXT("Select a log category with W/S.")) },
		{ EAeternaNotebookPage::Todo, TEXT("RuleTitle01"), FText::FromString(TEXT("Primary Objective")), FText::FromString(TEXT("Primary Objective")), FText::FromString(TEXT("Primary Objective")) },
		{ EAeternaNotebookPage::Todo, TEXT("RuleBody01"), FText::FromString(TEXT("Review the current maintenance task.")), FText::FromString(TEXT("Restore the displaced remains.")), FText::FromString(TEXT("Find and shut down the forbidden lights.")) },
		{ EAeternaNotebookPage::Todo, TEXT("Description"), FText::FromString(TEXT("Task Details")), FText::FromString(TEXT("Task Details")), FText::FromString(TEXT("Task Details")) },
		{ EAeternaNotebookPage::Todo, TEXT("Descriptions"), FText::FromString(TEXT("Review the current maintenance task.")), FText::FromString(TEXT("Restore the displaced remains.")), FText::FromString(TEXT("Find and shut down the forbidden lights.")) },
		{ EAeternaNotebookPage::Todo, TEXT("QuestSection"), FText::FromString(TEXT("Review the current maintenance task.")), FText::FromString(TEXT("Restore the displaced remains.")), FText::FromString(TEXT("Find and shut down the forbidden lights.")) },
		{ EAeternaNotebookPage::Warnings, TEXT("RuleTitle01"), FText::FromString(TEXT("Night Warning")), FText::FromString(TEXT("Night Warning")), FText::FromString(TEXT("Night Warning")) },
		{ EAeternaNotebookPage::Warnings, TEXT("RuleBody01"), FText::FromString(TEXT("Do not ignore deviations from the maintenance log.")), FText::FromString(TEXT("Return each item to its proper place before moving on.")), FText::FromString(TEXT("Keep watch for lights that should not be active.")) },
		{ EAeternaNotebookPage::Warnings, TEXT("Description"), FText::FromString(TEXT("Warning Notes")), FText::FromString(TEXT("Warning Notes")), FText::FromString(TEXT("Warning Notes")) },
		{ EAeternaNotebookPage::Warnings, TEXT("Descriptions"), FText::FromString(TEXT("Do not ignore deviations from the maintenance log.")), FText::FromString(TEXT("Return each item to its proper place before moving on.")), FText::FromString(TEXT("Keep watch for lights that should not be active.")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestSection"), FText::FromString(TEXT("Do not ignore deviations from the maintenance log.")), FText::FromString(TEXT("Return each item to its proper place before moving on.")), FText::FromString(TEXT("Keep watch for lights that should not be active.")) },
		{ EAeternaNotebookPage::Controls, TEXT("RuleTitle01"), FText::FromString(TEXT("Notebook Control")), FText::FromString(TEXT("Notebook Control")), FText::FromString(TEXT("Notebook Control")) },
		{ EAeternaNotebookPage::Controls, TEXT("RuleBody01"), FText::FromString(TEXT("Press W or S to change pages. Press TAB to close the notebook.")), FText::FromString(TEXT("Press W or S to change pages. Press TAB to close the notebook.")), FText::FromString(TEXT("Press W or S to change pages. Press TAB to close the notebook.")) },
		{ EAeternaNotebookPage::Controls, TEXT("Description"), FText::FromString(TEXT("Control Guide")), FText::FromString(TEXT("Control Guide")), FText::FromString(TEXT("Control Guide")) },
		{ EAeternaNotebookPage::Controls, TEXT("Descriptions"), FText::FromString(TEXT("W/S: change notebook page\nTAB: close notebook\nE: interact\nF: toggle headlamp")), FText::FromString(TEXT("W/S: change notebook page\nTAB: close notebook\nE: interact\nF: toggle headlamp")), FText::FromString(TEXT("W/S: change notebook page\nTAB: close notebook\nE: interact\nF: toggle headlamp")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestSection"), FText::FromString(TEXT("Press W or S to change pages. Press TAB to close the notebook.")), FText::FromString(TEXT("Press W or S to change pages. Press TAB to close the notebook.")), FText::FromString(TEXT("Press W or S to change pages. Press TAB to close the notebook.")) }
	};

	JournalTextBlockNames =
	{
		TEXT("NotebookText"),
		TEXT("NotebookContent"),
		TEXT("ContentText"),
		TEXT("RuleText"),
		TEXT("JournalText"),
		TEXT("QuestJournalText"),
		TEXT("Text_Rules"),
		TEXT("Txt_Rules")
	};
}

void UAeternaNotebookHudComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetAeternaCharacter())
	{
		if (AAeternaCharacter* OwnerCharacter = Cast<AAeternaCharacter>(GetOwner()))
		{
			InitializePlayerComponent(OwnerCharacter);
		}
	}

	UpdateNotebookHud();
}

void UAeternaNotebookHudComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);
	CreateNotebookHudWidget();
	UpdateNotebookHud();
}

void UAeternaNotebookHudComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!NotebookHudWidget)
	{
		CreateNotebookHudWidget();
	}

	if (!NotebookJournalWidget)
	{
		CreateNotebookJournalWidget();
	}

	UpdateNotebookHud();
	ProcessNotebookPageInput();
}

void UAeternaNotebookHudComponent::UpdateNotebookHud()
{
	CreateNotebookHudWidget();
	if (!NotebookHudWidget)
	{
		return;
	}

	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	NotebookHudWidget->SetVisibility(AeternaCharacter && AeternaCharacter->HasNotebook()
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);

	UpdateNotebookHudPosition();
	UpdateNotebookJournalPosition();
}

void UAeternaNotebookHudComponent::SetNotebookOpen(bool bOpen)
{
	CreateNotebookJournalWidget();
	if (!NotebookJournalWidget)
	{
		return;
	}

	if (bOpen)
	{
		CurrentNotebookPage = EAeternaNotebookPage::Todo;
		ApplyNotebookJournalText();
		UpdateNotebookJournalPosition();
		NotebookJournalWidget->SetRenderOpacity(FMath::Clamp(JournalRenderOpacity, 0.0f, 1.0f));
		NotebookJournalWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		bNotebookPageInputHeld = false;
		NotebookJournalWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAeternaNotebookHudComponent::SetNotebookPage(EAeternaNotebookPage NewPage)
{
	CurrentNotebookPage = NewPage;
	ApplyNotebookJournalText();
}

void UAeternaNotebookHudComponent::ShowPreviousNotebookPage()
{
	const int32 CurrentPageIndex = static_cast<int32>(CurrentNotebookPage);
	const int32 PreviousPageIndex = (CurrentPageIndex + 2) % 3;
	SetNotebookPage(static_cast<EAeternaNotebookPage>(PreviousPageIndex));
}

void UAeternaNotebookHudComponent::ShowNextNotebookPage()
{
	const int32 CurrentPageIndex = static_cast<int32>(CurrentNotebookPage);
	const int32 NextPageIndex = (CurrentPageIndex + 1) % 3;
	SetNotebookPage(static_cast<EAeternaNotebookPage>(NextPageIndex));
}

void UAeternaNotebookHudComponent::CreateNotebookHudWidget()
{
	if (NotebookHudWidget)
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

	UAeternaNotebookHudWidget* NativeNotebookHudWidget = CreateWidget<UAeternaNotebookHudWidget>(PlayerController, UAeternaNotebookHudWidget::StaticClass());
	NotebookHudWidget = NativeNotebookHudWidget;
	if (NotebookHudWidget)
	{
		if (NativeNotebookHudWidget)
		{
			NativeNotebookHudWidget->SetWidgetSize(HudWidgetSize);
			NativeNotebookHudWidget->SetNotebookIconTexture(ResolveNotebookIconTexture());
		}

		NotebookHudWidget->AddToViewport(ViewportZOrder);
		NotebookHudWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
		NotebookHudWidget->SetVisibility(ESlateVisibility::Collapsed);
		UpdateNotebookHudPosition();
	}
}

void UAeternaNotebookHudComponent::CreateNotebookJournalWidget()
{
	if (NotebookJournalWidget)
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

	const TSubclassOf<UUserWidget> JournalClass = ResolveNotebookJournalWidgetClass();
	if (JournalClass)
	{
		NotebookJournalWidget = CreateWidget<UUserWidget>(PlayerController, JournalClass);
	}
	else if (bUseNativeJournalFallback)
	{
		NotebookJournalWidget = CreateWidget<UAeternaNotebookJournalWidget>(PlayerController, UAeternaNotebookJournalWidget::StaticClass());
	}

	if (NotebookJournalWidget)
	{
		if (UAeternaNotebookJournalWidget* NativeJournalWidget = Cast<UAeternaNotebookJournalWidget>(NotebookJournalWidget))
		{
			NativeJournalWidget->SetBackgroundTexture(ResolveNotebookJournalBackgroundTexture());
		}

		NotebookJournalWidget->AddToViewport(ViewportZOrder + 1);
		NotebookJournalWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		NotebookJournalWidget->SetRenderOpacity(FMath::Clamp(JournalRenderOpacity, 0.0f, 1.0f));
		NotebookJournalWidget->SetVisibility(ESlateVisibility::Collapsed);
		UpdateNotebookJournalPosition();
	}
}

void UAeternaNotebookHudComponent::UpdateNotebookHudPosition()
{
	if (!NotebookHudWidget)
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

	const FVector2D ViewportSize(static_cast<float>(ViewportSizeX), static_cast<float>(ViewportSizeY));
	const float ViewportScale = AeternaHudLayout::GetViewportScale(ViewportSize, ReferenceViewportSize);
	const FVector2D WidgetPosition = AeternaHudLayout::GetAnchoredPosition(ViewportSize, ViewportAnchorNormalized, ViewportOffset, HudWidgetSize, ViewportScale);
	AeternaHudLayout::ApplyScaledViewportLayout(NotebookHudWidget, WidgetPosition, HudWidgetSize, ViewportScale);
}

void UAeternaNotebookHudComponent::UpdateNotebookJournalPosition()
{
	if (!NotebookJournalWidget)
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

	const FVector2D ViewportSize(static_cast<float>(ViewportSizeX), static_cast<float>(ViewportSizeY));
	const FVector2D JournalSize(
		FMath::Max(1.0f, ViewportSize.X * FMath::Clamp(JournalViewportWidthRatio, 0.1f, 1.0f)),
		FMath::Max(1.0f, ViewportSize.Y * FMath::Clamp(JournalViewportHeightRatio, 0.1f, 1.0f)));
	const FVector2D JournalPosition(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);

	if (UAeternaNotebookJournalWidget* NativeJournalWidget = Cast<UAeternaNotebookJournalWidget>(NotebookJournalWidget))
	{
		NativeJournalWidget->SetWidgetSize(JournalSize);
	}

	NotebookJournalWidget->SetDesiredSizeInViewport(JournalSize);
	NotebookJournalWidget->SetPositionInViewport(JournalPosition, true);
}

void UAeternaNotebookHudComponent::ApplyNotebookJournalText()
{
	if (UAeternaNotebookJournalWidget* NativeJournalWidget = Cast<UAeternaNotebookJournalWidget>(NotebookJournalWidget))
	{
		NativeJournalWidget->SetNotebookText(ResolveNotebookText());
		return;
	}

	if (!NotebookJournalWidget || !NotebookJournalWidget->WidgetTree)
	{
		return;
	}

	const FText NotebookText = ResolveNotebookText();
	bool bAppliedText = false;

	ApplyNotebookNavigationText();

	for (const FAeternaNotebookTextSlot& TextSlot : JournalTextSlots)
	{
		if (TextSlot.TextBlockName.IsNone())
		{
			continue;
		}

		if (TextSlot.Page != CurrentNotebookPage)
		{
			continue;
		}

		if (SetNamedTextBlock(TextSlot.TextBlockName, ResolveNotebookSlotText(TextSlot)))
		{
			bAppliedText = true;
		}
	}

	if (!bAppliedText)
	{
		NotebookJournalWidget->WidgetTree->ForEachWidget([this, &NotebookText, &bAppliedText](UWidget* Widget)
		{
			UTextBlock* TextBlock = Cast<UTextBlock>(Widget);
			if (!TextBlock)
			{
				return;
			}

			if (JournalTextBlockNames.Contains(TextBlock->GetFName()))
			{
				TextBlock->SetText(NotebookText);
				bAppliedText = true;
			}
		});
	}

	if (!bAppliedText)
	{
		NotebookJournalWidget->WidgetTree->ForEachWidget([&NotebookText, &bAppliedText](UWidget* Widget)
		{
			if (bAppliedText)
			{
				return;
			}

			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				TextBlock->SetText(NotebookText);
				bAppliedText = true;
			}
		});
	}
}

void UAeternaNotebookHudComponent::ApplyNotebookNavigationText()
{
	SetFirstMatchingTextBlock(
		{ TEXT("LeftObjective01"), TEXT("WB_CatCurrent"), TEXT("CurrentQuest"), TEXT("TabTodo"), TEXT("NotebookTabTodo") },
		CurrentNotebookPage == EAeternaNotebookPage::Todo
			? FText::FromString(TEXT("> TO DO"))
			: FText::FromString(TEXT("  TO DO")));

	SetFirstMatchingTextBlock(
		{ TEXT("LeftObjective02"), TEXT("WB_CatCompleted"), TEXT("CurrentQuest_1"), TEXT("CompletedQuest"), TEXT("TabWarnings"), TEXT("NotebookTabWarnings") },
		CurrentNotebookPage == EAeternaNotebookPage::Warnings
			? FText::FromString(TEXT("> WARNINGS"))
			: FText::FromString(TEXT("  WARNINGS")));

	SetFirstMatchingTextBlock(
		{ TEXT("LeftObjective03"), TEXT("WB_CatFailed"), TEXT("CurrentQuest_2"), TEXT("FailedQuest"), TEXT("TabControls"), TEXT("NotebookTabControls") },
		CurrentNotebookPage == EAeternaNotebookPage::Controls
			? FText::FromString(TEXT("> CONTROLS"))
			: FText::FromString(TEXT("  CONTROLS")));
}

void UAeternaNotebookHudComponent::ProcessNotebookPageInput()
{
	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter || !AeternaCharacter->IsNotebookOpen())
	{
		bNotebookPageInputHeld = false;
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(AeternaCharacter->GetController());
	if (!PlayerController)
	{
		PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	}

	if (!PlayerController)
	{
		return;
	}

	const bool bWDown = PlayerController->IsInputKeyDown(EKeys::W);
	const bool bSDown = PlayerController->IsInputKeyDown(EKeys::S);
	if (!bWDown && !bSDown)
	{
		bNotebookPageInputHeld = false;
		return;
	}

	if (bNotebookPageInputHeld)
	{
		return;
	}

	bNotebookPageInputHeld = true;
	if (bWDown)
	{
		ShowPreviousNotebookPage();
	}
	else
	{
		ShowNextNotebookPage();
	}
}

bool UAeternaNotebookHudComponent::SetNamedTextBlock(FName TextBlockName, const FText& Text)
{
	if (TextBlockName.IsNone() || !NotebookJournalWidget || !NotebookJournalWidget->WidgetTree)
	{
		return false;
	}

	if (UTextBlock* TextBlock = FindTextBlockWithinWidget(NotebookJournalWidget->WidgetTree->FindWidget(TextBlockName)))
	{
		TextBlock->SetText(Text);
		return true;
	}

	return false;
}

void UAeternaNotebookHudComponent::SetFirstMatchingTextBlock(const TArray<FName>& TextBlockNames, const FText& Text)
{
	if (!NotebookJournalWidget || !NotebookJournalWidget->WidgetTree)
	{
		return;
	}

	for (const FName& TextBlockName : TextBlockNames)
	{
		if (TextBlockName.IsNone())
		{
			continue;
		}

		if (SetNamedTextBlock(TextBlockName, Text))
		{
			return;
		}
	}
}

UTextBlock* UAeternaNotebookHudComponent::FindTextBlockWithinWidget(UWidget* RootWidget) const
{
	if (!RootWidget)
	{
		return nullptr;
	}

	if (UTextBlock* TextBlock = Cast<UTextBlock>(RootWidget))
	{
		return TextBlock;
	}

	if (const UUserWidget* UserWidget = Cast<UUserWidget>(RootWidget))
	{
		if (!UserWidget->WidgetTree)
		{
			return nullptr;
		}

		UTextBlock* FoundTextBlock = nullptr;
		UserWidget->WidgetTree->ForEachWidget([&FoundTextBlock](UWidget* ChildWidget)
		{
			if (!FoundTextBlock)
			{
				FoundTextBlock = Cast<UTextBlock>(ChildWidget);
			}
		});

		return FoundTextBlock;
	}

	return nullptr;
}

FText UAeternaNotebookHudComponent::ResolveNotebookSlotText(const FAeternaNotebookTextSlot& TextSlot) const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	const FName CurrentScenarioId = ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;

	if (CurrentScenarioId == TEXT("S02_GrandHallFossil"))
	{
		return TextSlot.Scenario2Text;
	}

	if (CurrentScenarioId == TEXT("S03_ForbiddenLight"))
	{
		return TextSlot.Scenario3Text;
	}

	return TextSlot.Scenario1Text;
}

FText UAeternaNotebookHudComponent::ResolveNotebookText() const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	const FName CurrentScenarioId = ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;

	if (CurrentScenarioId == TEXT("S02_GrandHallFossil"))
	{
		return Scenario2NotebookText;
	}

	if (CurrentScenarioId == TEXT("S03_ForbiddenLight"))
	{
		return Scenario3NotebookText;
	}

	return Scenario1NotebookText;
}

UTexture2D* UAeternaNotebookHudComponent::ResolveNotebookIconTexture() const
{
	if (NotebookIconTexture)
	{
		return NotebookIconTexture;
	}

	const TCHAR* TexturePaths[] =
	{
		TEXT("/Game/QuestSystem_Notebook/Texture/museum-notebook-toggle-icon.museum-notebook-toggle-icon"),
		TEXT("/Game/QuestSystem_Notebook/Texture/museum-notebook-toggle-icon")
	};

	for (const TCHAR* TexturePath : TexturePaths)
	{
		if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TexturePath))
		{
			return Texture;
		}
	}

	return nullptr;
}

UTexture2D* UAeternaNotebookHudComponent::ResolveNotebookJournalBackgroundTexture() const
{
	if (NotebookJournalBackgroundTexture)
	{
		return NotebookJournalBackgroundTexture;
	}

	const TCHAR* TexturePaths[] =
	{
		TEXT("/Game/QuestSystem_Notebook/Texture/openingMap.openingMap"),
		TEXT("/Game/QuestSystem_Notebook/Texture/openingMap"),
		TEXT("/Game/QuestSystem_Notebook/Texture/T_bgQuest_old.T_bgQuest_old"),
		TEXT("/Game/QuestSystem_Notebook/Texture/T_bgQuest_old"),
		TEXT("/Game/QuestSystem/Texture/T_bgQuest.T_bgQuest"),
		TEXT("/Game/QuestSystem/Texture/T_bgQuest")
	};

	for (const TCHAR* TexturePath : TexturePaths)
	{
		if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TexturePath))
		{
			return Texture;
		}
	}

	return nullptr;
}

TSubclassOf<UUserWidget> UAeternaNotebookHudComponent::ResolveNotebookJournalWidgetClass() const
{
	if (NotebookJournalWidgetClass)
	{
		return NotebookJournalWidgetClass;
	}

	const TCHAR* WidgetPaths[] =
	{
		TEXT("/Game/QuestSystem_Notebook/Widgets/WB_QuestJournal_UI.WB_QuestJournal_UI_C"),
		TEXT("/Game/QuestSystem_Notebook/Widgets/WB_QuestJournal_UI_C")
	};

	for (const TCHAR* WidgetPath : WidgetPaths)
	{
		if (UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, WidgetPath))
		{
			return WidgetClass;
		}
	}

	return nullptr;
}
