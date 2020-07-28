// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NGameplayAbility_FireGun.h"


#include "DrawDebugHelpers.h"
#include "Abilities/NPlayMontageAndWaitForEventTask.h"
#include "Camera/CameraComponent.h"
#include "Player/NCharacter.h"
#include "Characters/Components/NCombatComponent.h"
#include "Components/SphereComponent.h"
#include "Items/Equipment/NProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/ActorComponent.h"

UNGameplayAbility_FireGun::UNGameplayAbility_FireGun()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Gun.Shoot"));
    AbilityTags.AddTag(Tag);
    ActivationOwnedTags.AddTag(Tag);

    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill")));

    Range = 1000.f;
    Damage = 12.f;
}

void UNGameplayAbility_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }

    UNPlayMontageAndWaitForEventTask* Task = UNPlayMontageAndWaitForEventTask::PlayMontageAndWaitForEvent(this, NAME_None, FireMontage, FGameplayTagContainer(), 1.0f, NAME_None, false, 1.0f);
    Task->OnBlendOut.AddDynamic(this, &UNGameplayAbility_FireGun::OnComplete);
    Task->OnCompleted.AddDynamic(this, &UNGameplayAbility_FireGun::OnComplete);
    Task->OnInterrupted.AddDynamic(this, &UNGameplayAbility_FireGun::OnCancelled);
    Task->OnCancelled.AddDynamic(this, &UNGameplayAbility_FireGun::OnCancelled);
    Task->EventReceived.AddDynamic(this, &UNGameplayAbility_FireGun::EventReceived);

    Task->ReadyForActivation();
    EventReceived(FGameplayTag::RequestGameplayTag(FName("Event.Montage.SpawnProjectile")), FGameplayEventData());
}

void UNGameplayAbility_FireGun::OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UNGameplayAbility_FireGun::OnComplete(FGameplayTag EventTag, FGameplayEventData EventData)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UNGameplayAbility_FireGun::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
    if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.EndAbility")))
    {
        UKismetSystemLibrary::PrintString(GetWorld(), "Ending Ability");

        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // Only spawn projectiles on the server.
    // TODO Setup local prediction of projectiles
    if (GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority && EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.SpawnProjectile")))
    {
        if (!OwningCharacter || !IsLocallyControlled())
        {
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
            return;
        }

        if (CombatComponent)
        {
            const FVector TraceStart = Interface->GetTraceStartComponent()->GetComponentLocation();
            FVector End = TraceStart + Interface->GetTraceStartComponent()->GetComponentRotation().Vector() * Range;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(GetOwningActorFromActorInfo());

            FHitResult Hit;
            GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_Visibility, Params);

            if (Hit.IsValidBlockingHit())
            {
                UKismetSystemLibrary::PrintString(GetWorld(), "Hit!");
                End = Hit.Location;
            }
            else
            {
                UKismetSystemLibrary::PrintString(GetWorld(), "Miss!");
            }
            
           
       
            FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffect, GetAbilityLevel());
            DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
            
            FVector MuzzleLocation = CombatComponent->GetRangedWeaponMesh() ? CombatComponent->GetRangedWeaponMesh()->GetSocketLocation(FName("Muzzle")) : FVector();
            UKismetSystemLibrary::PrintString(GetWorld(), MuzzleLocation.ToString());
            FRotator ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, End);
            FTransform MuzzleTransform = FTransform(ProjectileRotation.Quaternion(), MuzzleLocation);
            
            ANProjectile* Projectile = GetWorld()->SpawnActorDeferred<ANProjectile>(ProjectileClass, MuzzleTransform, GetOwningActorFromActorInfo(), OwningCharacter, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
            // Not sure about using IgnoreActorWhenMoving, may add an excessive amount of actors to ignore list, may be more efficient to just check if overlap is instigator in the overlap events instead.
            CombatComponent->GetOwnerMesh()->IgnoreActorWhenMoving(Projectile, true);
            Projectile->CollisionComponent->IgnoreActorWhenMoving(OwningCharacter, true);
            Projectile->DamageEffectSpecHandle = DamageEffectSpecHandle;
            Projectile->Range = Range;
            Projectile->FinishSpawning(MuzzleTransform);

            // ServerSpawnProjectile(FVector_NetQuantize(End));

            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        }
        
    }
}

bool UNGameplayAbility_FireGun::ServerSpawnProjectile_Validate(FVector_NetQuantize SpawnVector)
{
    return true;
}

void UNGameplayAbility_FireGun::ServerSpawnProjectile_Implementation(FVector_NetQuantize End)
{
    DrawDebugSphere(GetWorld(), GetAvatarActorFromActorInfo()->GetActorLocation(), 50.f, 10, FColor::Red, false, 2, 1, 10);
    UKismetSystemLibrary::PrintString(GetWorld(), "ServerSpawnProjectile");
    FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffect, GetAbilityLevel());
    DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
    
    FVector MuzzleLocation = CombatComponent->GetRangedWeaponMesh() ? CombatComponent->GetRangedWeaponMesh()->GetSocketLocation(FName("Muzzle")) : FVector();
    FRotator ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(MuzzleLocation, End);
    FTransform MuzzleTransform = FTransform(ProjectileRotation.Quaternion(), MuzzleLocation);
           
    ANProjectile* Projectile = GetWorld()->SpawnActorDeferred<ANProjectile>(ProjectileClass, MuzzleTransform, GetOwningActorFromActorInfo(), OwningCharacter, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    // Not sure about using IgnoreActorWhenMoving, may add an excessive amount of actors to ignore list, may be more efficient to just check if overlap is instigator in the overlap events instead.
    CombatComponent->GetOwnerMesh()->IgnoreActorWhenMoving(Projectile, true);
    Projectile->CollisionComponent->IgnoreActorWhenMoving(OwningCharacter, true);
    Projectile->DamageEffectSpecHandle = DamageEffectSpecHandle;
    Projectile->Range = Range;
    Projectile->FinishSpawning(MuzzleTransform);
}
