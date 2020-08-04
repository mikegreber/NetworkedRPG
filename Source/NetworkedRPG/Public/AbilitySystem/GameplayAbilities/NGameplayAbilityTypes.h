// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "NGameplayAbilityTypes.generated.h"


/**
* Struct defining a list of gameplay effects, a tag, and targeting info
* These containers are defined statically in blueprints or assets and then turn into Specs at runtime
*/
class UNGameplayAbility;
class UNTargetType;

USTRUCT(BlueprintType)
struct FNGameplayEffectContainer
{
	GENERATED_BODY()

	FNGameplayEffectContainer() {}

	/** Sets the way targeting happens */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TSubclassOf<UNTargetType> TargetType;

	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;
};

/** A "processed" version of NGameplayEffectContainer that can be passed around and eventually applied */
USTRUCT(BlueprintType)
struct FNGameplayEffectContainerSpec
{
	GENERATED_BODY()

	FNGameplayEffectContainerSpec():
		OwningAbility(nullptr)
	{}

	FNGameplayEffectContainerSpec(UNGameplayAbility* OwningAbility):
		OwningAbility(OwningAbility)
	{}

	/** Computed target data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayAbilityTargetDataHandle TargetData;

	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	/** Returns true if this has any valid effect specs */
	bool HasValidEffects() const;

	/** Returns true if this has any valid targets */
	bool HasValidTargets() const;

	/** Adds new targets to target data */
	void AddTargets(const TArray<FGameplayAbilityTargetDataHandle>& TargetData, const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);

	/** Add a new target to target data */
	void SetTarget(AActor* TargetActors);

	/** Clears target data */
	void ClearTargets();

	/** Sets SetByCallerMagnitude for any GameplayEffectSpecs matching the input ModifierTag. */
	void SetSetByCallerMagnitude(const FGameplayTag& ModifierTag, const float Magnitude);
	void SetSetByCallerMagnitude(const FName& ModifierTag, const float Magnitude);

	/** Applies all effects in the container to the input target actor if there is one, else to any previously set targets. */
	void ApplyEffectToTarget(AActor* TargetActor = nullptr);

private:
	/** Reference to the ability which created this spec */
	UNGameplayAbility* OwningAbility;
};