// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "UObject/Interface.h"
#include "NInteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETWORKEDRPG_API INInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void Interact(AActor* InActor) = 0;

	virtual FVector GetIndicatorOffset() = 0;

	virtual ENInteractableType GetInteractableType() = 0;

	virtual USceneComponent* GetInteractableRootComponent() = 0;
};
