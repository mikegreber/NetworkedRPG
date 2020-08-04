// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/NInventoryComponent.h"
#include "Components/Combat/NCombatComponent.h"
#include "NAssetManager.h"
#include "Items/Data/NItem.h"
#include "Items/Actors/NPickupActor.h"
#include "Player/NPlayerController.h"
#include "UI/Inventory/NDropItemWidget.h"
#include "UI/Inventory/NInventoryWidget.h"
#include "NetworkedRPG/NetworkedRPGGameMode.h"
#include "Net/UnrealNetwork.h"


// static int32 DebugInventoryComponent = 0;
FAutoConsoleVariableRef CVarDebugInventoryComponent(
    TEXT("NRPG.Debug.InventoryComponent"),
    DebugInventoryComponent,
    TEXT("Print debug information for InventoryComponent"),
    ECVF_Cheat);


// Sets default values for this component's properties
UNInventoryComponent::UNInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxSlots = 10;
}


void UNInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UNInventoryComponent, InventoryData, COND_OwnerOnly);
}


void UNInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNInventoryComponent::Initialize);
}


int32 UNInventoryComponent::AddInventoryItem(UNItem* NewItem, int32 ItemCount, int32 ItemLevel, bool bAutoSlot)
{
	if (!OwnerHasAuthority())
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s Failed: Tried to call on Client."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
		return ItemCount;
	}
	
	if (!NewItem)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s Failed: Tried to add null item."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
		return ItemCount;
	}

	if (ItemCount <= 0 || ItemLevel <= 0)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s Failed: trying to add %s with negative count or level."), *FString(__FUNCTION__), *GetName(), *NewItem->ItemName), EPrintType::Failure);
		return ItemCount;
	}
	
	if (NewItem->CanStack())
	{
		// See if we already have a stackable item slotted with room on its stack
		for (FNInventorySlot& Slot : InventoryData)
		{
			if (Slot.ItemData == NewItem && Slot.Count < Slot.ItemData->StackMaxCount)
			{					
				const int32 AddCount = FMath::Min(ItemCount, Slot.ItemData->StackMaxCount - Slot.Count);
				Slot.Count += AddCount;
				ItemCount -= AddCount;

				if (DebugInventoryComponent)
				{
					Print(GetWorld(), FString::Printf(TEXT("%s %s %d %ss stacked on existing inventory slot."), *FString(__FUNCTION__), *GetName(), AddCount, *NewItem->ItemName));
				}
				
				if (ItemCount == 0)
				{
					return ItemCount;
				}
			}
		}
	}
	
	if (ItemCount > 0)
	{
		const int32 CachedItemCount = ItemCount;
		FNInventorySlot& NewInventorySlot = AddNewItemSlot(NewItem,ItemCount, ItemLevel);

		if (DebugInventoryComponent)
		{
			if (ItemCount < CachedItemCount)
			{
				const int32 AddedCount = CachedItemCount - ItemCount;
				const int32 SlotCount = AddedCount / NewItem->StackMaxCount;
				Print(GetWorld(), FString::Printf(TEXT("%s %d slots added to inventory holding a total of %d %ss."), *FString(__FUNCTION__), SlotCount, AddedCount, *NewItem->ItemName));
			}
			
			if (ItemCount > 0)
			{
				Print(GetWorld(), FString::Printf(TEXT("%s Inventory Full: %d %ss could not be added to the inventory."), *FString(__FUNCTION__), ItemCount, *NewItem->ItemName), EPrintType::Warning);
			}
		}

		if (NewInventorySlot && bAutoSlot)
		{
			CombatComponent->AutoSlotItem(NewInventorySlot);
		}
	}

	return ItemCount;
}


void UNInventoryComponent::Initialize()
{
	PlayerController = Cast<APlayerController>(GetOwner());
	if (PlayerController)
	{
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			CombatComponent = Pawn->FindComponentByClass<UNCombatComponent>();
			CombatComponent->OnItemRemovedFromSlot.AddDynamic(this, &UNInventoryComponent::OnCombatItemRemovedFromSlot);
			
			if (PlayerController->IsLocalController())
			{
				PlayerController->InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &UNInventoryComponent::OpenInventory);
			}
		}
	}
}


