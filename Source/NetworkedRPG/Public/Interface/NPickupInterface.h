// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "NPickupInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETWORKEDRPG_API INPickupInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual ENPickupType GetPickupType() = 0;
	
	virtual void PickUp(AActor* OwningActor) = 0;

	virtual FVector GetActorLocation() = 0;
};
