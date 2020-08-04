// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Items/Data/NItem.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "NInventoryTypes.generated.h"

/**
* A slot that can hold any UNItem data. Use for inventory. Use subclasses for character equipped items in CombatComponent.
*/
USTRUCT(BlueprintType)
struct FNInventorySlot
{	
	GENERATED_USTRUCT_BODY()

	friend class UNInventoryComponent;
	
	FNInventorySlot():
		ItemData(nullptr),
		SlotNumber(-1),
		Count(-1),
		bIsSlotted(false)
	{}

	FNInventorySlot(UNItem* ItemData, int32 Count, int32 SlotNumber):
		ItemData(ItemData),
		SlotNumber(SlotNumber),
		Count(Count),
		bIsSlotted(false)
	{}

	virtual ~FNInventorySlot() = default;

	/** **Replicated** All visual properties of this slot are generated from ItemData. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InventorySlot")
	UNItem* ItemData;

	/** **Replicated** The slot number is used as a key to link slotted items their counterpart in the inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InventorySlot")
	int32 SlotNumber;

	/** **Replicated** The number of items in the slot. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "InventorySlot")
	int32 Count;

	/** Put an item from the inventory in this slot. */
	virtual bool SlotItem(FNInventorySlot& InventoryItem);

	/** Will remove the item in this slot and return a copy of it. Returns NullSlot if there was no item slotted. */
	virtual FNInventorySlot DeSlotItem();

	/** Will remove the item in this slot and return a copy of it. Returns NullSlot if there was no item slotted. */
	virtual FNInventorySlot GetItemCopy();

	/** Sets ItemData and generates any assets that go with it. Returns true if successfully set. */
	virtual bool SetItemData(UNItem* InItemData);
	
	/** Resets all slot item data to defaults. Keeps data identifying the slot. */
	virtual void ClearSlot();

	/** Returns true if there is an item in this slot. */
	virtual bool IsSlotted() const { return bIsSlotted; }

	static FNInventorySlot& NullSlot() { return NullInventorySlot; }

	virtual operator bool() const { return *this != NullInventorySlot; }	
	friend bool operator==(const FNInventorySlot& Lhs, const FNInventorySlot& RHS) { return Lhs.SlotNumber == RHS.SlotNumber && Lhs.Count == RHS.Count && Lhs.ItemData == RHS.ItemData; }	
	friend bool operator!=(const FNInventorySlot& Lhs, const FNInventorySlot& RHS) { return !(Lhs == RHS); }

	virtual FString ToString(const FString& Class = FString("FNInventorySlot")) const;

protected:
	
	/** **Replicated** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsSlotted;

	static FNInventorySlot NullInventorySlot;

};


/**
* Base class for a slot that holds data equipment and can be used with Gameplay Abilities.
*/
USTRUCT(BlueprintType)
struct FNEquipmentSlot : public FNInventorySlot
{
	GENERATED_USTRUCT_BODY()

	friend class UNCombatComponent;

	FNEquipmentSlot():
		FNInventorySlot(),
		SlotId(ENItemSlotId::None),
		ItemMesh(nullptr),
		OwnerCharacter(nullptr),
		AbilitySystemComponent(nullptr)
	{}

	explicit FNEquipmentSlot(const FPrimaryAssetType& ItemType, const ENItemSlotId SlotId):
		FNInventorySlot(),
		ItemType(ItemType),
		SlotId(SlotId),
		ItemMesh(nullptr),
		OwnerCharacter(nullptr),
		AbilitySystemComponent(nullptr)
	{}
	
	/** The type of items that can go into this slot */
	UPROPERTY(NotReplicated, EditAnywhere, BlueprintReadOnly, Category = "SlotType")
	FPrimaryAssetType ItemType;

	/** Unique to slotted item - The number of this slot. Corresponds to index of item in inventory. */
	UPROPERTY(NotReplicated, EditAnywhere, BlueprintReadOnly, Category = "SlotType")
	ENItemSlotId SlotId;

	/** **MUST CALL** before this slot can be used. */
	virtual void Initialize(ACharacter* Owner);

	// /** Put an item from the inventory in this slot. */
	// virtual bool SlotItem(FNInventorySlot& InventoryItem) override;
	
	/** If there is an item in this slot, remove it. */
	virtual FNInventorySlot DeSlotItem() override;

	/** Sets ItemData and generates any assets that go with it. */
	virtual bool SetItemData(UNItem* InItemData) override;
	
	/** Resets all slot item data to defaults. Keeps data identifying the slot. */
	virtual void ClearSlot() override;

	static FNEquipmentSlot& NullSlot() { return NullEquipmentSlot; }

	virtual operator bool() const override { return *this != NullEquipmentSlot; }

	friend bool operator==(const FNEquipmentSlot& Lhs, const FNEquipmentSlot& RHS)
	{
		return static_cast<const FNInventorySlot&>(Lhs) == static_cast<const FNInventorySlot&>(RHS)
			&& Lhs.ItemType == RHS.ItemType
			&& Lhs.SlotId == RHS.SlotId;
	}
	
	friend bool operator!=(const FNEquipmentSlot& Lhs, const FNEquipmentSlot& RHS)
	{
		return !(Lhs == RHS);
	}

	virtual FString ToString(const FString& Type = FString("FNEquipmentSlot")) const override;
	
protected:

