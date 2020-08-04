// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/NInventoryTypes.h"
#include "Components/Inventory/NInventoryComponent.h"
#include "Items/Data/NItem.h"
#include "Items/Data/NWeaponItem.h"
#include "Items/Actors/NMeleeWeaponActor.h"
#include "Items/Actors/NRangedWeaponActor.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"
#include "AbilitySystem/NAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "NetworkedRPG/NetworkedRPG.h"


FNInventorySlot FNInventorySlot::NullInventorySlot = FNInventorySlot();
FNEquipmentSlot FNEquipmentSlot::NullEquipmentSlot = FNEquipmentSlot();
FNArmourSlot FNArmourSlot::NullArmourSlot = FNArmourSlot();
FNWeaponSlot FNWeaponSlot::NullWeaponSlot = FNWeaponSlot();

/**
 * InventorySlot
 */
bool FNInventorySlot::SlotItem(FNInventorySlot& InventoryItem)
{   
    if (!InventoryItem.ItemData)
    {
        return false;
    }
    
    // Update the input item from inventory so we can see it is slotted.
    InventoryItem.bIsSlotted = true;

    bIsSlotted = SetItemData(InventoryItem.ItemData);
    if (bIsSlotted)
    {
        Count = InventoryItem.Count;
        SlotNumber = InventoryItem.SlotNumber;
    }
    
    return bIsSlotted;
}

FNInventorySlot FNInventorySlot::DeSlotItem()
{
    
    if (bIsSlotted)
    {
        FNInventorySlot OutSlot = *this;
        ClearSlot();
        return OutSlot;
    }
    
    if (DebugCombatComponent)
    {
        Print(nullptr, FString::Printf(TEXT("%s Failed: Called on empty slot."), *FString(__FUNCTION__)), EPrintType::Failure);
    }

    return NullSlot();
}

FNInventorySlot FNInventorySlot::GetItemCopy()
{
    if (bIsSlotted)
    {
        FNInventorySlot OutSlot = *this;
        return OutSlot;
    }
    
    if (DebugCombatComponent)
    {
        Print(nullptr, FString::Printf(TEXT("%s Failed: Called on empty slot."), *FString(__FUNCTION__)), EPrintType::Failure);
    }
    
    return NullSlot();
}

