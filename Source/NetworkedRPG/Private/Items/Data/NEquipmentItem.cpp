// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Data/NEquipmentItem.h"

UEnum* UNEquipmentItem::EquipmentSocketEnum = nullptr;
UEnum* UNEquipmentItem::SlotIdEnum = nullptr;

UNEquipmentItem::UNEquipmentItem() : UNItem()
{
    EquipmentSocketEnum = FindObject<UEnum>(ANY_PACKAGE, *FString("ENEquipmentSocket"));
    SlotIdEnum = FindObject<UEnum>(ANY_PACKAGE, *FString("ENItemSlotId"));
}


void UNEquipmentItem::PostLoad()
{
    Super::PostLoad();

    const FString EnumAsString = EquipmentSocketEnum->GetNameStringByIndex(static_cast<int32>(EquippedSocket));
    EquippedSocketName = FName(EnumAsString);
}


void UNEquipmentItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
 
    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UNEquipmentItem, EquippedSocket) && EquipmentSocketEnum)
    {
        const FString EnumAsString = EquipmentSocketEnum->GetNameStringByIndex(static_cast<int32>(EquippedSocket));
        EquippedSocketName = FName(EnumAsString);
    }
}


void UNEquipmentItem::PreSave(const ITargetPlatform* TargetPlatform)
{
    Super::PreSave(TargetPlatform);

    if (EquippedSocket == ENEquipmentSocket::None)
    {
        ShowWarning("EquippedSocket not set!");
    }
}


FTransform UNEquipmentItem::GetEquippedTransform() const
{
    return FTransform(AttachmentOffset.RelativeRotationOffset, AttachmentOffset.RelativeLocationOffset);
}
