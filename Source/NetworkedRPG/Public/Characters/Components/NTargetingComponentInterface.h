// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "UObject/Interface.h"
#include "NTargetingComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNTargetingComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETWORKEDRPG_API INTargetingComponentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual APlayerController* GetPlayerController() = 0;

	virtual class UNMovementSystemComponent* GetMovementSystemComponent() = 0;

	virtual class UNSpringArmComponent* GetSpringArm() = 0;

	virtual FVector GetEyesLocation() = 0;
};
