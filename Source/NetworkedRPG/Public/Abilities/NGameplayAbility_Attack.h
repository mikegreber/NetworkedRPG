// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/NGameplayAbility.h"
#include "NGameplayAbility_Attack.generated.h"

class ANWeaponActor;
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_Attack : public UNGameplayAbility
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category="Attack")
    bool CanAttack(AActor* Target) const;

protected:
    /** Reference to the current active weapon for activating and deactivating */
    UPROPERTY()
    ANWeaponActor* ActiveWeapon;
    
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    UFUNCTION()
    void OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult);

    /** Called when the active weapon overlaps an object that overlaps Weapon collision */
    UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "OnWeaponHit"))
    void K2_OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult);

};
