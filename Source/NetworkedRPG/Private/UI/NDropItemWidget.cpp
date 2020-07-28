// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/NDropItemWidget.h"

#include "Characters/Components/NInventoryComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UNDropItemWidget::Initialize(const FNInventorySlot& InInventorySlot, UNInventoryComponent* InInventoryComponent)
{
    InventorySlot = InInventorySlot;
    InventoryComponent = InInventoryComponent;
    Max = InventorySlot.Count;
    Count = 1;
    CountText->SetText(FText::FromString(FString::FromInt(Count)));
    
    UpButton->OnClicked.AddDynamic(this, &UNDropItemWidget::OnUpClicked);
    DownButton->OnClicked.AddDynamic(this, &UNDropItemWidget::OnDownClicked);
    CancelButton->OnClicked.AddDynamic(this, &UNDropItemWidget::OnCancelClicked);
    ConfirmButton->OnClicked.AddDynamic(this, &UNDropItemWidget::OnConfirmClicked);
}

void UNDropItemWidget::OnUpClicked()
{
    if (Count < Max)
    {
        Count++;
    }

    CountText->SetText(FText::FromString(FString::FromInt(Count)));
}

void UNDropItemWidget::OnDownClicked()
{
    if (Count > 1)
    {
        Count--;
    }

    CountText->SetText(FText::FromString(FString::FromInt(Count)));
}

void UNDropItemWidget::OnCancelClicked()
{
    RemoveFromParent();
}

void UNDropItemWidget::OnConfirmClicked()
{
    InventoryComponent->DropItem(InventorySlot, Count);
    RemoveFromParent();
}
