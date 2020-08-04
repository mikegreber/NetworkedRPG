// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Actors/NWeaponActor.h"
#include "NRangedWeaponActor.generated.h"

class ANGATA_LineTrace;
class ANCharacterBase;
class UNAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

/**
*  A ranged weapon actor. Subclassed from ANWeaponActor but has support for Ammunition.
*  NOT meant to be blueprinted, should spawned within game based off of UNWeaponItem data.
*  MUST call Initialize() after spawning.
*/
UCLASS()
class NETWORKEDRPG_API ANRangedWeaponActor : public ANWeaponActor
{
	GENERATED_BODY()

public:
	ANRangedWeaponActor();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. References and State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Ammo currently in the clip. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing=OnRep_ClipAmmo, Category = "RangedWeapon|Ammo")
	int32 ClipAmmo;

	/** Max amount of ammo we can hold in the clip. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing=OnRep_MaxClipAmmo, Category = "RangedWeapon|Ammo")
	int32 MaxClipAmmo;

	/** Will have this tag when firing (Automatic fire). */
	FGameplayTag WeaponIsFiringTag;

	UPROPERTY()
	ANGATA_LineTrace* LineTraceTargetActor;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Replicate ClipAmmo and MaxClipAmmo to client only */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Only replicated ClipAmmo if not currently firing. */
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UPROPERTY(BlueprintAssignable, Category = "RangedWeapon|Ammo")
	FWeaponAmmoChangedDelegate OnClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "RangedWeapon|Ammo")
	FWeaponAmmoChangedDelegate OnMaxClipAmmoChanged;

	/** Sets ClipAmmo and broadcasts OnClipAmmoChanged. */
	void SetClipAmmo(int32 NewClipAmmo);

	/** Sets MaxClipAmmo ammo and broadcasts OnClipAmmoChanged. */
	void SetMaxClipAmmo(int32 NewMaxClipAmmo);

	/** Broadcasts OnClipAmmoChanged. */
	UFUNCTION()
	virtual void OnRep_ClipAmmo(int32 OldClipAmmo);

	/** Broadcasts OnMaxClipAmmoChanged. */
	UFUNCTION()
	virtual void OnRep_MaxClipAmmo(int32 OldMaxClipAmmo);
};
