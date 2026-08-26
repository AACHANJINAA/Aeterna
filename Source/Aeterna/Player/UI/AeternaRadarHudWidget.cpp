// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/UI/AeternaRadarHudWidget.h"

#include "Rendering/DrawElements.h"
#include "Widgets/Layout/SBox.h"

TSharedRef<SWidget> UAeternaRadarHudWidget::RebuildWidget()
{
	return SNew(SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y);
}

int32 UAeternaRadarHudWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 BaseLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const FVector2D Center = LocalSize * 0.5f;
	const float MaxHorizontalRadius = LocalSize.X * 0.5f - 6.0f;
	const float MaxVerticalRadius = LocalSize.Y * 0.5f - 6.0f;
	const float PaintRadius = FMath::Max(8.0f, FMath::Min(RadarRadius, FMath::Min(MaxHorizontalRadius, MaxVerticalRadius)));

	const FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry();

	DrawCircle(OutDrawElements, BaseLayer + 1, AllottedGeometry, Center, PaintRadius, OuterRingColor, OuterRingThickness);
	for (int32 RingIndex = 1; RingIndex <= 6; ++RingIndex)
	{
		DrawCircle(OutDrawElements, BaseLayer + 1, AllottedGeometry, Center, PaintRadius * (static_cast<float>(RingIndex) / 7.0f), RingColor, RingThickness);
	}

	for (int32 TickIndex = 0; TickIndex < 36; ++TickIndex)
	{
		const float TickRadians = FMath::DegreesToRadians(static_cast<float>(TickIndex) * 10.0f - 90.0f);
		const FVector2D Direction(FMath::Cos(TickRadians), FMath::Sin(TickRadians));
		const bool bMajorTick = (TickIndex % 3) == 0;
		const float TickLength = bMajorTick ? 9.0f : 5.0f;

		TArray<FVector2D> TickLine;
		TickLine.Add(Center + Direction * (PaintRadius + 3.0f));
		TickLine.Add(Center + Direction * (PaintRadius + 3.0f + TickLength));
		FSlateDrawElement::MakeLines(OutDrawElements, BaseLayer + 1, PaintGeometry, TickLine, ESlateDrawEffect::None, OuterRingColor.CopyWithNewOpacity(bMajorTick ? 0.75f : 0.38f), true, bMajorTick ? 1.5f : 1.0f);
	}

	TArray<FVector2D> CrossLine;
	CrossLine.Add(Center + FVector2D(-PaintRadius, 0.0f));
	CrossLine.Add(Center + FVector2D(PaintRadius, 0.0f));
	FSlateDrawElement::MakeLines(OutDrawElements, BaseLayer + 1, PaintGeometry, CrossLine, ESlateDrawEffect::None, RingColor.CopyWithNewOpacity(0.22f), true, 1.0f);

	CrossLine.Reset();
	CrossLine.Add(Center + FVector2D(0.0f, -PaintRadius));
	CrossLine.Add(Center + FVector2D(0.0f, PaintRadius));
	FSlateDrawElement::MakeLines(OutDrawElements, BaseLayer + 1, PaintGeometry, CrossLine, ESlateDrawEffect::None, RingColor.CopyWithNewOpacity(0.22f), true, 1.0f);

	const float SweepRadians = FMath::DegreesToRadians(SweepAngleDegrees);
	for (int32 TrailIndex = 0; TrailIndex < 11; ++TrailIndex)
	{
		const float TrailAlpha = FMath::Lerp(0.30f, 0.02f, static_cast<float>(TrailIndex) / 10.0f);
		const float TrailRadians = SweepRadians - FMath::DegreesToRadians(static_cast<float>(TrailIndex) * 4.0f);
		const FVector2D Direction(FMath::Cos(TrailRadians), FMath::Sin(TrailRadians));

		TArray<FVector2D> SweepLine;
		SweepLine.Add(Center);
		SweepLine.Add(Center + Direction * PaintRadius);
		FSlateDrawElement::MakeLines(OutDrawElements, BaseLayer + 2, PaintGeometry, SweepLine, ESlateDrawEffect::None, SweepColor.CopyWithNewOpacity(TrailAlpha), true, 3.0f);
	}

	TArray<FVector2D> SweepArc;
	BuildArcPoints(Center, PaintRadius * 0.98f, SweepRadians - FMath::DegreesToRadians(34.0f), SweepRadians, 16, SweepArc);
	FSlateDrawElement::MakeLines(OutDrawElements, BaseLayer + 2, PaintGeometry, SweepArc, ESlateDrawEffect::None, SweepColor.CopyWithNewOpacity(0.55f), true, 2.0f);

	DrawCircle(OutDrawElements, BaseLayer + 4, AllottedGeometry, Center, 5.0f, PlayerColor, 2.0f);
	DrawCircle(OutDrawElements, BaseLayer + 4, AllottedGeometry, Center, 2.0f, PlayerColor, 2.0f);

	for (const FAeternaRadarBlip& Blip : QuestBlips)
	{
		const FVector2D BlipPosition = Center + Blip.Position * PaintRadius;
		const float Pulse = 0.5f + 0.5f * FMath::Sin(FPlatformTime::Seconds() * 6.0);
		DrawCircle(OutDrawElements, BaseLayer + 5, AllottedGeometry, BlipPosition, BlipRadius + Pulse * 2.0f, QuestColor.CopyWithNewOpacity(Blip.bClampedToEdge ? 0.95f : 0.8f), 2.0f);
		DrawCircle(OutDrawElements, BaseLayer + 5, AllottedGeometry, BlipPosition, BlipRadius * 0.55f, QuestColor, 2.0f);
	}

	TArray<FVector2D> NorthTick;
	NorthTick.Add(Center + FVector2D(0.0f, -PaintRadius - 4.0f));
	NorthTick.Add(Center + FVector2D(0.0f, -PaintRadius + 8.0f));
	FSlateDrawElement::MakeLines(OutDrawElements, BaseLayer + 6, PaintGeometry, NorthTick, ESlateDrawEffect::None, OuterRingColor, true, 2.0f);

	return BaseLayer + 6;
}

