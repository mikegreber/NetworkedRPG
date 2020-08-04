// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"
#include "NGameplayAbility_Damaging.generated.h"

/**
 * Base class for a UNGameplayAbility that deals damage that can be set by the Damage property. Should override.
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_Damaging : public UNGameplayAbility
{
	GENERATED_BODY()

public:
	UNGameplayAbility_Damaging();

	// Set OnWeaponHit callback.
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** Returns true if TargetActor and OwningActor implement INDamageableInterface and are not on the same team. */
	bool CanDamage(AActor* TargetActor) const;

	/** The name of the tag of the FNGameplayEffectContainerSpec for the effect to apply when executing.
	* EffectContainerMap **MUST** have container with this tag as the key. */
	UPROPERTY(EditAnywhere, Category = "Settings|GameplayEffects")
	FGameplayTag EffectHitTag;

private:
	/** The name of the data tag of GameplayEffect within the EffectContainerMap that we want to set using the Damage property.
	* The gameplay effect must have an attribute modifier with a Magnitude Calculation Type that is Set By Caller with this tag. */
	UPROPERTY(EditAnywhere, Category = "Settings|GameplayEffects")
	FGameplayTag DamageModifierTag;

	/** How much damage should to inflict. */
	UPROPERTY(EditAnywhere, Category = "Settings|GameplayEffects")
	float Damage;
};
