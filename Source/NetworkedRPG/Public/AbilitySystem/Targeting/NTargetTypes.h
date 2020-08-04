// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "NTargetTypes.generated.h"

struct FGameplayEventData;
class ANCharacterBase;
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNTargetType : public UObject
{
	GENERATED_BODY()

public:
	UNTargetType() {}

	// Determine which targets to apply gameplay effects to.
	virtual void GetTargets(ANCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

/** Trivial target type that uses the owner */
UCLASS(NotBlueprintable)
class NETWORKEDRPG_API UNTargetType_UseOwner : public UNTargetType
{
	GENERATED_BODY()

public:
	UNTargetType_UseOwner() {}

	/** Uses the passed in event data */
	virtual void GetTargets(ANCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};


UCLASS(NotBlueprintable)
class NETWORKEDRPG_API UNTargetType_UseEventData : public UNTargetType
{
	GENERATED_BODY()

public:
	UNTargetType_UseEventData() {}

	/** Uses the passed in event data */
	virtual void GetTargets(ANCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};