// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NInventoryTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"

#include "NInventoryItemWidget.generated.h"

class UTextBlock;
class UImage;
class UNInventoryComponent;

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UNInventoryComponent* InventoryComponent;

	FNInventorySlot InventorySlot;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
    UTextBlock* DescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountText;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemImage;

	UPROPERTY(meta = (BindWidget))
	UImage* ImageUnderlay;
	
	UPROPERTY(meta = (BindWidget))
	UButton* EquipButton;

	UPROPERTY(meta = (BindWidget))
    UButton* DropButton;

	UPROPERTY(EditAnywhere, Category = "Appearance|InventoryItem")
	FLinearColor EquippedColor;

	UPROPERTY(EditAnywhere, Category = "Appearance|InventoryItem")
	FLinearColor UnequippedColor;
	
	bool bItemSlotted;

	void SetInfo(FNInventorySlot& InSlot, UNInventoryComponent* InInventoryComponent);

	UFUNCTION()
	void DropItem();

	UFUNCTION()
    void EquipItem();
};
