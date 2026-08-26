// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AeternaRadarHudWidget.generated.h"

class SWidget;

USTRUCT(BlueprintType)
struct FAeternaRadarBlip
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Radar")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Radar")
	bool bClampedToEdge = false;
};

UCLASS()
class AETERNA_API UAeternaRadarHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UFUNCTION(BlueprintCallable, Category="Radar")
	void SetRadarState(float InSweepAngleDegrees, const TArray<FAeternaRadarBlip>& InQuestBlips);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Layout")
	FVector2D WidgetSize = FVector2D(184.0f, 184.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Layout", meta=(ClampMin="8.0"))
	float RadarRadius = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Layout", meta=(ClampMin="1"))
	int32 CircleSegments = 80;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor PanelColor = FLinearColor(0.015f, 0.06f, 0.055f, 0.26f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor RingColor = FLinearColor(0.15f, 1.0f, 0.78f, 0.50f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor OuterRingColor = FLinearColor(0.15f, 1.0f, 0.78f, 0.82f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor AccentColor = FLinearColor(0.82f, 1.0f, 0.94f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor SweepColor = FLinearColor(0.42f, 1.0f, 0.82f, 0.32f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor PlayerColor = FLinearColor(0.92f, 0.98f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style")
	FLinearColor QuestColor = FLinearColor(0.42f, 1.0f, 0.82f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style", meta=(ClampMin="0.0"))
	float RingThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style", meta=(ClampMin="0.0"))
	float OuterRingThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Radar|Style", meta=(ClampMin="0.0"))
	float BlipRadius = 4.0f;

private:
	static void BuildCirclePoints(const FVector2D& Center, float Radius, int32 Segments, TArray<FVector2D>& OutPoints);
	static void BuildArcPoints(const FVector2D& Center, float Radius, float StartAngleRadians, float EndAngleRadians, int32 Segments, TArray<FVector2D>& OutPoints);
	void DrawCircle(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& Geometry, const FVector2D& Center, float Radius, const FLinearColor& Color, float Thickness) const;

	float SweepAngleDegrees = 0.0f;
	TArray<FAeternaRadarBlip> QuestBlips;
};
