// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "NTypes.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FCameraMode
{
	GENERATED_USTRUCT_BODY()

	FCameraMode():
		SocketOffset(0.f, 0.f, 80.f),
		CameraLagSpeed(4.0f),
		CameraRotationLagSpeed(20.f),
		CameraLagMaxDistance(200.f)
	{}

	FCameraMode(const FVector& SocketOffset, const float CameraLagSpeed, const float CameraRotationLagSpeed, const float CameraLagMaxDistance):
		SocketOffset(SocketOffset),
		CameraLagSpeed(CameraLagSpeed),
		CameraRotationLagSpeed(CameraRotationLagSpeed),
		CameraLagMaxDistance(CameraLagMaxDistance)
	{}

	/** The camera relative offset for the camera state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SocketOffset;

	/** Controls how quickly camera reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float CameraLagSpeed;

	/** Controls how quickly camera rotation reaches target position. Low values are slower (more lag), high values are faster (less lag), while zero is instant (no lag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float CameraRotationLagSpeed;

	/** Controls how the greatest distance away from the target distance the camera lag can be. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraLagMaxDistance;	
};

USTRUCT(BlueprintType)
struct FCameraModeSettings
{
	GENERATED_USTRUCT_BODY()

	FCameraModeSettings(){}

	explicit FCameraModeSettings(const FCameraMode& Initial)
		: Walking(Initial),
		  Running(Initial),
		  Sprinting(Initial)
	{}

	FCameraModeSettings(const FCameraMode& Walking, const FCameraMode& Running,
		const FCameraMode& Sprinting)
		: Walking(Walking),
		  Running(Running),
		  Sprinting(Sprinting)
	{}

	/** The camera offset when walking */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FCameraMode Walking;

	/** The camera offset when running */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FCameraMode Running;

	/** The camera offset when sprinting */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FCameraMode Sprinting;

	/** Returns the appropriate CameraMode for the input Gait. */
	FCameraMode GetCurrent(ENMovementGait Gait) const
	{
		switch (Gait)
		{
		case ENMovementGait::Walking:
			return Walking;
		case ENMovementGait::Running:
			return Running;
		case ENMovementGait::Sprinting:
			return Sprinting;
		}

		return Walking;
	}
};
