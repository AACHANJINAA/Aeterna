// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ScenarioVariantSubsystem.h"

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

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (!Actor)
		{
			continue;
		}

		bool bManagedByTag = false;
		bool bActiveInScenario = false;

		for (const FName& Tag : Actor->Tags)
		{
			if (!Tag.ToString().StartsWith(TagPrefix, ESearchCase::CaseSensitive))
			{
				continue;
			}

			bManagedByTag = true;
			if (Tag == ActiveTag)
			{
				bActiveInScenario = true;
				break;
			}
		}

		if (!bManagedByTag)
		{
			continue;
		}

		Actor->SetActorHiddenInGame(!bActiveInScenario);
		Actor->SetActorEnableCollision(bActiveInScenario);
	}
}

void UScenarioVariantSubsystem::HandleScenarioStarted(FName ScenarioId)
{
	ApplyScenarioToTaggedActors(ScenarioId);
}
