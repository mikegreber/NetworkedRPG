// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbility_FireGun.h"
#include "GameplayAbilities/Public/Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/Combat/NCombatComponent.h"
#include "AbilitySystem/NGameplayAbilityActorInterface.h"
#include "Items/Actors/NProjectile.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"


UNGameplayAbility_FireGun::UNGameplayAbility_FireGun()
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Gun.Shoot"));
    AbilityTags.AddTag(Tag);
    ActivationOwnedTags.AddTag(Tag);
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill")));
    Range = 1000.f;
}


void UNGameplayAbility_FireGun::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{   
    if (!ProjectileClass || !FireMontage)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s ProjectileClass and/or FireMontage not set in blueprints."), *GetName()), EPrintType::ShutDown);
    }

    Super::OnGiveAbility(ActorInfo, Spec);
}


void UNGameplayAbility_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }

    // Play fire montage
    UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FireMontage);
    Task->OnBlendOut.AddDynamic(this, &UNGameplayAbility_FireGun::OnComplete);
    Task->OnCompleted.AddDynamic(this, &UNGameplayAbility_FireGun::OnComplete);
    Task->OnInterrupted.AddDynamic(this, &UNGameplayAbility_FireGun::OnComplete);
    Task->OnCancelled.AddDynamic(this, &UNGameplayAbility_FireGun::OnComplete);
    Task->ReadyForActivation();

    // TODO Setup local prediction of projectiles, server only for now
    if (GetOwningActorFromActorInfo()->GetLocalRole() != ROLE_Authority)
    {
        return;
    }
    
    if (!OwningCharacter || !CombatComponent)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    const FVector TraceStart = Interface->GetTraceStartComponent()->GetComponentLocation();
    FVector End = TraceStart + Interface->GetTraceStartComponent()->GetComponentRotation().Vector() * Range;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwningActorFromActorInfo());

    FHitResult Hit;
    GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_Visibility, Params);

    if (Hit.IsValidBlockingHit())
    {
        End = Hit.Location;
    }

    // Find weapon muzzle transform
    FVector MuzzleLocation = CombatComponent->GetRangedWeaponMesh() ? CombatComponent->GetRangedWeaponMesh()->GetSocketLocation(FName("Muzzle")) : FVector();
    FRotator ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, End);
    FTransform MuzzleTransform = FTransform(ProjectileRotation.Quaternion(), MuzzleLocation);

    // Spawn and initialize projectile
    ANProjectile* Projectile = GetWorld()->SpawnActorDeferred<ANProjectile>(ProjectileClass, MuzzleTransform, GetOwningActorFromActorInfo(), OwningCharacter, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    Projectile->Initialize(EffectContainerSpecMap[EffectHitTag], Range, Velocity);
    Projectile->FinishSpawning(MuzzleTransform);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);   
}


void UNGameplayAbility_FireGun::OnComplete()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
