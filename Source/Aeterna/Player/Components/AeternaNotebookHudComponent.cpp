// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaNotebookHudComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AeternaCharacter.h"
#include "Player/UI/AeternaHudLayoutUtils.h"
#include "Player/UI/AeternaNotebookJournalWidget.h"
#include "Player/UI/AeternaNotebookHudWidget.h"
#include "Sound/SoundBase.h"
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
		{ EAeternaNotebookPage::Todo, TEXT("QuestRegion"), FText::FromString(TEXT("DAY 1")), FText::FromString(TEXT("DAY 2")), FText::FromString(TEXT("DAY 3")) },
		{ EAeternaNotebookPage::Warnings, TEXT("QuestRegion"), FText::FromString(TEXT("DAY 1")), FText::FromString(TEXT("DAY 2")), FText::FromString(TEXT("DAY 3")) },
		{ EAeternaNotebookPage::Controls, TEXT("QuestRegion"), FText::FromString(TEXT("DAY 1")), FText::FromString(TEXT("DAY 2")), FText::FromString(TEXT("DAY 3")) },
		{ EAeternaNotebookPage::Todo, TEXT("TXT_SelectGuide"), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty() },
		{ EAeternaNotebookPage::Warnings, TEXT("TXT_SelectGuide"), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty() },
		{ EAeternaNotebookPage::Controls, TEXT("TXT_SelectGuide"), FText::GetEmpty(), FText::GetEmpty(), FText::GetEmpty() },
		{ EAeternaNotebookPage::Todo, TEXT("Descriptions"), FText::FromString(TEXT("진열장의 화석을 스캔하십시오.\n배터리 팩 1개를 회수하십시오.\n티라노사우루스 알림판을 스캔하십시오.")), FText::FromString(TEXT("떨어진 뼈 12개를 회수하십시오.\n회수한 뼈는 화석 복원 구역에 두십시오.\n홀을 떠나기 전에 진행 상황을 확인하십시오.")), FText::FromString(TEXT("비정상 점등된 조명을 찾으십시오.\n켜져 있으면 안 되는 불을 모두 끄십시오.\n회로가 안정되면 경비실로 복귀하십시오.")) },
		{ EAeternaNotebookPage::Warnings, TEXT("Descriptions"), FText::FromString(TEXT("업무와 무관한 전시물은 스캔하지 마십시오.\n헤드램프 사용은 짧게 하십시오.\n기록과 현장이 다르다면, 기록을 먼저 믿으십시오.")), FText::FromString(TEXT("떨어진 뼈는 모두 회수해서 제자리에 두십시오.\n화석의 눈구멍은 비추지 마십시오. 비어 있는지 확인하는 순간 비어 있지 않게 됩니다.\n화석이 눈앞에서 사라졌다면 움직이지 마십시오.\n매시 22분에는 움직이지 마십시오.")), FText::FromString(TEXT("1시 이후 켜져 있는 불은 끄십시오. 박물관에 다른 근무자는 없습니다.\n빨간색 불이 켜지는 걸 목격하셨다면 즉시 경비실로 도망치십시오.")) },
		{ EAeternaNotebookPage::Controls, TEXT("Descriptions"), FText::FromString(TEXT("WASD: 이동\n마우스: 시점 조작\nE: 상호작용\nF: 헤드램프 켜기/끄기\nShift: 달리기")), FText::FromString(TEXT("WASD: 이동\n마우스: 시점 조작\nE: 상호작용\nF: 헤드램프 켜기/끄기\nShift: 달리기")), FText::FromString(TEXT("WASD: 이동\n마우스: 시점 조작\nE: 상호작용\nF: 헤드램프 켜기/끄기\nShift: 달리기")) }
	};

	JournalTextVerticalOffsets =
	{
		{ TEXT("QuestName"), 30.0f },
		{ TEXT("QuestCategory"), 0.0f },
		{ TEXT("QuestRegion"), 0.0f },
		{ TEXT("TXT_SelectGuide"), 0.0f },
		{ TEXT("Description"), 0.0f },
		{ TEXT("Descriptions"), 0.0f }
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
	PlayPageTurnSound();
}

void UAeternaNotebookHudComponent::ShowNextNotebookPage()
{
	const int32 CurrentPageIndex = static_cast<int32>(CurrentNotebookPage);
	const int32 NextPageIndex = (CurrentPageIndex + 1) % 3;
	SetNotebookPage(static_cast<EAeternaNotebookPage>(NextPageIndex));
	PlayPageTurnSound();
}

void UAeternaNotebookHudComponent::ShowPreviousNotebookDay()
{
	const int32 PreviousDayIndex = FMath::Max(0, CurrentNotebookDayIndex - 1);
	if (PreviousDayIndex != CurrentNotebookDayIndex)
	{
		CurrentNotebookDayIndex = PreviousDayIndex;
		ApplyNotebookJournalText();
		PlayPageTurnSound();
	}
}

void UAeternaNotebookHudComponent::ShowNextNotebookDay()
{
	const int32 NextDayIndex = FMath::Min(2, CurrentNotebookDayIndex + 1);
	if (NextDayIndex != CurrentNotebookDayIndex)
	{
		CurrentNotebookDayIndex = NextDayIndex;
		ApplyNotebookJournalText();
		PlayPageTurnSound();
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
	ApplyNotebookTextOffsets();

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

void UAeternaNotebookHudComponent::ApplyNotebookTextOffsets()
{
	for (const TPair<FName, float>& TextOffset : JournalTextVerticalOffsets)
	{
		SetNamedWidgetRenderTranslation(TextOffset.Key, FVector2D(0.0f, TextOffset.Value));
	}
}

void UAeternaNotebookHudComponent::PlayPageTurnSound()
{
	if (!CachedPageTurnSound)
	{
		CachedPageTurnSound = ResolvePageTurnSound();
	}

	if (!CachedPageTurnSound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, CachedPageTurnSound, PageTurnVolume);
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

bool UAeternaNotebookHudComponent::SetNamedWidgetRenderTranslation(FName WidgetName, FVector2D Translation)
{
	if (WidgetName.IsNone() || !NotebookJournalWidget || !NotebookJournalWidget->WidgetTree)
	{
		return false;
	}

	if (UWidget* Widget = NotebookJournalWidget->WidgetTree->FindWidget(WidgetName))
	{
		Widget->SetRenderTranslation(Translation);
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

USoundBase* UAeternaNotebookHudComponent::ResolvePageTurnSound() const
{
	if (PageTurnSound)
	{
		return PageTurnSound;
	}

	const TCHAR* SoundPaths[] =
	{
		TEXT("/Game/Resource/Audio/Page_turn_1.Page_turn_1"),
		TEXT("/Game/Resource/Audio/Page_turn_1")
	};

	for (const TCHAR* SoundPath : SoundPaths)
	{
		if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, SoundPath))
		{
			return Sound;
		}
	}

	return nullptr;
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
