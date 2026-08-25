// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaObjectiveHudComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Player/AeternaCharacter.h"
#include "Player/Components/AeternaScanProgressComponent.h"
#include "Player/UI/AeternaObjectiveHudWidget.h"

UAeternaObjectiveHudComponent::UAeternaObjectiveHudComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;

	FAeternaScenarioObjectiveHudDefinition S01Objective;
	S01Objective.ScenarioId = TEXT("S01_Handover");
	FAeternaScenarioObjectiveHudItem S01DisplayCaseObjective;
	S01DisplayCaseObjective.ItemText = FText::FromString(TEXT("전시 케이스 화석 스캔"));
	S01DisplayCaseObjective.ProgressId = TEXT("S01_Scan_DisplayFossil");
	S01Objective.Items.Add(S01DisplayCaseObjective);

	FAeternaScenarioObjectiveHudItem S01BatteryObjective;
	S01BatteryObjective.ItemText = FText::FromString(TEXT("배터리 회수"));
	S01BatteryObjective.ProgressId = TEXT("S01_Scan_Battery");
	S01Objective.Items.Add(S01BatteryObjective);

	FAeternaScenarioObjectiveHudItem S01TRexSignObjective;
	S01TRexSignObjective.ItemText = FText::FromString(TEXT("티라노 알림판 스캔"));
	S01TRexSignObjective.ProgressId = TEXT("S01_Scan_TRexSign");
	S01Objective.Items.Add(S01TRexSignObjective);
	ScenarioObjectives.Add(S01Objective);

	FAeternaScenarioObjectiveHudDefinition S02Objective;
	S02Objective.ScenarioId = TEXT("S02_GrandHallFossil");
	FAeternaScenarioObjectiveHudItem S02TailObjective;
	S02TailObjective.ItemText = FText::FromString(TEXT("꼬리뼈 제자리 설치"));
	S02Objective.Items.Add(S02TailObjective);

	FAeternaScenarioObjectiveHudItem S02RibObjective;
	S02RibObjective.ItemText = FText::FromString(TEXT("갈비뼈 제자리 설치"));
	S02Objective.Items.Add(S02RibObjective);

	FAeternaScenarioObjectiveHudItem S02ClavicleObjective;
	S02ClavicleObjective.ItemText = FText::FromString(TEXT("쇄골뼈 제자리 설치"));
	S02Objective.Items.Add(S02ClavicleObjective);
	ScenarioObjectives.Add(S02Objective);
}

void UAeternaObjectiveHudComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetAeternaCharacter())
	{
		if (AAeternaCharacter* OwnerCharacter = Cast<AAeternaCharacter>(GetOwner()))
		{
			InitializePlayerComponent(OwnerCharacter);
		}
	}

	BindScenarioManager();
	BindScanProgress();
	CreateObjectiveHudWidget();
	RefreshObjectiveHud();
}

void UAeternaObjectiveHudComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindScanProgress();
	UnbindScenarioManager();

	Super::EndPlay(EndPlayReason);
}

void UAeternaObjectiveHudComponent::InitializePlayerComponent(AAeternaCharacter* InPlayerCharacter)
{
	Super::InitializePlayerComponent(InPlayerCharacter);
	BindScanProgress();
	CreateObjectiveHudWidget();
}

void UAeternaObjectiveHudComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ObjectiveHudWidget)
	{
		CreateObjectiveHudWidget();
	}

	if (ObjectiveHudWidget)
	{
		RefreshObjectiveHud();
		UpdateObjectiveHudPosition();
	}
}

void UAeternaObjectiveHudComponent::RefreshObjectiveHud()
{
	BindScenarioManager();
	BindScanProgress();

	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		CurrentScenarioId = ScenarioManager->GetCurrentScenarioId();
		CurrentRunState = ScenarioManager->GetRunState();
	}

	if (UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get())
	{
		CurrentCount = ScanProgressComponent->GetCompletedScanCount();
		RequiredCount = ScanProgressComponent->GetRequiredScanCount();
	}

	BuildCurrentObjectiveItems();
	CreateObjectiveHudWidget();
	ApplyObjectiveHud();
	UpdateObjectiveHudVisibility();
}

