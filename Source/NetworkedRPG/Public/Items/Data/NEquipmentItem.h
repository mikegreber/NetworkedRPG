// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Data/NItem.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "NEquipmentItem.generated.h"

USTRUCT(BlueprintType)
struct FAttachmentOffset
{
    GENERATED_USTRUCT_BODY()
	
    FAttachmentOffset(){}
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativeLocationOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RelativeRotationOffset;		
};

/** Sockets corresponding to sockets on the character skeleton. */
UENUM(BlueprintType)
enum class ENEquipmentSocket : uint8
{
    None						UMETA(DisplayName = "None"),
    RightHandSocket	    		UMETA(DisplayName = "RightHandSocket"),
    LeftHandSocket				UMETA(DisplayName = "LeftHandSocket"),
    LeftHolsterSocket   		UMETA(DisplayName = "LeftHolsterSocket"),
    RightHolsterSocket			UMETA(DisplayName = "RightHolsterSocket"),
    UpperBackHolsterSocket    	UMETA(DisplayName = "UpperBackHolsterSocket"),
    HeadSocket    	            UMETA(DisplayName = "HeadSocket"),
};

/**
 *  An item that can be attached to a slot on a skeletal mesh.
 */
UCLASS()
class NETWORKEDRPG_API UNEquipmentItem : public UNItem
{
	GENERATED_BODY()

public:
    UNEquipmentItem();

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 1. Blueprint Settings
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** The Slot for this item*/
    UPROPERTY(EditAnywhere, Category = "Equipment")
    ENItemSlotId SlotId;

    /** The offset from the socket the item is attached to. */
    UPROPERTY(EditAnywhere, Category = "Attachment")
    FAttachmentOffset AttachmentOffset;

protected:
    /** The socket this item will be attached to when equipped. */
    UPROPERTY(EditAnywhere, Category = "Attachment")
    ENEquipmentSocket EquippedSocket;
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 2. Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Set EquippedSocketName to EquippedSocket. */
    virtual void PostLoad() override;
    
    /** Update EquippedSocketName when EquippedSocket is changed. */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    
    /** Gives warning if EquippedSocket is not set on save. */
    virtual void PreSave(const ITargetPlatform* TargetPlatform) override;

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 3. Interface and Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Get the name of the socket this item will be attached to when equipped (Generated when EquippedSocket is selected).*/
    FName EquippedSocketName;

    /** Returns the equipped relative transform offset. */
    FTransform GetEquippedTransform() const;

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 4. Static Members
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Holds ENEquipmentSocket in UEnum* form so we can get the string names of the enum. */
    static UEnum* EquipmentSocketEnum;

    /** Holds ENSlotId in UEnum* form so we can get the string names of the enum. */
    static UEnum* SlotIdEnum;
};