void UAeternaRadarHudWidget::SetRadarState(float InSweepAngleDegrees, const TArray<FAeternaRadarBlip>& InQuestBlips)
{
	SweepAngleDegrees = FMath::Fmod(InSweepAngleDegrees, 360.0f);
	if (SweepAngleDegrees < 0.0f)
	{
		SweepAngleDegrees += 360.0f;
	}
	QuestBlips = InQuestBlips;
	InvalidateLayoutAndVolatility();
}

void UAeternaRadarHudWidget::BuildCirclePoints(const FVector2D& Center, float Radius, int32 Segments, TArray<FVector2D>& OutPoints)
{
	const int32 SafeSegments = FMath::Max(8, Segments);
	OutPoints.Reset(SafeSegments + 1);
	for (int32 Index = 0; Index <= SafeSegments; ++Index)
	{
		const float Angle = (static_cast<float>(Index) / static_cast<float>(SafeSegments)) * UE_TWO_PI;
		OutPoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
	}
}

void UAeternaRadarHudWidget::BuildArcPoints(const FVector2D& Center, float Radius, float StartAngleRadians, float EndAngleRadians, int32 Segments, TArray<FVector2D>& OutPoints)
{
	const int32 SafeSegments = FMath::Max(2, Segments);
	OutPoints.Reset(SafeSegments + 1);
	for (int32 Index = 0; Index <= SafeSegments; ++Index)
	{
		const float Alpha = static_cast<float>(Index) / static_cast<float>(SafeSegments);
		const float Angle = FMath::Lerp(StartAngleRadians, EndAngleRadians, Alpha);
		OutPoints.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
	}
}

void UAeternaRadarHudWidget::DrawCircle(FSlateWindowElementList& OutDrawElements, int32 LayerId, const FGeometry& Geometry, const FVector2D& Center, float Radius, const FLinearColor& Color, float Thickness) const
{
	TArray<FVector2D> CirclePoints;
	BuildCirclePoints(Center, Radius, CircleSegments, CirclePoints);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, Geometry.ToPaintGeometry(), CirclePoints, ESlateDrawEffect::None, Color, true, Thickness);
}
