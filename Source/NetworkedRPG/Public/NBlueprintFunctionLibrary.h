// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category="NetworkedRPG|AbilitySystem")
	static float GetMaxAttributeUpdatesPerSecond() { return MAX_ATTRIBUTE_UPDATES_PER_SECOND; }

	UFUNCTION(BlueprintCallable, Category="NetworkedRPG|AbilitySystem")
	static float GetTimeBetweenAttributeUpdates() { return 1.0f / MAX_ATTRIBUTE_UPDATES_PER_SECOND; }
	
};
