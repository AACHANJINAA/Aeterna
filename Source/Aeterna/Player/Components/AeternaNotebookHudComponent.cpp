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
		{ EAeternaNotebookPage::Todo, TEXT("QuestName"), FText::FromString(TEXT("M-05 MAINTENANCE LOG")), FText::FromString(TEXT("M-05 RESTORATION LOG")), FText::FromString(TEXT("M-05 LIGHTING LOG")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestName"), FText::FromString(TEXT("M-05 RISK NOTICE")), FText::FromString(TEXT("M-05 HANDLING NOTICE")), FText::FromString(TEXT("M-05 ELECTRICAL NOTICE")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestName"), FText::FromString(TEXT("M-05 OPERATOR GUIDE")), FText::FromString(TEXT("M-05 OPERATOR GUIDE")), FText::FromString(TEXT("M-05 OPERATOR GUIDE")) },
		{ EAeternaNotebookPage::Todo, TEXT("QuestCategory"), FText::FromString(TEXT("TO DO")), FText::FromString(TEXT("TO DO")), FText::FromString(TEXT("TO DO")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestCategory"), FText::FromString(TEXT("WARNINGS")), FText::FromString(TEXT("WARNINGS")), FText::FromString(TEXT("WARNINGS")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestCategory"), FText::FromString(TEXT("CONTROLS")), FText::FromString(TEXT("CONTROLS")), FText::FromString(TEXT("CONTROLS")) },
		{ EAeternaNotebookPage::Todo, TEXT("QuestRegion"), FText::FromString(TEXT("DAY 01")), FText::FromString(TEXT("DAY 02")), FText::FromString(TEXT("DAY 03")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestRegion"), FText::FromString(TEXT("DAY 01")), FText::FromString(TEXT("DAY 02")), FText::FromString(TEXT("DAY 03")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestRegion"), FText::FromString(TEXT("CONTROLS")), FText::FromString(TEXT("CONTROLS")), FText::FromString(TEXT("CONTROLS")) },
		{ EAeternaNotebookPage::Todo, TEXT("TXT_SelectGuide"), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty() },
		{ EAeternaNotebookPage::Warnings, TEXT("TXT_SelectGuide"), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty() },
		{ EAeternaNotebookPage::Controls, TEXT("TXT_SelectGuide"), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty() },
		{ EAeternaNotebookPage::Todo, TEXT("Description"), FText::FromString(TEXT("Assigned Work")), FText::FromString(TEXT("Assigned Work")), FText::FromString(TEXT("Assigned Work")) },
		{ EAeternaNotebookPage::Todo, TEXT("Descriptions"), FText::FromString(TEXT("Scan the display case fossil.\nRecover one battery pack.\nScan the Tirano information board.")), FText::FromString(TEXT("Restore twelve displaced bones.\nPlace each remain at the fossil restoration area.\nConfirm progress before leaving the hall.")), FText::FromString(TEXT("Find the abnormal blue lamps.\nTurn off every forbidden light.\nReturn to the office after the circuit is stable.")) },
		{ EAeternaNotebookPage::Warnings, TEXT("Description"), FText::FromString(TEXT("Operational Caution")), FText::FromString(TEXT("Operational Caution")), FText::FromString(TEXT("Operational Caution")) },
		{ EAeternaNotebookPage::Warnings, TEXT("Descriptions"), FText::FromString(TEXT("Do not scan unrelated exhibits.\nKeep headlamp usage brief.\nIf the log differs from the room, trust the log first.")), FText::FromString(TEXT("Do not carry remains outside the restoration route.\nDo not place bones on unmarked furniture.\nCheck the radar before moving to the next remain.")), FText::FromString(TEXT("Do not ignore active blue light.\nDo not stay under unstable lamps.\nUse the radar to confirm forbidden light targets.")) },
		{ EAeternaNotebookPage::Controls, TEXT("Description"), FText::FromString(TEXT("Input Reference")), FText::FromString(TEXT("Input Reference")), FText::FromString(TEXT("Input Reference")) },
		{ EAeternaNotebookPage::Controls, TEXT("Descriptions"), FText::FromString(TEXT("WASD: Move\nMouse Movement: Adjust View / FOV\nE: Interact\nF: Headlamp On/Off\nShift: Sprint")), FText::FromString(TEXT("WASD: Move\nMouse Movement: Adjust View / FOV\nE: Interact\nF: Headlamp On/Off\nShift: Sprint")), FText::FromString(TEXT("WASD: Move\nMouse Movement: Adjust View / FOV\nE: Interact\nF: Headlamp On/Off\nShift: Sprint")) }
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
		CurrentNotebookDayIndex = GetCurrentScenarioNotebookDayIndex();
		ApplyNotebookJournalText();
		UpdateNotebookJournalPosition();
		NotebookJournalWidget->SetRenderOpacity(FMath::Clamp(JournalRenderOpacity, 0.0f, 1.0f));
		NotebookJournalWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		bNotebookPageInputHeld = false;
		bNotebookDayInputHeld = false;
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

void UAeternaNotebookHudComponent::ShowPreviousNotebookDay()
{
	const int32 PreviousDayIndex = FMath::Max(0, CurrentNotebookDayIndex - 1);
	if (PreviousDayIndex != CurrentNotebookDayIndex)
	{
		CurrentNotebookDayIndex = PreviousDayIndex;
		ApplyNotebookJournalText();
	}
}

