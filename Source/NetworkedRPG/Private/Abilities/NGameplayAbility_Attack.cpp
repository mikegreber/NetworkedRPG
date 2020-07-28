// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NGameplayAbility_Attack.h"
#include "Characters/Components/NCombatComponent.h"
#include "Items/Equipment/NWeaponActor.h"
#include "Interface/NDamageableInterface.h"
#include "Kismet/KismetSystemLibrary.h"

class ANWeaponActor;

bool UNGameplayAbility_Attack::CanAttack(AActor* InTarget) const
{
    const TScriptInterface<INDamageableInterface> Owner = GetAvatarActorFromActorInfo();
    const TScriptInterface<INDamageableInterface> Target = InTarget;

    if (Owner && Target)
    {
        return Owner->GetTeam() != Target->GetTeam();
    }

    return false;
}

void UNGameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (CombatComponent)
    {
        CombatComponent->OnWeaponHit.AddDynamic(this, &UNGameplayAbility_Attack::OnWeaponHit);
    }
}

void UNGameplayAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (CombatComponent)
    {
        CombatComponent->OnWeaponHit.RemoveAll(this);
    }
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UNGameplayAbility_Attack::OnWeaponHit(AActor* HitActor, bool bFromSweep, const FHitResult& SweepResult)
{
    K2_OnWeaponHit(HitActor, bFromSweep, SweepResult);  
}

void UNGameplayAbility_Attack::K2_OnWeaponHit_Implementation(AActor* HitActor, bool bFromSweep,
    const FHitResult& SweepResult)
{
    UKismetSystemLibrary::PrintString(GetWorld(), "GameplayAbility_Attack: Need to override OnWeaponHit in Blueprint.", true, true, FColor::Red);
}

