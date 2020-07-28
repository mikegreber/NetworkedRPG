// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/NItem.h"
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
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNEquipmentItem : public UNItem
{
	GENERATED_BODY()

public:

    UNEquipmentItem();

    static UEnum* EquipmentSocketEnum;
    static UEnum* SlotIdEnum;

    /** Automatically update the SocketName when the SlotId is changed */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    /** Returns the RelativeTransform that this items mesh should have */
    FTransform GetTransform() const;

    /** The Slot for this item*/
    UPROPERTY(EditAnywhere, Category = "Equipment")
    ENItemSlotId SlotId;

    UPROPERTY(EditAnywhere, Category = "Attachment")
    FAttachmentOffset AttachmentOffset;
    	
    UPROPERTY(VisibleAnywhere, Category = "Attachment")
    FName EquippedSocketName;

protected:
    UPROPERTY(EditAnywhere, Category = "Attachment")
    ENEquipmentSocket EquippedSocket;

};