void UNInventoryComponent::OpenInventory()
{
	if (!InventoryWidgetClass)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s InventoryWidgetClass not set in Blueprint."), *FString(__FUNCTION__)), EPrintType::ShutDown);
		return;
	}
    
	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UNInventoryWidget>(PlayerController, InventoryWidgetClass);
		InventoryWidget->Initialize(this);
	}

	if (InventoryWidget->IsInViewport())
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Inventory opened."), *FString(__FUNCTION__)), EPrintType::Log);
		}
		
		InventoryWidget->RemoveFromParent();
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
	else
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Inventory closed."), *FString(__FUNCTION__)), EPrintType::Log);
		}
		
		InventoryWidget->AddToViewport();
		InventoryWidget->Update();
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeGameAndUI());
	}
}


void UNInventoryComponent::OpenDropWidget(const FNInventorySlot& InventorySlot)
{
	if (!DropItemWidgetClass)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s DropItemWidgetClass not set in blueprint."), *FString(__FUNCTION__)), EPrintType::Error);
		return;	
	}
	
	UNDropItemWidget* DropItemWidget = CreateWidget<UNDropItemWidget>(Cast<APlayerController>(GetOwner()), DropItemWidgetClass);
	DropItemWidget->InitializeData(InventorySlot, this);
	DropItemWidget->AddToViewport();
}


void UNInventoryComponent::UpdateInventoryWidget() const
{
	if (InventoryWidget)
	{
		InventoryWidget->Update();
	}
}


FNInventorySlot& UNInventoryComponent::AddNewItemSlot(UNItem* NewItem, int32& ItemCount, int32 ItemLevel)
{
	if (MaxSlots <= InventoryData.Num())
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Inventory Full."), *FString(__FUNCTION__)), EPrintType::Warning);
		}
		return FNInventorySlot::NullSlot();
	}
	
	// Add the item to a new slot, up to as many as the StackMaxCount
	const int32 AddCount = FMath::Min(ItemCount, NewItem->StackMaxCount);
	InventoryData.Emplace(FNInventorySlot(NewItem, AddCount, InventoryData.Num()));
	ItemCount -= AddCount;
	
	// If count is greater than can fit in this slot, and we still have room in inventory, call again
	if (ItemCount > 0 && InventoryData.Num() < MaxSlots)
	{
		return AddNewItemSlot(NewItem, ItemCount, ItemLevel);	
	}
	
	return InventoryData.Last();
}


FNInventorySlot& UNInventoryComponent::FindInventorySlot(const int32& SlotNumber)
{
	if (SlotNumber > 0 && SlotNumber < InventoryData.Num())
	{
		return InventoryData[SlotNumber];
	}

	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Failed."), *FString(__FUNCTION__)), EPrintType::Warning);
	}

	return FNInventorySlot::NullSlot();
}


FNInventorySlot& UNInventoryComponent::FindInventorySlot(const FNInventorySlot& InventorySlot)
{
	{
		FNInventorySlot& Slot = FindInventorySlot(InventorySlot.SlotNumber);
		if (Slot == InventorySlot)  // Likely unnecessary check
		{
			return Slot;
		}
	}

	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Primary method failed."), *FString(__FUNCTION__)), EPrintType::Warning);
	}
	
	// Backup for now.. need to test more but the order of replicated TArrays should remain consistent so above method should work.
	for (auto& Slot : InventoryData)
	{
		if (Slot == InventorySlot)
		{
			return Slot;
		}
	}

	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Slot not found."), *FString(__FUNCTION__)), EPrintType::Error);
	}

	return FNInventorySlot::NullSlot();
}


void UNInventoryComponent::SlotItem(const FNInventorySlot& InventorySlot)
{
	// Only call on server
	if (GetOwnerRole() != ROLE_Authority)
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s called from client."), *FString(__FUNCTION__),  *InventorySlot.ItemData->ItemName), EPrintType::Log);
		}
		ServerSlotItem(InventorySlot);
		return;
	}

	FNInventorySlot& Slot = FindInventorySlot(InventorySlot);
	if (Slot)
	{
		CombatComponent->SlotItem(Slot);
	}
	else
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Failed. Invalid Slot."), *FString(__FUNCTION__)), EPrintType::Error);
	}
	
	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s"), *FString(__FUNCTION__),  *InventorySlot.ItemData->ItemName), EPrintType::Success);
	}
}