	/** Generated from ItemData. */
	UPROPERTY(NotReplicated, VisibleAnywhere, BlueprintReadWrite, Category = "EquipmentSlot")
	USkeletalMesh* ItemMesh;

	/** Generated from ItemData. */
	UPROPERTY(NotReplicated, VisibleAnywhere, Category = "EquipmentSlot")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	/** Persistent slot property set in Initialize() */
	UPROPERTY(NotReplicated)
	ACharacter* OwnerCharacter;

	/** Persistent slot property set in Initialize() */
	UPROPERTY(NotReplicated, VisibleAnywhere, Category = "EquipmentSlot")
	class UNAbilitySystemComponent* AbilitySystemComponent;

	static FNEquipmentSlot NullEquipmentSlot;

private:

	/** [Server] Grants gameplay abilities from the <UNItem* Item> if on the server. Called by SlotItem().*/
	void GiveGameplayAbilities(UNItem* Item);

	/** [Server] Removes gameplay abilities from the <UNItem* Item> if on the server. Called on DeSlotItem() and by GiveGameplayAbilities().*/
	void RemoveGameplayAbilities();
};



/**
* A slot that holds data for non-weapon equipment. Use with CombatComponent.
*/
USTRUCT(BlueprintType)
struct FNArmourSlot : public FNEquipmentSlot
{
	GENERATED_USTRUCT_BODY()

	FNArmourSlot():
		MeshComponent(nullptr)
	{}

	explicit FNArmourSlot(const FPrimaryAssetType& ItemType, const ENItemSlotId SlotId):
		FNEquipmentSlot(ItemType, SlotId),
		MeshComponent(nullptr)
	{}
	
	/** **MUST CALL** before this slot can be used. */
	virtual void Initialize(ACharacter* Owner) override;

	/** Sets ItemData and generates any assets that go with it. */
	virtual bool SetItemData(UNItem* InItemData) override;

	/** Resets all slot item data to defaults. Keeps data identifying the slot. */
	virtual void ClearSlot() override;

	static FNArmourSlot& NullSlot() { return NullArmourSlot; }

	virtual FString ToString(const FString& Type =  FString("FNArmourSlot")) const override;

protected:

	/** Persistent slot property set in Initialize() */
	UPROPERTY(NotReplicated, VisibleAnywhere)
	USkeletalMeshComponent* MeshComponent;

	static FNArmourSlot NullArmourSlot;
};

class ANWeaponActor;

/**
 * A slot that holds data for a weapon. Use with CombatComponent.
 */
USTRUCT(BlueprintType)
struct FNWeaponSlot : public FNEquipmentSlot
{
	GENERATED_USTRUCT_BODY()

	FNWeaponSlot():
		FNEquipmentSlot(),
		bIsEquipped(false),
		WeaponActor(nullptr) 
	{}

	explicit FNWeaponSlot(const FPrimaryAssetType& ItemType, const ENItemSlotId SlotId, FGameplayTag EquippedGameplayTag):
		FNEquipmentSlot(ItemType, SlotId),
		bIsEquipped(false),
		WeaponActor(nullptr)
	{}

	/** Sets ItemData and generates slot properties from it. */
	virtual bool SetItemData(UNItem* InItemData) override;
	
	/** Removes ItemData and anything generated by it. Keeps data identifying the slot. */
	virtual void ClearSlot() override;
	
	/** Equips the slotted weapon. Animates to position if bAnimate is true, else snaps to the equipped socket. */
	void Equip(bool bAnimate = true);

	/** Holsters the slotted weapon. Animates to position if bAnimate is true, else snaps to the holstered socket. */
	void Holster(bool bAnimate = true);

	/** Returns the weapon mesh if a weapon is equipped. Use for activating/deactivating collision of melee weapons. */
	USkeletalMeshComponent* GetWeaponMesh() const;
	
	/** Returns true if weapon is both slotted and equipped. */
	bool IsEquipped() const;

	/** Returns true if weapon is both slotted and holstered. */
	bool IsHolstered() const;
	
	static FNWeaponSlot& NullSlot() { return NullWeaponSlot; }

	virtual FString ToString(const FString& Type = FString("FNWeaponSlot")) const override;

protected:

	/** **Replicated** */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsEquipped;

	/** Set from ItemData. */
	UPROPERTY(NotReplicated, VisibleAnywhere, Category = "Weapon")
	ANWeaponActor* WeaponActor;

	static FNWeaponSlot NullWeaponSlot;
};


/**
 * Helper template function for searching TArrays of slots.
 */
template <class SlotType, class PredicateSlotType = FNEquipmentSlot>
SlotType& SearchSlots(TArray<SlotType>& Slots, bool& bSlotFound, TFunctionRef<bool(const PredicateSlotType& Slot)> Predicate, bool bEmptyOnly = false);


template <class SlotType, class PredicateSlotType>
SlotType& SearchSlots(TArray<SlotType>& Slots, bool& bSlotFound, TFunctionRef<bool(const PredicateSlotType& Slot)> Predicate, bool bEmptyOnly)
{
	for (auto& Slot : Slots)
	{
		if (Predicate(Slot))
		{
			bSlotFound = true;
			
			if (!bEmptyOnly || !Slot.IsSlotted())
			{
				return Slot;
			}
		}
	}

	return SlotType::NullSlot();
}

