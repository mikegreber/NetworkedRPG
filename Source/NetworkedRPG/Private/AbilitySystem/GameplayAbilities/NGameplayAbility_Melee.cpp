// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbility_Melee.h"
#include "Components/Combat/NCombatComponent.h"
#include "Interface/NDamageableInterface.h"

UNGameplayAbility_Melee::UNGameplayAbility_Melee()
{
}


void UNGameplayAbility_Melee::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	
	if (CombatComponent)
	{
		CombatComponent->OnWeaponHit.AddDynamic(this, &UNGameplayAbility_Melee::OnWeaponHit);
	}
}


void UNGameplayAbility_Melee::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (CombatComponent)
	{
		CombatComponent->OnWeaponHit.RemoveAll(this);
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}


void UNGameplayAbility_Melee::OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult)
{
	INDamageableInterface* Damageable = Cast<INDamageableInterface>(HitActor);
	if (!Damageable)
	{
		return;
	}
	
	FNGameplayEffectContainerSpec Spec = EffectContainerSpecMap[EffectHitTag];
	Spec.ApplyEffectToTarget(HitActor);
	Damageable->OnHit(10, 0);
	
	BP_OnWeaponHit(HitActor, bFromSweep, SweepResult);  
}



