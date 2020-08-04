// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NGameplayAbility_Damaging.h"
#include "NGameplayAbility_Attack.generated.h"

class ANWeaponActor;

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_Attack : public UNGameplayAbility_Damaging
{
	GENERATED_BODY()

public:
	UNGameplayAbility_Attack();

private:
	/** Reference to the current active weapon for activating and deactivating */
	UPROPERTY()
	ANWeaponActor* ActiveWeapon;
	
public:
	// Set OnWeaponHit callback.
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	// Remove OnWeaponHit callback.
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:


	UFUNCTION()
	void OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when the active weapon overlaps an object that overlaps Weapon collision */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "OnWeaponHit"))
	void K2_OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult);

};
