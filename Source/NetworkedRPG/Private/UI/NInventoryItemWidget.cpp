// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NInventoryItemWidget.h"
#include "Components/TextBlock.h"
#include "Items/NItem.h"
#include "Characters/Components/NInventoryComponent.h"
#include "Components/Image.h"


void UNInventoryItemWidget::SetInfo(FNInventorySlot& InSlot, UNInventoryComponent* InInventoryComponent)
{
    if (!InSlot.ItemData)
    {
        return;
    }

    if (DebugInventoryComponent)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s %s"), *FString(__FUNCTION__), *GetName(), *InSlot.ToString()));
    }

    bItemSlotted = InSlot.IsSlotted();
    
    NameText->SetText(FText::FromString(InSlot.ItemData->ItemName));
    DescriptionText->SetText(InSlot.ItemData->ItemDescription);
    CountText->SetText(FText::FromString(FString::FromInt(InSlot.SlotNumber)));
    // CountText->SetText(FText::FromString(InSlot.Count > 1 ? FString::FromInt(InSlot.Count) : ""));
    ImageUnderlay->SetColorAndOpacity(bItemSlotted ? EquippedColor : UnequippedColor);
    ItemImage->SetBrushFromTexture(InSlot.ItemData->InventoryIcon);
    InventorySlot = InSlot;
    InventoryComponent = InInventoryComponent;
    DropButton->OnClicked.AddDynamic(this, &UNInventoryItemWidget::DropItem);
    EquipButton->OnClicked.AddDynamic(this, &UNInventoryItemWidget::EquipItem);
}

void UNInventoryItemWidget::DropItem()
{
    InventoryComponent->OpenDropWidget(InventorySlot);
}

void UNInventoryItemWidget::EquipItem()
{
    if (!bItemSlotted)
    {
        InventoryComponent->SlotItem(InventorySlot);
        InventoryComponent->UpdateInventoryWidget();
    }
    
}