void UAeternaObjectiveHudComponent::HandleScenarioStarted(FName ScenarioId)
{
	CurrentScenarioId = ScenarioId;
	RefreshObjectiveHud();
}

void UAeternaObjectiveHudComponent::HandleScenarioStateChanged(FName ScenarioId, EScenarioRunState RunState)
{
	(void)ScenarioId;
	CurrentRunState = RunState;
	UpdateObjectiveHudVisibility();
}

void UAeternaObjectiveHudComponent::HandleScanProgressChanged(int32 InCurrentCount, int32 InRequiredCount)
{
	CurrentCount = InCurrentCount;
	RequiredCount = InRequiredCount;
	BuildCurrentObjectiveItems();
	ApplyObjectiveHud();
}

void UAeternaObjectiveHudComponent::BindScenarioManager()
{
	if (BoundScenarioManager.IsValid())
	{
		return;
	}

	UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	if (!ScenarioManager)
	{
		return;
	}

	BoundScenarioManager = ScenarioManager;
	ScenarioManager->OnScenarioStarted.AddUniqueDynamic(this, &UAeternaObjectiveHudComponent::HandleScenarioStarted);
	ScenarioManager->OnScenarioStateChanged.AddUniqueDynamic(this, &UAeternaObjectiveHudComponent::HandleScenarioStateChanged);

	if (ScenarioManager->HasCurrentScenario())
	{
		CurrentScenarioId = ScenarioManager->GetCurrentScenarioId();
		CurrentRunState = ScenarioManager->GetRunState();
	}
}

void UAeternaObjectiveHudComponent::UnbindScenarioManager()
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioStarted.RemoveDynamic(this, &UAeternaObjectiveHudComponent::HandleScenarioStarted);
		ScenarioManager->OnScenarioStateChanged.RemoveDynamic(this, &UAeternaObjectiveHudComponent::HandleScenarioStateChanged);
	}

	BoundScenarioManager.Reset();
}

void UAeternaObjectiveHudComponent::BindScanProgress()
{
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	UAeternaScanProgressComponent* ScanProgressComponent = AeternaCharacter ? AeternaCharacter->FindComponentByClass<UAeternaScanProgressComponent>() : nullptr;
	if (!ScanProgressComponent || BoundScanProgressComponent.Get() == ScanProgressComponent)
	{
		return;
	}

	UnbindScanProgress();
	BoundScanProgressComponent = ScanProgressComponent;
	ScanProgressComponent->OnScanProgressChanged.AddUniqueDynamic(this, &UAeternaObjectiveHudComponent::HandleScanProgressChanged);
	CurrentCount = ScanProgressComponent->GetCompletedScanCount();
	RequiredCount = ScanProgressComponent->GetRequiredScanCount();
}

void UAeternaObjectiveHudComponent::UnbindScanProgress()
{
	if (UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get())
	{
		ScanProgressComponent->OnScanProgressChanged.RemoveDynamic(this, &UAeternaObjectiveHudComponent::HandleScanProgressChanged);
	}

	BoundScanProgressComponent.Reset();
}

void UAeternaObjectiveHudComponent::CreateObjectiveHudWidget()
{
	if (ObjectiveHudWidget)
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
		SetComponentTickEnabled(true);
		return;
	}

	if (!ObjectiveWidgetClass)
	{
		return;
	}

	ObjectiveHudWidget = CreateWidget<UAeternaObjectiveHudUserWidget>(PlayerController, ObjectiveWidgetClass);
	if (ObjectiveHudWidget)
	{
		ObjectiveHudWidget->AddToViewport(ViewportZOrder);
		ObjectiveHudWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
		ObjectiveHudWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ObjectiveHudWidget->ForceLayoutPrepass();
		UpdateObjectiveHudPosition();
		ApplyObjectiveHud();
		UpdateObjectiveHudVisibility();
	}
}

