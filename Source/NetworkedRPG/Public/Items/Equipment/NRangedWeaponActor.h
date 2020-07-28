// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "Abilities/NGATA_LineTrace.h"
#include "Items/Equipment/NWeaponActor.h"
#include "NRangedWeaponActor.generated.h"

class ANCharacterBase;
class UNAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API ANRangedWeaponActor : public ANWeaponActor
{
	GENERATED_BODY()

public:
    ANRangedWeaponActor();

    UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing=OnRep_ClipAmmo, Category = "RangedWeapon|Ammo")
    int32 ClipAmmo;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing=OnRep_MaxClipAmmo, Category = "RangedWeapon|Ammo")
    int32 MaxClipAmmo;

    UPROPERTY(BlueprintAssignable, Category = "RangedWeapon|Ammo")
    FWeaponAmmoChangedDelegate OnClipAmmoChanged;

    UPROPERTY(BlueprintAssignable, Category = "RangedWeapon|Ammo")
    FWeaponAmmoChangedDelegate OnMaxClipAmmoChanged;
    
    UPROPERTY()
    ANGATA_LineTrace* LineTraceTargetActor;
    
    FGameplayTag WeaponIsFiringTag;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
    
    void SetClipAmmo(int32 NewClipAmmo);

    void SetMaxClipAmmo(int32 NewMaxClipAmmo);
    
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnRep_ClipAmmo(int32 OldClipAmmo);

    UFUNCTION()
    virtual void OnRep_MaxClipAmmo(int32 OldMaxClipAmmo);

    
};