void UAeternaNotebookHudComponent::ShowNextNotebookDay()
{
	const int32 NextDayIndex = FMath::Min(2, CurrentNotebookDayIndex + 1);
	if (NextDayIndex != CurrentNotebookDayIndex)
	{
		CurrentNotebookDayIndex = NextDayIndex;
		ApplyNotebookJournalText();
	}
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

	ApplyNotebookSectionVisibility();
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

void UAeternaNotebookHudComponent::ApplyNotebookSectionVisibility()
{
	SetNamedWidgetVisibility(TEXT("BasicRuleBox"), ESlateVisibility::Collapsed);
	SetNamedWidgetVisibility(TEXT("RewardBox_1"), ESlateVisibility::Collapsed);
	SetNamedWidgetVisibility(TEXT("DescriptionBox_1"), ESlateVisibility::Collapsed);

	SetNamedWidgetVisibility(TEXT("QuestDetailBox"), ESlateVisibility::HitTestInvisible);
	SetNamedWidgetVisibility(TEXT("DescriptionBox"), ESlateVisibility::HitTestInvisible);
	SetNamedWidgetVisibility(TEXT("VerticalBox_129"), ESlateVisibility::HitTestInvisible);

	SetNamedWidgetVisibility(TEXT("RewardBox"), ESlateVisibility::Collapsed);
	SetNamedWidgetVisibility(TEXT("QuestGoalsBox"), ESlateVisibility::Collapsed);
	SetNamedWidgetVisibility(TEXT("ObservationBox"), ESlateVisibility::Collapsed);
	SetNamedWidgetVisibility(TEXT("Status_Complete_Box"), ESlateVisibility::Collapsed);
	SetNamedWidgetVisibility(TEXT("Status_InProgress_Box"), ESlateVisibility::Collapsed);
}

void UAeternaNotebookHudComponent::ProcessNotebookPageInput()
{
	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	if (!AeternaCharacter || !AeternaCharacter->IsNotebookOpen())
	{
		bNotebookPageInputHeld = false;
		bNotebookDayInputHeld = false;
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
	const bool bADown = PlayerController->IsInputKeyDown(EKeys::A);
	const bool bDDown = PlayerController->IsInputKeyDown(EKeys::D);
	if (!bWDown && !bSDown)
	{
		bNotebookPageInputHeld = false;
	}
	else if (!bNotebookPageInputHeld)
	{
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

	if (!bADown && !bDDown)
	{
		bNotebookDayInputHeld = false;
	}
	else if (!bNotebookDayInputHeld)
	{
		bNotebookDayInputHeld = true;
		if (bADown)
		{
			ShowPreviousNotebookDay();
		}
		else
		{
			ShowNextNotebookDay();
		}
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
		TextBlock->SetRenderOpacity(1.0f);
		return true;
	}

	return false;
}

bool UAeternaNotebookHudComponent::SetNamedWidgetVisibility(FName WidgetName, ESlateVisibility Visibility)
{
	if (WidgetName.IsNone() || !NotebookJournalWidget || !NotebookJournalWidget->WidgetTree)
	{
		return false;
	}

	if (UWidget* Widget = NotebookJournalWidget->WidgetTree->FindWidget(WidgetName))
	{
		Widget->SetVisibility(Visibility);
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

int32 UAeternaNotebookHudComponent::GetCurrentScenarioNotebookDayIndex() const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	const FName CurrentScenarioId = ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;

	if (CurrentScenarioId == TEXT("S02_GrandHallFossil"))
	{
		return 1;
	}

	if (CurrentScenarioId == TEXT("S03_ForbiddenLight"))
	{
		return 2;
	}

	return 0;
}

int32 UAeternaNotebookHudComponent::GetUnlockedNotebookDayIndex() const
{
	return GetCurrentScenarioNotebookDayIndex();
}

bool UAeternaNotebookHudComponent::IsNotebookDayLocked() const
{
	if (CurrentNotebookPage == EAeternaNotebookPage::Controls)
	{
		return false;
	}

	return CurrentNotebookDayIndex > GetUnlockedNotebookDayIndex();
}

FText UAeternaNotebookHudComponent::ResolveNotebookSlotText(const FAeternaNotebookTextSlot& TextSlot) const
{
	if (IsNotebookDayLocked()
		&& TextSlot.TextBlockName != FName(TEXT("QuestRegion"))
		&& TextSlot.TextBlockName != FName(TEXT("TXT_SelectGuide")))
	{
		return FText::FromString(TEXT("???"));
	}

	switch (FMath::Clamp(CurrentNotebookDayIndex, 0, 2))
	{
	case 1:
		return TextSlot.Scenario2Text;
	case 2:
		return TextSlot.Scenario3Text;
	default:
		return TextSlot.Scenario1Text;
	}
}

FText UAeternaNotebookHudComponent::ResolveNotebookText() const
{
	switch (FMath::Clamp(CurrentNotebookDayIndex, 0, 2))
	{
	case 1:
		return Scenario2NotebookText;
	case 2:
		return Scenario3NotebookText;
	default:
		return Scenario1NotebookText;
	}
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