void UAeternaObjectiveHudComponent::ApplyObjectiveHud()
{
	UAeternaObjectiveHudUserWidget* ObjectiveHudUserWidget = Cast<UAeternaObjectiveHudUserWidget>(ObjectiveHudWidget);
	if (!ObjectiveHudUserWidget)
	{
		return;
	}

	TArray<FAeternaObjectiveHudEntry> Entries;
	Entries.Reserve(CurrentItemTexts.Num());
	for (int32 Index = 0; Index < CurrentItemTexts.Num(); ++Index)
	{
		FAeternaObjectiveHudEntry Entry;
		Entry.ItemText = CurrentItemTexts[Index];
		Entry.bCompleted = CurrentItemCompleted.IsValidIndex(Index) && CurrentItemCompleted[Index];
		Entry.CurrentCount = Entry.bCompleted ? 1 : 0;
		Entry.RequiredCount = 1;
		Entries.Add(Entry);
	}

	ObjectiveHudUserWidget->SetObjectiveEntries(Entries);
	UpdateObjectiveHudPosition();
}

void UAeternaObjectiveHudComponent::UpdateObjectiveHudPosition()
{
	if (!ObjectiveHudWidget)
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

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	const FVector2D WidgetPosition(
		ViewportAnchorNormalized.X >= 0.5f
			? static_cast<float>(ViewportSizeX) - HudWidgetSize.X - ViewportOffset.X
			: ViewportOffset.X,
		ViewportAnchorNormalized.Y >= 0.5f
			? static_cast<float>(ViewportSizeY) - HudWidgetSize.Y - ViewportOffset.Y
			: ViewportOffset.Y);

	ObjectiveHudWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
	ObjectiveHudWidget->SetPositionInViewport(WidgetPosition, true);
}

void UAeternaObjectiveHudComponent::UpdateObjectiveHudVisibility()
{
	if (!ObjectiveHudWidget)
	{
		return;
	}

	const bool bHasObjective = !CurrentScenarioId.IsNone() && CurrentItemTexts.Num() > 0;
	const bool bActiveState = CurrentRunState == EScenarioRunState::Starting
		|| CurrentRunState == EScenarioRunState::Running
		|| CurrentRunState == EScenarioRunState::Restarting
		|| CurrentRunState == EScenarioRunState::Completed;
	ObjectiveHudWidget->SetVisibility((bHasObjective && bActiveState) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

const FAeternaScenarioObjectiveHudDefinition* UAeternaObjectiveHudComponent::FindObjectiveDefinition(FName ScenarioId) const
{
	for (const FAeternaScenarioObjectiveHudDefinition& ObjectiveDefinition : ScenarioObjectives)
	{
		if (ObjectiveDefinition.ScenarioId == ScenarioId)
		{
			return &ObjectiveDefinition;
		}
	}

	return nullptr;
}

void UAeternaObjectiveHudComponent::BuildCurrentObjectiveItems()
{
	CurrentItemTexts.Reset();
	CurrentItemCompleted.Reset();

	const FAeternaScenarioObjectiveHudDefinition* ObjectiveDefinition = FindObjectiveDefinition(CurrentScenarioId);
	if (!ObjectiveDefinition)
	{
		return;
	}

	const UAeternaScanProgressComponent* ScanProgressComponent = BoundScanProgressComponent.Get();
	for (int32 Index = 0; Index < ObjectiveDefinition->Items.Num(); ++Index)
	{
		const FAeternaScenarioObjectiveHudItem& Item = ObjectiveDefinition->Items[Index];
		FText DisplayText = Item.ItemText;
		if (CurrentScenarioId == TEXT("S02_GrandHallFossil") && DisplayText.ToString().Contains(TEXT("아래턱")))
		{
			DisplayText = FText::FromString(TEXT("쇄골뼈 제자리 설치"));
		}
		CurrentItemTexts.Add(DisplayText);

		const bool bCompleted = !Item.ProgressId.IsNone() && ScanProgressComponent
			? ScanProgressComponent->HasScannedPoint(Item.ProgressId)
			: Index < CurrentCount;
		CurrentItemCompleted.Add(bCompleted);
	}
}
