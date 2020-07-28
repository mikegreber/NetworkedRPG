// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NGameplayAbilityActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNGameplayAbilityActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETWORKEDRPG_API INGameplayAbilityActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** The component that we will do line traces from (camera for player controlled actors) */
	virtual class USceneComponent* GetTraceStartComponent() const = 0;
};
