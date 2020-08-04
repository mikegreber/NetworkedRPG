// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Inventory/NInventoryTypes.h"
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

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Indicator color of widget when item is equipped. */
	UPROPERTY(EditAnywhere, Category = "Settings|Appearance")
	FLinearColor EquippedColor;

	/** Indicator color of widget when item is not equipped. */
	UPROPERTY(EditAnywhere, Category = "Settings|Appearance")
	FLinearColor UnequippedColor;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. References and State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UPROPERTY()
	UNInventoryComponent* InventoryComponent;
	
	/** The slot represented by this widget. */
	FNInventorySlot InventorySlot;

	/** Is the slotted in the combat component? */
	bool bItemSlotted;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. WidgetComponents
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
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
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Interface and Callbacks
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Sets the properties of this widget. Must be called after CreateWidget(). */
	void Initialize(FNInventorySlot& InSlot, UNInventoryComponent* InInventoryComponent);

protected:
	/** Opens a DropItem Widget */
	UFUNCTION()
	void DropItem();

	/** Equips this item to the combat component. */
	UFUNCTION()
	void EquipItem();
};
