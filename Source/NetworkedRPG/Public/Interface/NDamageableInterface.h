// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "UObject/Interface.h"
#include "NDamageableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNDamageableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NETWORKEDRPG_API INDamageableInterface
{
	GENERATED_BODY()

public:

	virtual ENTeam GetTeam() const = 0;
};
