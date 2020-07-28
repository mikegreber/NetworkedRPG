// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NCombatComponentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNCombatComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Characters with a UNCombatComponent must implement this interface.
 */
class NETWORKEDRPG_API INCombatComponentInterface
{
	GENERATED_BODY()

public:
	virtual class UNMovementSystemComponent* GetMovementSystemComponent() const = 0;
	
	virtual class UNAbilitySystemComponent* GetNAbilitySystemComponent() const = 0;
};