bool FNInventorySlot::SetItemData(UNItem* InItemData)
{
    if (!InItemData)
    {
        if (DebugCombatComponent)
        {
            Print(nullptr, FString::Printf(TEXT("%s Failed: InItemData is null."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return false;
    }

    ItemData = InItemData;
    return true;
}

void FNInventorySlot::ClearSlot()
{
    Count = -1;
    SlotNumber = -1;
    ItemData = nullptr;
    bIsSlotted = false;
}

FString FNInventorySlot::ToString(const FString& Type) const
{
    return FString::Printf(TEXT("%s: %s, Count: %d, SlotNumber: %d, bIsSlotted: %s"), *Type, *FString(ItemData ? ItemData->ItemName : "empty"), Count, SlotNumber, *FString(bIsSlotted ? "true" : "false")); 
}




/**
* EquipmentSlot
*/

void FNEquipmentSlot::Initialize(ACharacter* Owner)
{
    if (!IsValid(Owner))
    {
        if (DebugCombatComponent)
        {
            Print(nullptr, FString::Printf(TEXT("%s Failed: Called with null <ACharacter* Owner>."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return;
    }
    
    OwnerCharacter = Owner;
    
    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerCharacter);
    if (ASI)
    {
        AbilitySystemComponent = Cast<UNAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
        if (!AbilitySystemComponent && DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Input <ACharacter* Owner> has null AbilitySystemComponent."), *FString(__FUNCTION__)), EPrintType::Warning);
        }
    }
    else
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Input <ACharacter* Owner> does not implement IAbilitySystemInterface."), *FString(__FUNCTION__)), EPrintType::Warning);
        }
    }

    if (DebugCombatComponent)
    {
        Print(nullptr, FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
    }
}


FNInventorySlot FNEquipmentSlot::DeSlotItem()
{
    if (DebugInventoryComponent)
    {
        if (ItemData)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Called on %s."), *FString(__FUNCTION__), *ItemData->ItemName), EPrintType::Warning);
        }
        else
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Called on empty slot."), *FString(__FUNCTION__)), EPrintType::Failure);
        }   
    }
    
    return Super::DeSlotItem();
}

bool FNEquipmentSlot::SetItemData(UNItem* InItemData)
{
    if (!InItemData)
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Failed: InItemData is null."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return false;
    }

    GiveGameplayAbilities(InItemData);
    
    return Super::SetItemData(InItemData);
}

void FNEquipmentSlot::ClearSlot()
{
    ItemMesh = nullptr;
    RemoveGameplayAbilities();

    Super::ClearSlot();
}

FString FNEquipmentSlot::ToString(const FString& Type) const
{
    const FString SlotIdAsString = UNEquipmentItem::SlotIdEnum ? UNEquipmentItem::SlotIdEnum->GetNameStringByIndex(static_cast<int32>(SlotId)) : FString("Error"); 
    return Super::ToString(Type) + FString::Printf(TEXT(", ItemType: %s, SlotId: %s"), *ItemType.ToString(), *SlotIdAsString) ;
}

void FNEquipmentSlot::GiveGameplayAbilities(UNItem* Item)
{
    if (!OwnerCharacter)
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Called with null OwnerCharacter, must call Initialize() on slot in begin play."), *FString(__FUNCTION__)), EPrintType::Failure);    
        }
        
        return;
    }
    
    // Only call on server
    if (!OwnerCharacter->HasAuthority())
    {
        return;
    }

    if (!IsValid(Item))
    {
        return;
    }

    // Clear gameplay abilities from previous slotted item
    RemoveGameplayAbilities();

    for (const auto AbilityInfo : Item->GrantedAbilities)
    {
        AbilitySpecHandles.Add(
            AbilitySystemComponent->GiveAbility(
                FGameplayAbilitySpec(AbilityInfo.Ability,
                AbilityInfo.AbilityLevel,
                static_cast<int32>(AbilityInfo.Ability.GetDefaultObject()->AbilityInputID)
        )));
    }
}

void FNEquipmentSlot::RemoveGameplayAbilities()
{
    if (!IsValid(OwnerCharacter))
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Called with null OwnerCharacter, must call Initialize() on slot in begin play."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return;
    }
    
    // Only call on server
    if (!OwnerCharacter->HasAuthority())
    {
        return;
    }
    
    for (const FGameplayAbilitySpecHandle& Ability : AbilitySpecHandles)
    {
        AbilitySystemComponent->SetRemoveAbilityOnEnd(Ability);
    }
    
    AbilitySpecHandles.Empty();
}



/**
* ArmourSlot
*/

void FNArmourSlot::Initialize(ACharacter* Owner)
{
    Super::Initialize(Owner);

    if (!IsValid(OwnerCharacter))
    {
        // Call to Super must failed
        return;
    }
    
    MeshComponent = NewObject<USkeletalMeshComponent>(Owner);
    MeshComponent->RegisterComponent();
    MeshComponent->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale);
}


bool FNArmourSlot::SetItemData(UNItem* InItemData)
{
    if (!InItemData)
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Failed: InItemData is null."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return false;
    }

    if (IsValid(MeshComponent))
    {
        MeshComponent->SetSkeletalMesh(InItemData->ItemMesh);
        
        UNEquipmentItem* EquipmentItem = Cast<UNEquipmentItem>(InItemData);
        if (EquipmentItem)
        {
            MeshComponent->SetRelativeTransform(EquipmentItem->GetEquippedTransform());
            MeshComponent->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, EquipmentItem->EquippedSocketName);
        }
    }

    return Super::SetItemData(InItemData);
}

void FNArmourSlot::ClearSlot()
{
    MeshComponent->SetSkeletalMesh(nullptr);

    Super::ClearSlot();
}


FString FNArmourSlot::ToString(const FString& Type) const
{
    return Super::ToString(Type);
}


