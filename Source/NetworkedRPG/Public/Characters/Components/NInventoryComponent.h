// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "NInventoryTypes.h"
#include "Components/ActorComponent.h"
#include "NInventoryComponent.generated.h"


int32 DebugInventoryComponent = 0;

class UNItem;
class UNDropItemWidget;
class UNInventoryWidget;
class UNCombatComponent;

/** Sections
* 	1. Blueprint Settings
* 	2. State
* 	3. References
* 	4. Overrides
* 	5. Interface
* 	6. Protected Methods
* 	7. RPC's
*/

/** Inventory component. Should be added to a player controller to keep the inventory persistent when characters die. */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKEDRPG_API UNInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UNInventoryWidget;
	friend class UNInventoryItemWidget;
	friend class UNDropItemWidget;

protected:
  /*****************************/
 /**  1. Blueprint Settings  **/
/*****************************/		
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxSlots;

	UPROPERTY(EditAnywhere, Category = "Inventory|UI")
	TSubclassOf<UNDropItemWidget> DropItemWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="Inventory|UI")
	TSubclassOf<UNInventoryWidget> InventoryWidgetClass;


  /*****************************/
 /**       2. State          **/
/*****************************/
	/** Holds all the inventory slots */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryData, VisibleAnywhere, Category = "Inventory")
	TArray<FNInventorySlot> InventoryData;
	

  /*****************************/
 /**     3. References       **/
/*****************************/	
private:
	/** Reference for equipping items*/
	UPROPERTY()
	UNCombatComponent* CombatComponent;
	UPROPERTY()
	APlayerController* PlayerController;
	UPROPERTY()
	UNInventoryWidget* InventoryWidget;

	
  /*****************************/
 /**     4. Overrides        **/
/*****************************/	
public:
	/** Replicates InventoryData. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Called when component is created */
	virtual void BeginPlay() override;


  /*****************************/
 /**     5. Interface        **/
/*****************************/	
public:	
    // Sets default values for this component's properties
    UNInventoryComponent();
	
	/** [server] Tries to add the item to the inventory. Will stack if stacking space is available.
	  * Will make new slots if no stacking space is available, and there is room in the inventory.
	  * Returns the number of items that could not be added (0 if all were added). */
	int32 AddInventoryItem(UNItem* NewItem, int32 ItemCount = 1, int32 ItemLevel = 1, bool bAutoSlot = true);


  /*****************************/
 /**  6. Protected Methods   **/
/*****************************/	
protected:
	/** Gets reference to the controlled players CombatComponent */
	void Initialize();

	/** [local] Opens the inventory. */
	void OpenInventory();

	/** [local] Opens the drop widget to select amount to drop */
	void OpenDropWidget(const FNInventorySlot& InventorySlot);

	/** [local] Populates the inventory widget with the current items. */
	void UpdateInventoryWidget() const;
	
	/** Creates a new slot and adds the item, or multiple slots if needed if there is room in the inventory.
	  * Returns a copy of the added slot (or the last added slot if multiple slots are added). */
    FNInventorySlot& AddNewItemSlot(UNItem* NewItem, int32& ItemCount, int32 ItemLevel = 1);

	/** Returns reference to the slot matching the input inventory slot */
	FNInventorySlot& FindInventorySlot(const int32& SlotNumber);
	FNInventorySlot& FindInventorySlot(const FNInventorySlot& InventorySlot);
	
	/** [local] Slots an item in the CombatComponent if we are the server, or calls server function. */
	void SlotItem(const FNInventorySlot& InventorySlot);

	/* [local] Removes the input Amount of the item in the Slot. Removes the Slot entirely if it is empty. Calls server function if we are client.
	* Spawns a pickup item on the server.*/
	void DropItem(const FNInventorySlot& InventorySlot, int32 Amount);

	/** [server] Removes the input Amount of the item in the Slot. Returns false if it fails. */
	bool RemoveInventoryItem(const FNInventorySlot& InInventorySlot, int32 Amount);
	
	/** [server] Spawns an item pickup */
	void SpawnItemPickup(UNItem* ItemData, int32 Count) const;

	/** Sets bIsSlotted to false on the corresponding InventorySlot. */
	UFUNCTION()
    void OnCombatItemRemovedFromSlot(const FNInventorySlot& RemovedSlot);
	
	/** Calls UpdateInventoryWidget(). */
	UFUNCTION()
    void OnRep_InventoryData() const;

	
  /*****************************/
 /**        7. RPC's         **/
/*****************************/	
	/** Calls DropItem().
	  * Removes the item from the servers version of the inventory and spawns a pickup actor. Will replicate inventory to owning client, and spawn a pickup on the server. */
	UFUNCTION(Server, Reliable, WithValidation)
    void ServerDropItem(const FNInventorySlot& InventorySlot, int32 Count);
	void ServerDropItem_Implementation(const FNInventorySlot& InventorySlot, int32 Count);
	bool ServerDropItem_Validate(const FNInventorySlot& InventorySlot, int32 Count);

	/** Calls SlotItem()
	  * Result will be replicate to all clients. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSlotItem(const FNInventorySlot& InventorySlot);
	void ServerSlotItem_Implementation(const FNInventorySlot& InventorySlot);
	bool ServerSlotItem_Validate(const FNInventorySlot& InventorySlot);
};
