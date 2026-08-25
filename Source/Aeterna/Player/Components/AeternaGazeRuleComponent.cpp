// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/AeternaGazeRuleComponent.h"

#include "Aeterna.h"
#include "Camera/CameraComponent.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Player/AeternaCharacter.h"

UAeternaGazeRuleComponent::UAeternaGazeRuleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAeternaGazeRuleComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheEyeActors();
	ResetEyeActors();

	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		BoundScenarioManager = ScenarioManager;
		ScenarioManager->OnScenarioStarted.AddUniqueDynamic(this, &UAeternaGazeRuleComponent::HandleScenarioStarted);

		// 스타터보다 늦게 BeginPlay가 돌아 시작 브로드캐스트를 놓쳤을 수 있습니다.
		if (ScenarioManager->HasCurrentScenario())
		{
			HandleScenarioStarted(ScenarioManager->GetCurrentScenarioId());
		}
	}
}

void UAeternaGazeRuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioStarted.RemoveDynamic(this, &UAeternaGazeRuleComponent::HandleScenarioStarted);
	}
	BoundScenarioManager.Reset();

	Super::EndPlay(EndPlayReason);
}

void UAeternaGazeRuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 위반이 발동한 뒤에는 나타나는 연출만 진행합니다.
	if (RevealingEyeActor)
	{
		UpdateReveal(DeltaTime);
		return;
	}

	if (!bRuleActive)
	{
		return;
	}

	const AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();

	// 헤드램프가 꺼져 있으면 비위반입니다. 빛이 곧 시선입니다 (SPEC §6-1 확정).
	if (!AeternaCharacter || !AeternaCharacter->IsHeadlampOn())
	{
		GazeSeconds = 0.0f;
		GazedEyeActor = nullptr;
		return;
	}

	AActor* EyeActor = FindGazedEyeActor();
	if (!EyeActor)
	{
		GazeSeconds = 0.0f;
		GazedEyeActor = nullptr;
		return;
	}

	// 대상이 바뀌면 누적을 이어가지 않습니다.
	if (GazedEyeActor != EyeActor)
	{
		GazedEyeActor = EyeActor;
		GazeSeconds = 0.0f;
	}

	GazeSeconds += DeltaTime;
	if (GazeSeconds >= GazeDurationSeconds)
	{
		TriggerViolation(EyeActor);
	}
}

void UAeternaGazeRuleComponent::ResetEyeActors()
{
	for (int32 Index = 0; Index < EyeActors.Num(); ++Index)
	{
		AActor* EyeActor = EyeActors[Index];
		if (!EyeActor)
		{
			continue;
		}

		EyeActor->SetActorHiddenInGame(true);
		EyeActor->SetActorEnableCollision(false);

		if (EyeAuthoredScales.IsValidIndex(Index))
		{
			EyeActor->SetActorScale3D(EyeAuthoredScales[Index]);
		}
	}

	RevealingEyeActor = nullptr;
	GazedEyeActor = nullptr;
	GazeSeconds = 0.0f;
	RevealElapsedSeconds = 0.0f;
}

float UAeternaGazeRuleComponent::GetGazeProgress() const
{
	return (GazeDurationSeconds > KINDA_SMALL_NUMBER) ? FMath::Clamp(GazeSeconds / GazeDurationSeconds, 0.0f, 1.0f) : 0.0f;
}

void UAeternaGazeRuleComponent::HandleScenarioStarted(FName ScenarioId)
{
	bRuleActive = (ScenarioId == RuleScenarioId);
	ResetEyeActors();
}

void UAeternaGazeRuleComponent::CacheEyeActors()
{
	EyeActors.Reset();
	EyeAuthoredScales.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FName EyeTag(GetEyeTag());
	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (Actor && Actor->Tags.Contains(EyeTag))
		{
			EyeActors.Add(Actor);
			EyeAuthoredScales.Add(Actor->GetActorScale3D());
		}
	}

	if (EyeActors.Num() == 0)
	{
		UE_LOG(LogAeterna, Warning, TEXT("[Gaze] Eye 태그가 붙은 액터가 없습니다. 눈구멍 규칙이 발동하지 않습니다."));
	}
}

