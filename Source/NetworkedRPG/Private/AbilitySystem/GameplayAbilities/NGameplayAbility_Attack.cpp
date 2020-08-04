// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbility_Attack.h"
#include "Components/Combat/NCombatComponent.h"
#include "Items/Actors/NWeaponActor.h"
#include "Interface/NDamageableInterface.h"


UNGameplayAbility_Attack::UNGameplayAbility_Attack()
{
}


void UNGameplayAbility_Attack::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);
    
    if (CombatComponent)
    {
        CombatComponent->OnWeaponHit.AddDynamic(this, &UNGameplayAbility_Attack::OnWeaponHit);
    }
}


void UNGameplayAbility_Attack::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    if (CombatComponent)
    {
        CombatComponent->OnWeaponHit.RemoveAll(this);
    }
}


void UNGameplayAbility_Attack::OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult)
{
    INDamageableInterface* Damageable = Cast<INDamageableInterface>(HitActor);
    if (!Damageable)
    {
        return;
    }
    
    FNGameplayEffectContainerSpec Spec = EffectContainerSpecMap[EffectHitTag];
    Spec.ApplyEffectToTarget(HitActor);
    Damageable->OnHit(10, 0);
    
    K2_OnWeaponHit(HitActor, bFromSweep, SweepResult);  
}


void UNGameplayAbility_Attack::K2_OnWeaponHit_Implementation(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult)
{
    // UKismetSystemLibrary::PrintString(GetWorld(), "GameplayAbility_Attack: Need to override OnWeaponHit in Blueprint.", true, true, FColor::Red);
}

