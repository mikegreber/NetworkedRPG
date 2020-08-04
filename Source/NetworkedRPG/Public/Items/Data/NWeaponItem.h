// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Items/Data/NEquipmentItem.h"
#include "NWeaponItem.generated.h"

class ANWeaponActor;

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNWeaponItem : public UNEquipmentItem
{
	GENERATED_BODY()

public:
	UNWeaponItem();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** The stance of the character when this item is equipped (for anim blueprint). */
	UPROPERTY(EditAnywhere, Category = "Weapon")
	ENStance Stance;
	
	/** The offset of the root of the spawned weapon from the HolsteredSocket. */
	UPROPERTY(EditAnywhere, Category = "Attachment")
	FAttachmentOffset HolsteredAttachmentOffset;

	/** The montage that will play to equip this weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* EquipMontage;

	/** The montage that will play to holster this weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HolsterMontage;

	/** Gameplay tag that will be given to host while this weapon is equipped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	FGameplayTag EquippedGameplayTag;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Attachment")
	ENEquipmentSocket HolsteredSocket;
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Set HolsteredSocketName to HolsteredSocket. */
	virtual void PostLoad() override;
	
	/** Sets HolsteredSocketName when HolsteredSocket changes. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	/** Gives warning when EquippedGameplayTag and/or HolsteredSocket is not set on save. */
	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Will be set by HolsteredSocket. */
	FName HolsteredSocketName;

	/** Returns the holstered relative transform offset. */
	FTransform GetHolsteredTransform() const;
};
