// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NGameplayAbility_Damaging.h"
#include "NGameplayAbility_FireGun.generated.h"

/**
* GameplayAbility class for gun weapons. Applies EffectContainerSpecMap Key Effect.Hit when the spawned projectile overlaps a target.
* Must set FireMontage, ProjectileClass, and Range in BP.
* Spawns a projectile on the server and plays an animation montage.
*/
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_FireGun : public UNGameplayAbility_Damaging
{
	GENERATED_BODY()

public:
	UNGameplayAbility_FireGun();

	/** Ensures FireMontage and ProjectileClass are set */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

protected:
	/** Handles firing the weapon/ */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** The montage to play when this ability is activated. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|Animation")
	UAnimMontage* FireMontage;

	/** The projectile class to spawn when this ability is activated. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|Animation")
	TSubclassOf<AActor> ProjectileClass;
	
	/** Maximum range of this weapon */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|Projectile")
	float Range;

	/** Projectile velocity. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|Projectile")
	float Velocity;
	
	/** Callback for when montage is finished */
	UFUNCTION()
	void OnComplete();

};