void UNInventoryComponent::DropItem(const FNInventorySlot& InventorySlot, const int32 Amount)
{	
	// Only run on server
	if (GetOwnerRole() != ROLE_Authority)
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Drop item called from client."), *FString(__FUNCTION__)), EPrintType::Log);
		}
		ServerDropItem(InventorySlot, Amount);
		return;
	}

	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Dropping %s"), *FString(__FUNCTION__),  *InventorySlot.ItemData->ItemName), EPrintType::Success);
	}

	const bool ItemRemoved = RemoveInventoryItem(InventorySlot, Amount);

	if (ItemRemoved)
	{
		SpawnItemPickup(InventorySlot.ItemData, Amount);
	}

	// will trigger on ListenServer only, handled in OnRep_InventoryData() on clients.
	UpdateInventoryWidget();
}


bool UNInventoryComponent::RemoveInventoryItem(const FNInventorySlot& InInventorySlot, int32 Amount)
{
	if (!OwnerHasAuthority())
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called from client."), *FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return false;
	}
	
	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
	}
	
	bool bItemRemoved = false;
	bool bSlotRemoved = false;
	int32 IndexToRemove = -1;

	//Iterate through entire array if slot is removed so we can update SlotNumbers
	for (int32 InventoryIndex = 0; InventoryIndex < InventoryData.Num(); ++InventoryIndex)
	{
		FNInventorySlot& Slot = InventoryData[InventoryIndex];
		
		if (bSlotRemoved)
		{
			// Must update SlotNumber is item is slotted.
			if (Slot.IsSlotted())
			{
				CombatComponent->UpdateSlotNumber(Slot, InventoryIndex - 1);	
			}
			
			Slot.SlotNumber -= 1;
		}
		else if (Slot.SlotNumber == InInventorySlot.SlotNumber)
		{
			bItemRemoved = true;
			
			if (InInventorySlot.Count > Amount)
			{
				Slot.Count -= Amount;	
				break;
			}

			IndexToRemove = InventoryIndex;
			bSlotRemoved = true;
		}
	}
	
	if (bSlotRemoved)
	{
		FNInventorySlot& ItemToRemove = InventoryData[IndexToRemove];
		if (ItemToRemove.IsSlotted())
		{
			CombatComponent->RemoveItemFromSlot(ItemToRemove);
		}
		
		InventoryData.RemoveAt(IndexToRemove);
	}

	if (DebugInventoryComponent && !bItemRemoved)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Failed. Item not removed."), *FString(__FUNCTION__)), EPrintType::Error);
	}

	return bItemRemoved;;
}


void UNInventoryComponent::SpawnItemPickup(UNItem* ItemData, int32 Count) const
{
	// Spawn the dropped item on the server
	if (!OwnerHasAuthority())
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called from client. Only call on Server."), *FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}

	if (!ItemData)
	{
		if (DebugInventoryComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Failed <UNItem* ItemData> was null."), *FString(__FUNCTION__)), EPrintType::Failure);
		}

		return;
	}

	if (DebugInventoryComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Spawning %s."), *FString(__FUNCTION__), *ItemData->ItemName), EPrintType::Log);
	}
	
	const FTransform SpawnTransform = Cast<ANPlayerController>(GetOwner())->GetPawn()->GetActorTransform();
	ANPickupActor* PickupActor = GetWorld()->SpawnActorDeferred<ANPickupActor>(Cast<ANetworkedRPGGameMode>(GetWorld()->GetAuthGameMode())->PickupItem, SpawnTransform);
	PickupActor->FinishSpawning(SpawnTransform);
	PickupActor->SetItem(ItemData, Count);
}


void UNInventoryComponent::OnCombatItemRemovedFromSlot(const FNInventorySlot& RemovedSlot)
{	
	FNInventorySlot& Slot = FindInventorySlot(RemovedSlot);
	if (Slot)
	{
		Slot.bIsSlotted = false;
	}

	if (DebugInventoryComponent)
	{
		if (Slot)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Inventory slot updated."), *FString(__FUNCTION__)), EPrintType::Log);
		}
		else
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Inventory slot not found."), *FString(__FUNCTION__)), EPrintType::Error);
		}
	}	
}


void UNInventoryComponent::OnRep_InventoryData() const
{
	UpdateInventoryWidget();
}


void UNInventoryComponent::ServerDropItem_Implementation(const FNInventorySlot& InventorySlot, int32 Count)
{
	DropItem(InventorySlot, Count);
}

bool UNInventoryComponent::ServerDropItem_Validate(const FNInventorySlot& InventorySlot, int32 Count)
{
	return true;
}


void UNInventoryComponent::ServerSlotItem_Implementation(const FNInventorySlot& InventorySlot)
{
	SlotItem(InventorySlot);
}

bool UNInventoryComponent::ServerSlotItem_Validate(const FNInventorySlot& InventorySlot)
{
	return true;
}