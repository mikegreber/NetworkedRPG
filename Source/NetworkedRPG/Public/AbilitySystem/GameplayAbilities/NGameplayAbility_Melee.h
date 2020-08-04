// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility_Damaging.h"
#include "NGameplayAbility_Melee.generated.h"

class ANWeaponActor;

/**
 * GameplayAbility class for melee weapons. Applies EffectContainerSpecMap Key Effect.Hit when the weapon overlaps a target.
 * Must implement ActivateAbility in BP Graph to add weapon swinging logic. Can add extra functionality to OnWeaponHit by
 * implementation in BP Graph.
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_Melee : public UNGameplayAbility_Damaging
{
	GENERATED_BODY()

public:
	UNGameplayAbility_Melee();

	// Set OnWeaponHit callback.
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	// Remove OnWeaponHit callback.
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	/** Called when the active weapon overlaps an object that overlaps Weapon collision */
	UFUNCTION()
	void OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when the active weapon overlaps an object that overlaps Weapon collision */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnWeaponHit"))
	void BP_OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult);
};
