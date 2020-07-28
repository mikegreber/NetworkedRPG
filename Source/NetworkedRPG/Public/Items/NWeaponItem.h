// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAssetManager.h"
#include "Items/NEquipmentItem.h"
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

	UNWeaponItem()
	  : UNEquipmentItem(),
		HolsteredSocketName("InactiveCombatSocket")
	{
		ItemType = UNAssetManager::WeaponItemType;
		EquippedSocketName = "ActiveCombatSocket";
	}

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<ANWeaponActor> WeaponActorClass;

	UPROPERTY(EditAnywhere, Category = "Weapon")
    ENCombatMode CombatMode;
	
	UPROPERTY(EditAnywhere, Category = "Attachment")
    FAttachmentOffset HolsteredAttachmentOffset;

	UPROPERTY(VisibleAnywhere, Category = "Attachment")
	FName HolsteredSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* EquipMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HolsterMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	FGameplayTag EquippedGameplayTag;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	/** Gives warning when EquippedGameplayTag is not set on save */
	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;

	FTransform GetHolsteredTransform() const
	{
		return FTransform(HolsteredAttachmentOffset.RelativeRotationOffset, HolsteredAttachmentOffset.RelativeLocationOffset);
	}

protected:
	UPROPERTY(EditAnywhere, Category = "Attachment")
	ENEquipmentSocket HolsteredSocket;
};
