// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

namespace AeternaHudLayout
{
	inline float GetViewportScale(const FVector2D& ViewportSize, const FVector2D& ReferenceViewportSize)
	{
		if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f || ReferenceViewportSize.X <= 0.0f || ReferenceViewportSize.Y <= 0.0f)
		{
			return 1.0f;
		}

		return FMath::Min(ViewportSize.X / ReferenceViewportSize.X, ViewportSize.Y / ReferenceViewportSize.Y);
	}

	inline FVector2D GetAnchoredPosition(const FVector2D& ViewportSize, const FVector2D& AnchorNormalized, const FVector2D& BaseOffset, const FVector2D& BaseWidgetSize, float Scale)
	{
		const FVector2D ScaledOffset = BaseOffset * Scale;
		const FVector2D ScaledWidgetSize = BaseWidgetSize * Scale;

		return FVector2D(
			AnchorNormalized.X >= 0.5f ? ViewportSize.X - ScaledWidgetSize.X - ScaledOffset.X : ScaledOffset.X,
			AnchorNormalized.Y >= 0.5f ? ViewportSize.Y - ScaledWidgetSize.Y - ScaledOffset.Y : ScaledOffset.Y);
	}

	inline FVector2D GetTopCenterPosition(const FVector2D& ViewportSize, const FVector2D& BaseOffset, const FVector2D& BaseWidgetSize, float Scale)
	{
		const FVector2D ScaledOffset = BaseOffset * Scale;
		const FVector2D ScaledWidgetSize = BaseWidgetSize * Scale;
		return FVector2D((ViewportSize.X - ScaledWidgetSize.X) * 0.5f + ScaledOffset.X, ScaledOffset.Y);
	}

	inline void ApplyScaledViewportLayout(UUserWidget* Widget, const FVector2D& Position, const FVector2D& BaseWidgetSize, float Scale)
	{
		if (!Widget)
		{
			return;
		}

		Widget->SetDesiredSizeInViewport(BaseWidgetSize);
		Widget->SetRenderScale(FVector2D(Scale, Scale));
		Widget->SetRenderTransformPivot(FVector2D::ZeroVector);
		Widget->SetAlignmentInViewport(FVector2D::ZeroVector);
		Widget->SetPositionInViewport(Position, true);
	}
}