/**
* WeaponSlot
*/

bool FNWeaponSlot::SetItemData(UNItem* InItemData)
{
    if (!InItemData)
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Failed: InItemData is null."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return false;
    }

    UNWeaponItem* WeaponItem = Cast<UNWeaponItem>(InItemData);
    if (!WeaponItem)
    {
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Failed: InItemData not of type WeaponItem."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return false;
    }

    // Spawn the new WeaponActor
    ANWeaponActor* NewWeaponActor;
    switch(WeaponItem->SlotId)
    {
    case ENItemSlotId::Ranged:
        NewWeaponActor = OwnerCharacter->GetMesh()->GetWorld()->SpawnActor<ANRangedWeaponActor>();
        break;
    case ENItemSlotId::Melee:
        NewWeaponActor = OwnerCharacter->GetMesh()->GetWorld()->SpawnActor<ANMeleeWeaponActor>();
        break;
    default:
        if (DebugCombatComponent)
        {
            Print(OwnerCharacter, FString::Printf(TEXT("%s Failed: Could not spawn the new weapon actor, invalid SlotId for weapon in UNWeaponItem. Can be Melee or Ranged."), *FString(__FUNCTION__)), EPrintType::Failure);
        }
        
        return false;
    }

    if (!NewWeaponActor)
    {
        Print(OwnerCharacter, FString::Printf(TEXT("%s Failed: Could not spawn the new weapon actor. This should never happen."), *FString(__FUNCTION__)), EPrintType::Failure);
        return false;
    }

    NewWeaponActor->Initialize(OwnerCharacter, WeaponItem);

    bool bEmptySlot = true;

    // Remove the old weapon actor if weapon was already slotted
    if (WeaponActor)
    {
        WeaponActor->Destroy();
        bEmptySlot = false;
    }

    WeaponActor = NewWeaponActor;

    // Play holster animation the slot was empty
    if (bEmptySlot)
    {
        Holster();
    }
    
    // Snap into current location if we already has a weapon equipped in this slot
    else if (IsEquipped())
    {
        Equip(false);
    }
    else
    {
        Holster(false);
    }
    
    return Super::SetItemData(InItemData);
}


void FNWeaponSlot::ClearSlot()
{
    if (WeaponActor)
    {
        WeaponActor->Destroy();
        WeaponActor = nullptr;
    }
    
    bIsEquipped = false;

    Super::ClearSlot();
}


void FNWeaponSlot::Equip(bool bAnimate)
{    
    if (IsValid(WeaponActor) && WeaponActor->Equip(bAnimate))
    {
        // Will trigger replication of slot and begin animation on other clients via OnRep_WeaponSlots.
        bIsEquipped = true;
    }
    else if (DebugCombatComponent)
    {
        Print(OwnerCharacter, FString::Printf(TEXT("%s Failed."), *FString(__FUNCTION__)), EPrintType::Failure);
    }
}


void FNWeaponSlot::Holster(bool bAnimate)
{
    if (IsValid(WeaponActor) && WeaponActor->Holster(bAnimate))
    {
        // Will trigger replication of slot and begin animation on other clients via OnRep_WeaponSlots.
        bIsEquipped = false;
    }
    else if (DebugCombatComponent)
    {
        Print(OwnerCharacter, FString::Printf(TEXT("%s Failed."), *FString(__FUNCTION__)), EPrintType::Failure);
    }
}


USkeletalMeshComponent* FNWeaponSlot::GetWeaponMesh() const
{
    return WeaponActor ? WeaponActor->GetMesh() : nullptr;
}


bool FNWeaponSlot::IsEquipped() const
{
    return IsSlotted() && bIsEquipped;
}


bool FNWeaponSlot::IsHolstered() const
{
    return IsSlotted() && !bIsEquipped;
}


FString FNWeaponSlot::ToString(const FString& Type) const
{
    return Super::ToString(Type) + FString::Printf(TEXT(", IsEquipped: %s"), *FString(bIsEquipped ? "true" : "false"));
}


