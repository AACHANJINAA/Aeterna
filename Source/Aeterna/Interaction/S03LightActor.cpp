// Copyright Epic Games, Inc. All Rights Reserved.

#include "Interaction/S03LightActor.h"

#include "Aeterna.h"
#include "Components/BoxComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Core/GameClockSubsystem.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/World.h"

AS03LightActor::AS03LightActor()
{
	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	SetRootComponent(InteractionBounds);
	InteractionBounds->SetBoxExtent(FVector(25.0f, 25.0f, 25.0f));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	LightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	LightComponent->SetupAttachment(InteractionBounds);
	ApplyLightDefaults();
	LightComponent->SetVisibility(false);

	// 소등은 스캔·충전 어느 쪽도 아닙니다. 프롬프트 문구로 무엇인지 알립니다.
	InteractionType = EAeternaInteractionType::Custom;
	InteractionPromptText = FText::FromString(TEXT("소등"));
	bRepeatable = false;

	// 끈 조명 하나가 목표 1개입니다. ScanPointId를 비워두면 액터 이름이
	// 그대로 ID가 되므로 배치한 조명마다 따로 집계됩니다.
	bCountsAsScanPoint = true;
	ScanPointId = NAME_None;

	ActiveScenarioIds.Reset();
	ActiveScenarioIds.Add(TEXT("S03_ForbiddenLight"));
}

void AS03LightActor::PostLoad()
{
	Super::PostLoad();

	if (FMath::IsNearlyEqual(ActiveLightIntensity, 5500.0f))
	{
		ActiveLightIntensity = 9000.0f;
	}
	if (FMath::IsNearlyEqual(ActiveLightAttenuationRadius, 1200.0f))
	{
		ActiveLightAttenuationRadius = 1600.0f;
	}
	if (FMath::IsNearlyEqual(ActiveMeshEmissiveStrength, 8.0f))
	{
		ActiveMeshEmissiveStrength = 30.0f;
	}
}

void AS03LightActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameClockSubsystem* GameClock = GetWorld() ? GetWorld()->GetSubsystem<UGameClockSubsystem>() : nullptr)
	{
		BoundGameClock = GameClock;
		GameClock->OnClockMinuteChanged.AddUniqueDynamic(this, &AS03LightActor::HandleClockMinuteChanged);
	}

	// 밤 시작 브로드캐스트(ResetInteraction)가 켤 때까지 꺼둡니다.
	// 여기서 켜면 밤1·밤2에서도 불이 들어옵니다.
	ApplyLightDefaults();
	SetLightOn(false);
	ApplyScenarioVisibility();
}

void AS03LightActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameClockSubsystem* GameClock = BoundGameClock.Get())
	{
		GameClock->OnClockMinuteChanged.RemoveDynamic(this, &AS03LightActor::HandleClockMinuteChanged);
	}
	BoundGameClock.Reset();

	Super::EndPlay(EndPlayReason);
}

void AS03LightActor::HandleClockMinuteChanged(int32 ClockMinutes)
{
	// 이미 껐다면 스케줄이 다시 켜지 않습니다. 재점등은 별도 연출입니다.
	if (bLightOn || IsInteractionCompleted() || TurnOnClockMinutes <= 0)
	{
		return;
	}

	if (!IsActiveInCurrentScenario())
	{
		ApplyScenarioVisibility();
		return;
	}

	if (ClockMinutes >= TurnOnClockMinutes)
	{
		SetLightOn(true);
		UE_LOG(LogAeterna, Log, TEXT("[S03] 비정상 점등 감지: %s"), *GetZoneLogName());
	}
}

void AS03LightActor::SetLightOn(bool bOn)
{
	if (bLightOn == bOn)
	{
		ApplyLightDefaults();
		ApplyMeshEmissive();
		return;
	}

	bLightOn = bOn;
	ApplyLightDefaults();

	if (LightComponent)
	{
		LightComponent->SetVisibility(bLightOn);
	}

	if (bLightOn)
	{
		TurnOnWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}

	ApplyMeshEmissive();
	BP_LightStateChanged(bLightOn);
}

bool AS03LightActor::CanInteract_Implementation(AActor* Interactor) const
{
	// 꺼진 불은 끌 수 없습니다.
	if (!bLightOn)
	{
		return false;
	}

	// 점등 직후 잠깐은 입력을 받지 않습니다. 켜지는 순간 눌린 E를 소등으로
	// 세면 플레이어가 누른 적 없는 조작이 성립합니다.
	if (InteractionLockAfterTurnOnSeconds > 0.0f && GetWorld())
	{
		if (GetWorld()->GetTimeSeconds() - TurnOnWorldTimeSeconds < InteractionLockAfterTurnOnSeconds)
		{
			return false;
		}
	}

	return Super::CanInteract_Implementation(Interactor);
}

void AS03LightActor::OnInteractionPerformed(AActor* Interactor)
{
	Super::OnInteractionPerformed(Interactor);

	SetLightOn(false);
	UE_LOG(LogAeterna, Log, TEXT("[S03] 소등 확인: %s"), *GetZoneLogName());
}

void AS03LightActor::ResetInteraction()
{
	Super::ResetInteraction();

	// 자기 밤에서만 켭니다. 다른 밤이 시작될 때는 꺼진 채로 남습니다.
	SetLightOn(IsActiveInCurrentScenario() && TurnOnClockMinutes <= 0);
	ApplyScenarioVisibility();
}

bool AS03LightActor::IsActiveInCurrentScenario() const
{
	const UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr;
	const FName CurrentScenarioId = ScenarioManager ? ScenarioManager->GetCurrentScenarioId() : NAME_None;

	return IsActiveForScenario(CurrentScenarioId);
}

void AS03LightActor::ApplyScenarioVisibility()
{
	const bool bActiveInScenario = IsActiveInCurrentScenario();
	SetActorHiddenInGame(!bActiveInScenario);
	SetActorEnableCollision(bActiveInScenario);

	if (!bActiveInScenario)
	{
		SetLightOn(false);
	}
}

void AS03LightActor::ApplyLightDefaults()
{
	if (!LightComponent)
	{
		return;
	}

	LightComponent->SetLightColor(ActiveLightColor);
	LightComponent->SetIntensity(ActiveLightIntensity);
	LightComponent->SetAttenuationRadius(ActiveLightAttenuationRadius);
}

void AS03LightActor::ApplyMeshEmissive()
{
	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents);

	const FLinearColor EmissiveColor = bLightOn ? ActiveLightColor * ActiveMeshEmissiveStrength : FLinearColor::Black;
	const float EmissiveStrength = bLightOn ? ActiveMeshEmissiveStrength : 0.0f;
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		for (const FName& ParameterName : MeshEmissiveColorParameterNames)
		{
			if (!ParameterName.IsNone())
			{
				MeshComponent->SetVectorParameterValueOnMaterials(ParameterName, FVector(EmissiveColor.R, EmissiveColor.G, EmissiveColor.B));
			}
		}

		for (const FName& ParameterName : MeshEmissiveStrengthParameterNames)
		{
			if (!ParameterName.IsNone())
			{
				MeshComponent->SetScalarParameterValueOnMaterials(ParameterName, EmissiveStrength);
			}
		}
	}
}

FString AS03LightActor::GetZoneLogName() const
{
	return ZoneName.IsEmpty() ? GetName() : ZoneName.ToString();
}
