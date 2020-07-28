// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NInventoryItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "NInventoryWidget.generated.h"

class UVerticalBox;
class UNInventoryComponent;
class UButton;
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
	bool bIsActive;
	bool bInitialized;
	
protected:
	/***/
	virtual void AddToScreen(ULocalPlayer* LocalPlayer, int32 ZOrder) override;
	
public:
	virtual void RemoveFromParent() override;
	
protected:

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemList;

	UPROPERTY(meta = (BindWidget))
    UVerticalBox* EquipmentList;
	
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY()
	UNInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<UNInventoryItemWidget> InventoryItemClass;

public:

	/** MUST CALL after CreateWidget<>(), before AddToParent() */
    void Initialize(UNInventoryComponent* InInventoryComponent);

	/** Updates the item list */
	UFUNCTION()
	void UpdateItems();

protected:

	/** Sets button bindings */
	void ActivateMenu();

	/** Removes button bindings */
	void DeactivateMenu();

	/** Updates the equipment list */
	UFUNCTION()
	void UpdateEquipment();
	
	UFUNCTION()
	void CloseInventory();

};
