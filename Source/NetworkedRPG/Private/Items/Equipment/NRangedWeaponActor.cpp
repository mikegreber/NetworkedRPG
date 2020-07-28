// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Equipment/NRangedWeaponActor.h"
#include "Characters/NCharacterBase.h"
#include "Net/UnrealNetwork.h"

ANRangedWeaponActor::ANRangedWeaponActor() : ANWeaponActor()
{
}

void ANRangedWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION(ANRangedWeaponActor, ClipAmmo, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ANRangedWeaponActor, MaxClipAmmo, COND_OwnerOnly);
}

void ANRangedWeaponActor::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);
    
    DOREPLIFETIME_ACTIVE_OVERRIDE(ANRangedWeaponActor, ClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)));
}


void ANRangedWeaponActor::SetClipAmmo(int32 NewClipAmmo)
{
    int32 OldClipAmmo = ClipAmmo;
    ClipAmmo = NewClipAmmo;
    OnClipAmmoChanged.Broadcast(OldClipAmmo, ClipAmmo);
}

void ANRangedWeaponActor::SetMaxClipAmmo(int32 NewMaxClipAmmo)
{
    int32 OldMaxClipAmmo = MaxClipAmmo;
    MaxClipAmmo = NewMaxClipAmmo;
    OnMaxClipAmmoChanged.Broadcast(OldMaxClipAmmo, MaxClipAmmo);
}


void ANRangedWeaponActor::BeginPlay()
{
    Super::BeginPlay();
}

void ANRangedWeaponActor::OnRep_ClipAmmo(int32 OldClipAmmo)
{
    OnClipAmmoChanged.Broadcast(OldClipAmmo, ClipAmmo);
}

void ANRangedWeaponActor::OnRep_MaxClipAmmo(int32 OldMaxClipAmmo)
{
    OnMaxClipAmmoChanged.Broadcast(OldMaxClipAmmo, MaxClipAmmo);
}
