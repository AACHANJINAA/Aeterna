// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioVariantSubsystem.h"

#include "Aeterna.h"
#include "Core/ScenarioManagerSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

void UScenarioVariantSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UScenarioManagerSubsystem::StaticClass());
	Super::Initialize(Collection);

	if (UScenarioManagerSubsystem* ScenarioManager = GetWorld() ? GetWorld()->GetSubsystem<UScenarioManagerSubsystem>() : nullptr)
	{
		BoundScenarioManager = ScenarioManager;
		ScenarioManager->OnScenarioStarted.AddUniqueDynamic(this, &UScenarioVariantSubsystem::HandleScenarioStarted);
	}
}

void UScenarioVariantSubsystem::Deinitialize()
{
	if (UScenarioManagerSubsystem* ScenarioManager = BoundScenarioManager.Get())
	{
		ScenarioManager->OnScenarioStarted.RemoveDynamic(this, &UScenarioVariantSubsystem::HandleScenarioStarted);
	}
	BoundScenarioManager.Reset();

	Super::Deinitialize();
}

FName UScenarioVariantSubsystem::MakeScenarioTag(FName ScenarioId)
{
	if (ScenarioId.IsNone())
	{
		return NAME_None;
	}

	return FName(*(FString(GetScenarioTagPrefix()) + ScenarioId.ToString()));
}

void UScenarioVariantSubsystem::ApplyScenarioToTaggedActors(FName ScenarioId)
{
	UWorld* World = GetWorld();
	if (!World || ScenarioId.IsNone())
	{
		return;
	}

	const FString TagPrefix = GetScenarioTagPrefix();
	const FName ActiveTag = MakeScenarioTag(ScenarioId);

	int32 ShownCount = 0;
	int32 HiddenCount = 0;

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor)
		{
			continue;
		}

		bool bActiveInScenario = false;
		int32 ScenarioTagCount = 0;

		for (const FName& Tag : Actor->Tags)
		{
			if (!Tag.ToString().StartsWith(TagPrefix, ESearchCase::CaseSensitive))
			{
				continue;
			}

			++ScenarioTagCount;
			bActiveInScenario |= (Tag == ActiveTag);
		}

		if (ScenarioTagCount == 0)
		{
			continue;
		}

		Actor->SetActorHiddenInGame(!bActiveInScenario);
		Actor->SetActorEnableCollision(bActiveInScenario);

		if (bActiveInScenario)
		{
			++ShownCount;
		}
		else
		{
			++HiddenCount;
		}

		// 복제하면 원본 태그가 따라오므로, 밤 태그가 여러 개 붙은 액터는 짚어줍니다.
		// 여러 밤에 걸쳐 존재해야 하는 액터라면 정상이지만, 대부분은 지우는 걸 잊은 경우입니다.
		UE_LOG(LogAeterna, Verbose,
			TEXT("[ScenarioVariant] %s : %s (밤 태그 %d개)"),
			*GetNameSafe(Actor),
			bActiveInScenario ? TEXT("표시") : TEXT("숨김"),
			ScenarioTagCount);
	}

	UE_LOG(LogAeterna, Log,
		TEXT("[ScenarioVariant] 밤 '%s' 적용 완료 — 표시 %d개 / 숨김 %d개"),
		*ScenarioId.ToString(), ShownCount, HiddenCount);
}

void UScenarioVariantSubsystem::HandleScenarioStarted(FName ScenarioId)
{
	ApplyScenarioToTaggedActors(ScenarioId);
}