AActor* UAeternaGazeRuleComponent::FindGazedEyeActor() const
{
	for (AActor* EyeActor : EyeActors)
	{
		if (IsGazingAtEye(EyeActor))
		{
			return EyeActor;
		}
	}

	return nullptr;
}

bool UAeternaGazeRuleComponent::IsGazingAtEye(const AActor* EyeActor) const
{
	const UCameraComponent* CameraComponent = GetFirstPersonCamera();
	AAeternaCharacter* AeternaCharacter = GetAeternaCharacter();
	UWorld* World = GetWorld();
	if (!EyeActor || !CameraComponent || !AeternaCharacter || !World)
	{
		return false;
	}

	const FVector CameraLocation = CameraComponent->GetComponentLocation();
	const FVector EyeLocation = EyeActor->GetActorLocation();
	const FVector ToEye = EyeLocation - CameraLocation;

	const float DistanceToEye = ToEye.Size();
	if (DistanceToEye > MaxGazeDistance || DistanceToEye <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// 시야 중앙 콘 판정.
	const float CosAngle = FVector::DotProduct(ToEye / DistanceToEye, CameraComponent->GetForwardVector());
	if (CosAngle < FMath::Cos(FMath::DegreesToRadians(CenterConeHalfAngle)))
	{
		return false;
	}

	// 가림 판정. 눈구멍은 뚫려 있으므로 복합 콜리전으로 봅니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AeternaGazeOcclusion), true, AeternaCharacter);
	QueryParams.AddIgnoredActor(EyeActor);

	FHitResult HitResult;
	if (!World->LineTraceSingleByChannel(HitResult, CameraLocation, EyeLocation, ECC_Visibility, QueryParams))
	{
		return true;
	}

	// 눈구멍 바로 앞까지 도달했으면 가린 것으로 보지 않습니다 (두개골 안쪽 면 등).
	return HitResult.Distance >= DistanceToEye - 20.0f;
}

void UAeternaGazeRuleComponent::TriggerViolation(AActor* EyeActor)
{
	RevealingEyeActor = EyeActor;
	RevealElapsedSeconds = 0.0f;
	GazedEyeActor = nullptr;
	GazeSeconds = 0.0f;

	// 크기 0에서 시작해 커지며 나타납니다.
	EyeActor->SetActorHiddenInGame(false);
	EyeActor->SetActorScale3D(FVector::ZeroVector);

	UE_LOG(LogAeterna, Log, TEXT("[Gaze] 눈구멍 응시 위반: %s"), *GetNameSafe(EyeActor));

	if (AAeternaCharacter* AeternaCharacter = GetAeternaCharacter())
	{
		AeternaCharacter->BP_GazeRuleViolated(EyeActor);
	}

	// 실패 처리와 재시작 흐름은 ScenarioManager와 시나리오 스타터에 맡깁니다.
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->FailCurrentScenarioByRuleViolation();
	}

	bRuleActive = false;
}

void UAeternaGazeRuleComponent::UpdateReveal(float DeltaTime)
{
	const int32 EyeIndex = EyeActors.IndexOfByKey(RevealingEyeActor);
	const FVector TargetScale = EyeAuthoredScales.IsValidIndex(EyeIndex) ? EyeAuthoredScales[EyeIndex] : FVector::OneVector;

	RevealElapsedSeconds += DeltaTime;

	const float Progress = (RevealSeconds > KINDA_SMALL_NUMBER)
		? FMath::Clamp(RevealElapsedSeconds / RevealSeconds, 0.0f, 1.0f)
		: 1.0f;

	RevealingEyeActor->SetActorScale3D(TargetScale * Progress);

	if (Progress >= 1.0f)
	{
		// 다 나타났으면 이 컴포넌트가 할 일은 끝입니다. 페이드와 재시작은 스타터가 이어받습니다.
		RevealingEyeActor = nullptr;
	}
}
