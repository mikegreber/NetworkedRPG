// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/NInventoryWidget.h"
#include "Components/Inventory/NInventoryComponent.h"
#include "UI/Inventory/NInventoryItemWidget.h"
#include "Components/Combat/NCombatComponent.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"


void UNInventoryWidget::AddToScreen(ULocalPlayer* LocalPlayer, int32 ZOrder)
{
    if (!bInitialized)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s Widget not initialized, must call Initialize() after CreateWidget to set InventoryComponent reference."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        return;
    }
    
    if (!bIsActive)
    {
        bIsActive = true;
        ActivateMenu();
    }

    Super::AddToScreen(LocalPlayer, ZOrder);
   
}

void UNInventoryWidget::RemoveFromParent()
{
    if (bIsActive)
    {
        bIsActive = false;
        DeactivateMenu();
    }
    
    Super::RemoveFromParent();
}


void UNInventoryWidget::Initialize(UNInventoryComponent* InInventoryComponent)
{
    if (!InInventoryComponent)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s called with null <UNInventoryComponent* InInventoryComponent>."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        return;
    }
    
    InventoryComponent = InInventoryComponent;
    InventoryComponent->CombatComponent->OnWeaponSlotsUpdated.AddDynamic(this, &UNInventoryWidget::UpdateEquipment);
    bInitialized = true;
}

void UNInventoryWidget::ActivateMenu()
{
    CloseButton->OnClicked.AddDynamic(this, &UNInventoryWidget::CloseInventory);
    RefreshButton->OnPressed.AddDynamic(this, &UNInventoryWidget::Update);
}

void UNInventoryWidget::DeactivateMenu()
{
    CloseButton->OnClicked.RemoveAll(this);
    RefreshButton->OnPressed.RemoveAll(this);
}

void UNInventoryWidget::CloseInventory()
{
    GetOwningPlayer()->bShowMouseCursor = false;
    GetOwningPlayer()->SetInputMode(FInputModeGameOnly());
    DeactivateMenu();
    
    RemoveFromParent();
}

void UNInventoryWidget::Update()
{
    if (!InventoryItemClass)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s Must set InventoryItemClass in blueprint."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        return;
    }

    if (!IsValid(InventoryComponent))
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s InventoryComponent is invalid. Must call Initialize()"), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        return;
    }

    if (DebugInventoryComponent)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s"), *FString(__FUNCTION__), *GetName()));
    }
    
    ItemList->ClearChildren();
    for (auto &InSlot : InventoryComponent->InventoryData)
    {
        UNInventoryItemWidget* InventoryItem = CreateWidget<UNInventoryItemWidget>(this, InventoryItemClass);
        InventoryItem->Initialize(InSlot, InventoryComponent);
        ItemList->AddChildToVerticalBox(InventoryItem);
    }

    UpdateEquipment();
}

void UNInventoryWidget::UpdateEquipment()
{
    if (!InventoryItemClass)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s Must set InventoryItemClass in blueprint."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        return;
    }

    if (!IsValid(InventoryComponent))
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s InventoryComponent is invalid. Must call Initialize()."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        return;
    }

    if (DebugInventoryComponent)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s"), *FString(__FUNCTION__), *GetName()));
    }
    
    EquipmentList->ClearChildren();
    for (auto &InSlot : InventoryComponent->CombatComponent->WeaponSlots)
    {
        if (InSlot.IsSlotted())
        {
            UNInventoryItemWidget* InventoryItem = CreateWidget<UNInventoryItemWidget>(this, InventoryItemClass);
            InventoryItem->Initialize(InSlot, InventoryComponent);
            EquipmentList->AddChildToVerticalBox(InventoryItem);
        }
    }

    for (auto &InSlot : InventoryComponent->CombatComponent->ArmourSlots)
    {
        if (InSlot.IsSlotted())
        {
            UNInventoryItemWidget* InventoryItem = CreateWidget<UNInventoryItemWidget>(this, InventoryItemClass);
            InventoryItem->Initialize(InSlot, InventoryComponent);
            EquipmentList->AddChildToVerticalBox(InventoryItem);
        }
    }
}


