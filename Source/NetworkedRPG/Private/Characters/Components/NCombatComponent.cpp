// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/NCombatComponent.h"
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h"
#include "DrawDebugHelpers.h"
#include "Characters/Components/NCombatComponentInterface.h"
#include "Characters/Components/NMovementSystemComponent.h"
#include "Characters/Components/NSpringArmComponent.h"
#include "GameFramework/Character.h"
#include "Abilities/NAbilitySystemComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

#include "NAssetManager.h"
#include "Blueprint/UserWidget.h"
#include "Items/NEquipmentItem.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


FAutoConsoleVariableRef CVarDebugCombatComponent(
	TEXT("NRPG.Debug.CombatComponent"),
	DebugCombatComponent,
	TEXT("Show debug information for CombatComponent"),
	ECVF_Cheat
	);

// Sets default values for this component's properties
UNCombatComponent::UNCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	
	WeaponSlots.Emplace(FNWeaponSlot(UNAssetManager::WeaponItemType, ENItemSlotId::Ranged, FGameplayTag::RequestGameplayTag(FName("Weapon.Ranged.Equipped"))));
	WeaponSlots.Emplace(FNWeaponSlot(UNAssetManager::WeaponItemType, ENItemSlotId::Melee,  FGameplayTag::RequestGameplayTag(FName("Weapon.Melee.Equipped"))));

	ActiveWeaponSwapGameplayTag = FGameplayTag::RequestGameplayTag(FName("Weapon.ActiveWeaponSwap"));
	
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::Head));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::Neck));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::Torso));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::Waist));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::Legs));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::LeftRing));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::RightRing));
	ArmourSlots.Emplace(FNArmourSlot(UNAssetManager::ArmourItemType, ENItemSlotId::Feet));

	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability1));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability2));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability3));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability4));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability5));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability6));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability7));
	ItemSlots.Emplace(FNEquipmentSlot(UNAssetManager::ConsumableItemType, ENItemSlotId::Ability8));
	
	RangedCameraMode = FCameraModeSettings(FCameraMode(AimingCameraOffset, 20.f, 0.f, 200.f));	
	MeleeCameraMode = FCameraModeSettings(FCameraMode({50.f, 0.f, 80.f}, 4.f, 20.f, 200.f));

	// Targeting Defaults
	TargetingCameraMode = FCameraModeSettings(FCameraMode({0.f, 0.f, 80.f}, 5.f, 2.5f, 300.f ));
	TargetingCameraMode.Walking.CameraLagSpeed = 4.f;
	TargetingCameraMode.Walking.CameraRotationLagSpeed = 2.f;
	TargetingCameraMode.Walking.CameraLagMaxDistance = 200.f;
	
	MaxAngleForTargeting = 100.f;
	MaxTargetDetectionDistance = 1000.f;
	MaxTargetLockDistance = 1000.f;
	TargetingCameraPitchModifier = 17.f;

	CollisionObjectType = ECC_Targeting;

	SetIsReplicated(true);
}


void UNCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsLocked() && PlayerController->IsLocalController())
	{
		const FVector VectorToTarget = Target->GetComponentLocation() - GetEyesLocation();
		if (DebugCombatComponent)
		{
			DrawDebugLine(GetWorld(), Target->GetComponentLocation(), GetEyesLocation(), FColor::Red);
		}
		
		if (VectorToTarget.Size() > MaxTargetLockDistance)
		{
			SetTarget(nullptr);
		}
		else
		{
			// Keep camera locked on target
			FRotator TargetRotation = VectorToTarget.Rotation();
			TargetRotation.Pitch -= TargetingCameraPitchModifier;

			PlayerController->SetControlRotation(TargetRotation);
		}
	}
	else
	{
		UnLock();
	}
}


void UNCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNCombatComponent::Initialize);
}


void UNCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Equipment Slots
	DOREPLIFETIME(UNCombatComponent, WeaponSlots);
	DOREPLIFETIME(UNCombatComponent, ArmourSlots);

	// TODO make this only rep to owner once implemented
	DOREPLIFETIME(UNCombatComponent, ItemSlots);

	// Targeting system
	DOREPLIFETIME_CONDITION(UNCombatComponent, Target, COND_SkipOwner);
}


void UNCombatComponent::Initialize()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	Interface = Character;

	if (!IsValid(Character) || !Interface)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s CombatComponent Can only be added to an ACharacter that implements the INCombatComponentInterface."), *FString(__FUNCTION__)), EPrintType::ShutDown);
		return;
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Warning);
	}

	// Set references
	OwningCharacter = Character;
	PlayerController = Character->GetController<APlayerController>();
	SpringArm = Character->FindComponentByClass<UNSpringArmComponent>();
	Interface->GetMovementSystemComponent()->OnSystemUpdated.AddDynamic(this, &UNCombatComponent::OnMovementSystemUpdated);
	
	if (PlayerController && PlayerController->IsLocalPlayerController())
	{
		// Initialize targeting system
		CreateCollisionSphere();
		CreateTargetWidget();
		CreateAimingReticle();
	}

	if (MaxTargetLockDistance < MaxTargetDetectionDistance)
	{
		MaxTargetLockDistance = MaxTargetDetectionDistance;

		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s MaxTargetLockDistance should be greater or equal to MaxTargetDetectionDistance - Set values in blueprint."), *FString(__FUNCTION__), *OwningCharacter->GetName()), EPrintType::Warning);
		}
	}

	// Initialize all inventory slots
	for (auto& Slot : WeaponSlots)
	{
		Slot.Initialize(OwningCharacter);
	}

	for (auto& Slot : ArmourSlots)
	{
		Slot.Initialize(OwningCharacter);
	}

	for (auto& Slot : ItemSlots)
	{
		Slot.Initialize(OwningCharacter);
	}
	

	OnWeaponSwapping.AddDynamic(this, &UNCombatComponent::SetActiveWeaponSwap);
}


bool UNCombatComponent::SlotItem(FNInventorySlot& InventorySlot)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s called on client."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
		}
		
		return false;
	}
	
	if (!InventorySlot.ItemData)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s called with nullptr Item."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
		}
		
		return false;
	}

	FNInventorySlot ReplacedItem;
	bool Slotted = false;
	
	UNEquipmentItem* EquipmentItem = Cast<UNEquipmentItem>(InventorySlot.ItemData);
	if (EquipmentItem)
	{
		FNEquipmentSlot& Slot = FindSlot(EquipmentItem->SlotId, false);
		if (Slot)
		{
			ReplacedItem = Slot.GetItemCopy();
			Slotted = Slot.SlotItem(InventorySlot);
		}
	}

	if (ReplacedItem)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s ReplacedItem: %s."), *FString(__FUNCTION__), *GetName(), *ReplacedItem.ToString()), EPrintType::Warning);
		}
		
		OnItemRemovedFromSlot.Broadcast(ReplacedItem);
	}

	return Slotted;
}


bool UNCombatComponent::AutoSlotItem(FNInventorySlot& InventorySlot)
{
	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s."), *FString(__FUNCTION__)), EPrintType::Log);
	}
	
	UNEquipmentItem* NewItem = Cast<UNEquipmentItem>(InventorySlot.ItemData);
	
	if (!NewItem)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s <UNEquipmentItem* NewItem> was null."),*FString(__FUNCTION__), *GetName()), EPrintType::Failure);
		}
		
		return false;
	}

	if (NewItem->SlotId == ENItemSlotId::None)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s %s Tried adding item with no SlotId."),*FString(__FUNCTION__), *GetName()), EPrintType::Failure);
		}
		
		return false;
	}
	
	FNEquipmentSlot& Slot = FindSlot(NewItem->SlotId, true);
	if (Slot)
	{
		Slot.SlotItem(InventorySlot);
		return true;
	}

	return false;
}


bool UNCombatComponent::RemoveItemFromSlot(const ENItemSlotId& SlotId)
{
	if (!OwnerHasAuthority())
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Called on Client. Only call on server."),*FString(__FUNCTION__)), EPrintType::Warning);
		return false;
	}
	
	if (SlotId == ENItemSlotId::None)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called with invalid SlotId."), *FString(__FUNCTION__)), EPrintType::Error);
		}

		return false;
	}

	if (DebugCombatComponent)
	{
		const FString SlotIdAsString = UNEquipmentItem::SlotIdEnum ? UNEquipmentItem::SlotIdEnum->GetNameStringByIndex(static_cast<int32>(SlotId)) : FString("Error"); 
		Print(GetWorld(), FString::Printf(TEXT("%s SlotId: %s"), *FString(__FUNCTION__), *SlotIdAsString), EPrintType::Log);
	}

	FNInventorySlot RemovedItem;
	FNEquipmentSlot& SlotToRemove = FindSlot(SlotId, false);
	if (SlotToRemove)
	{
		RemovedItem = SlotToRemove.DeSlotItem();
	}

	if (RemovedItem)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s RemovedItem: %s."), *FString(__FUNCTION__), *RemovedItem.ToString()), EPrintType::Success);
		}
		
		OnItemRemovedFromSlot.Broadcast(RemovedItem);
		return true;
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Failed. Item not removed. %s"), *FString(__FUNCTION__), *RemovedItem.ToString()), EPrintType::Error);
	}

	return false;
}

bool UNCombatComponent::RemoveItemFromSlot(const FNInventorySlot& InventorySlot)
{
	if (!InventorySlot.ItemData)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s null ItemData in InventorySlot"), *FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return false;
	}
	
	UNEquipmentItem* EquipmentItem = Cast<UNEquipmentItem>(InventorySlot.ItemData);
	if (EquipmentItem)
	{
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called on item InventorySlot"), *FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return RemoveItemFromSlot(EquipmentItem->SlotId);
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Failed. Non UNEquipmentItem in slot."), *FString(__FUNCTION__)), EPrintType::Error);
	}

	return false;
}


bool UNCombatComponent::UpdateSlotNumber(const int32& OriginalSlotNumber, const int32 NewSlotNumber)
{
	if (!OwnerHasAuthority())
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Called on Client. Only call on server."),*FString(__FUNCTION__)), EPrintType::Warning);
		return false;
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);	
	}
	
	FNInventorySlot& Slot = FindSlot(OriginalSlotNumber);
	if (Slot)
	{
		Slot.SlotNumber = NewSlotNumber;
		return true;
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Failed to update"), *FString(__FUNCTION__)), EPrintType::Failure);	
	}

	return false;
}


bool UNCombatComponent::UpdateSlotNumber(const FNInventorySlot& InSlot, int32 NewSlotNumber)
{
	return UpdateSlotNumber(InSlot.SlotNumber, NewSlotNumber);
}


FNEquipmentSlot& UNCombatComponent::FindSlot(int32 SlotNumber, bool bEmptyOnly)
{
	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s SlotNumber: %d, bEmptyOnly: %s."), *FString(__FUNCTION__), SlotNumber, *FString(bEmptyOnly ? "true" : "false")), EPrintType::Log);
	}
	
	return FindSlot([&](const FNEquipmentSlot& Slot)
	{
		return Slot.SlotNumber == SlotNumber;
	},
	bEmptyOnly
	);
}

FNEquipmentSlot& UNCombatComponent::FindSlot(const ENItemSlotId SlotId, bool bEmptyOnly)
{
	if (DebugCombatComponent)
	{
		const FString SlotIdAsString = UNEquipmentItem::SlotIdEnum ? UNEquipmentItem::SlotIdEnum->GetNameStringByIndex(static_cast<int32>(SlotId)) : FString("Error"); 
		Print(GetWorld(), FString::Printf(TEXT("%s SlotId: %s, bEmptyOnly: %s."), *FString(__FUNCTION__), *SlotIdAsString, *FString(bEmptyOnly ? "true" : "false")), EPrintType::Log);
	}
	
	return FindSlot([&](const FNEquipmentSlot& Slot)
    {
        return Slot.SlotId == SlotId;
    },
    bEmptyOnly
    );
}

FNEquipmentSlot& UNCombatComponent::FindSlot(const FNInventorySlot& InSlot, bool bEmptyOnly)
{
	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s InSlot: %s, bEmptyOnly: %s."), *FString(__FUNCTION__), *InSlot.ToString(), *FString(bEmptyOnly ? "true" : "false")), EPrintType::Log);
	}
	
	return FindSlot([&](const FNEquipmentSlot& Slot)
    {
		return Slot == InSlot;
    },
    bEmptyOnly
    );
}


FNEquipmentSlot& UNCombatComponent::FindSlot(TFunctionRef<bool(const FNEquipmentSlot& Slot)> Predicate, bool bEmptyOnly)
{
	bool FoundMatchingSlot = false;

	{
		auto& Slot = SearchSlots<FNWeaponSlot>(WeaponSlots, FoundMatchingSlot, Predicate, bEmptyOnly);
		if (Slot) return Slot;
	}
	
	{
		auto& Slot = SearchSlots<FNArmourSlot>(ArmourSlots, FoundMatchingSlot, Predicate, bEmptyOnly);
		if (Slot) return Slot;
	}
	
	{
		auto& Slot = SearchSlots<FNEquipmentSlot>(ItemSlots, FoundMatchingSlot, Predicate, bEmptyOnly);
		if (Slot) return Slot;
	}

	if (DebugCombatComponent)
	{
		if (FoundMatchingSlot && bEmptyOnly)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s No empty slot found."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		else
		{
			Print(GetWorld(), FString::Printf(TEXT("%s No matching slot found."),*FString(__FUNCTION__)), EPrintType::Failure);
		}
	}

	return FNEquipmentSlot::NullSlot();
}


FNWeaponSlot& UNCombatComponent::FindWeapon(const ENItemSlotId SlotId)
{
	bool Found;
	return SearchSlots<FNWeaponSlot, FNWeaponSlot>(WeaponSlots, Found,
		[&](const FNWeaponSlot& Slot)
    {
        return Slot.SlotId == SlotId;
    });
}


void UNCombatComponent::OnRep_WeaponSlots(TArray<FNWeaponSlot> OldWeaponSlots)
{
	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)));
	}
	
	for (int32 SlotIndex = 0; SlotIndex < FMath::Min(WeaponSlots.Num(),OldWeaponSlots.Num()); SlotIndex ++)
	{
		UpdateWeaponSlot(WeaponSlots[SlotIndex], OldWeaponSlots[SlotIndex]);
	}

	OnWeaponSlotsUpdated.Broadcast();
}


void UNCombatComponent::OnRep_ArmourSlots(TArray<FNArmourSlot> OldArmourSlots)
{
	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)));
	}
	
	for (int32 SlotIndex = 0; SlotIndex < FMath::Min(ArmourSlots.Num(),OldArmourSlots.Num()); SlotIndex ++)
	{
		UpdateArmourSlot(ArmourSlots[SlotIndex], OldArmourSlots[SlotIndex]);
	}

	OnArmourSlotsUpdated.Broadcast();
}


void UNCombatComponent::OnRep_ItemSlots(TArray<FNEquipmentSlot> OldItemSlots)
{
	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)));
	}
	
	for (int32 SlotIndex = 0; SlotIndex < FMath::Min(ItemSlots.Num(),OldItemSlots.Num()); SlotIndex ++)
	{
		UpdateItemSlot(ItemSlots[SlotIndex], OldItemSlots[SlotIndex]);
	}

	OnItemSlotsUpdated.Broadcast();
}


void UNCombatComponent::UpdateWeaponSlot(FNWeaponSlot& WeaponSlot, FNWeaponSlot& OldWeaponSlot) const
{
	// New ItemData
	if (WeaponSlot.ItemData != OldWeaponSlot.ItemData)
	{
		if (WeaponSlot.ItemData)
		{
			WeaponSlot.SlotItem(WeaponSlot);
		}
		else
		{
			WeaponSlot.ClearSlot();
			return;
		}
	}

	// If Just slotted from empty, play holster animation
	if (WeaponSlot.IsSlotted() && !OldWeaponSlot.IsSlotted())
	{
		WeaponSlot.Holster();
	}

	// Only on remote machines, trigger when there is a new equipped state. This is already handled locally for local player.
	if (!OwningCharacter->IsLocallyControlled() && (WeaponSlot.IsEquipped() != OldWeaponSlot.IsEquipped()))
	{
		if (WeaponSlot.IsEquipped())
		{
			WeaponSlot.Equip();
		}
		else
		{
			WeaponSlot.Holster();
		}
	}
}


void UNCombatComponent::UpdateArmourSlot(FNArmourSlot& ArmourSlot, FNArmourSlot& OldArmourSlot) const
{
	// New ItemData
	if (ArmourSlot.ItemData != OldArmourSlot.ItemData)
	{
		if (ArmourSlot.ItemData)
		{
			ArmourSlot.SlotItem(ArmourSlot);
		}
		else
		{
			ArmourSlot.DeSlotItem();
		}
	}
}


void UNCombatComponent::UpdateItemSlot(FNEquipmentSlot& ItemSlot, FNEquipmentSlot& OldItemSlot) const
{
	if (ItemSlot.ItemData != OldItemSlot.ItemData)
	{
		if (ItemSlot.ItemData)
		{
			ItemSlot.SlotItem(ItemSlot);
		}
		else
		{
			ItemSlot.DeSlotItem();
		}
	}
}


USkeletalMeshComponent* UNCombatComponent::GetOwnerMesh() const
{
	return OwningCharacter->GetMesh();
}


USkeletalMeshComponent* UNCombatComponent::GetRangedWeaponMesh()
{
	return FindWeapon(ENItemSlotId::Ranged).GetWeaponMesh();
}


USkeletalMeshComponent* UNCombatComponent::GetMeleeWeaponMesh()
{
	return FindWeapon(ENItemSlotId::Melee).GetWeaponMesh();
}


void UNCombatComponent::ActivateWeapon()
{
	USkeletalMeshComponent* WeaponMesh = GetMeleeWeaponMesh();
	if (WeaponMesh)
	{
		WeaponMesh->SetGenerateOverlapEvents(true);
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s"),*FString(__FUNCTION__), *FString(WeaponMesh ? "" : " Failed.")), WeaponMesh ? EPrintType::Log : EPrintType::Failure);	
	}
}


void UNCombatComponent::DeactivateWeapon()
{
	USkeletalMeshComponent* WeaponMesh = GetMeleeWeaponMesh();
	if (WeaponMesh)
	{
		WeaponMesh->SetGenerateOverlapEvents(false);
	}
	
	ClearHits();

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s"),*FString(__FUNCTION__), *FString(WeaponMesh ? "" : " Failed.")), WeaponMesh ? EPrintType::Log : EPrintType::Failure);	
	}
}


void UNCombatComponent::ClearHits()
{
	HitActors.Reset();
}


bool UNCombatComponent::IsWeaponHolstered(ENItemSlotId SlotId)
{
	return FindWeapon(SlotId).IsHolstered();
}

bool UNCombatComponent::IsWeaponEquipped(ENItemSlotId SlotId)
{
	return FindWeapon(SlotId).IsEquipped();
}

void UNCombatComponent::EquipWeapon(ENItemSlotId SlotId)
{
	if (bActiveWeaponChange)
	{
		return;
	}
	
	bool bSuccess = false;

	FNWeaponSlot& Slot = FindWeapon(SlotId);
	if (Slot.IsHolstered())
	{
		Slot.Equip();
		bSuccess = true;
	}

	if (!OwnerHasAuthority() && bSuccess)
	{
		ServerEquipWeapon(SlotId);
	}
}


void UNCombatComponent::HolsterWeapon(ENItemSlotId SlotId)
{
	if (bActiveWeaponChange)
	{
		return;
	}
	
	bool bSuccess = false;
	
	FNWeaponSlot& Slot = FindWeapon(SlotId);
	if (Slot.IsEquipped())
	{
		Slot.Holster();
		bSuccess = true;
	}

	if (!OwnerHasAuthority() && bSuccess)
	{
		ServerHolsterWeapon(SlotId);
	}
}


void UNCombatComponent::CreateAimingReticle()
{
	if (PlayerController->IsLocalController() && AimingReticleClass)
	{
		AimingReticle = CreateWidget<UUserWidget>(PlayerController, AimingReticleClass);
		AimingReticle->AddToViewport();
		AimingReticle->SetVisibility(ESlateVisibility::Hidden);
	}
}


void UNCombatComponent::UpdateCombatState(ENCombatType InCombatType, ENCombatMode InCombatMode)
{
	ActiveCombatType = InCombatType;
	
	UpdateCameraMode();
	
	Interface->GetMovementSystemComponent()->SetRotationMode(
        InCombatType == ENCombatType::Ranged ?
        ENRotationMode::LookingDirection :
        ENRotationMode::VelocityDirection);

	Interface->GetMovementSystemComponent()->SetCombatMode(InCombatMode);
	
	if (AimingReticle)
	{
		AimingReticle->SetVisibility(
            InCombatType == ENCombatType::Ranged ?
            ESlateVisibility::Visible :
            ESlateVisibility::Hidden);
	}	
}


void UNCombatComponent::UpdateCameraMode() const
{
	// TODO try change implementation so locked is a combat type
	// 
	// Set appropriate camera and rotation modes
	if (IsLocked())
	{
		Interface->GetMovementSystemComponent()->SetCameraMode(TargetingCameraMode);
	}
	else
	{
		switch(ActiveCombatType)
		{
		case ENCombatType::None:
			Interface->GetMovementSystemComponent()->SetCameraModeToDefault();
			break;
		case ENCombatType::Melee:
			Interface->GetMovementSystemComponent()->SetCameraMode(MeleeCameraMode);
			break;
		case ENCombatType::Ranged:
			Interface->GetMovementSystemComponent()->SetCameraMode(RangedCameraMode);
			break;
		}
	}
}


void UNCombatComponent::OnWeaponCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != GetOwner() && !HitActors.Contains(OtherActor))
	{	
		HitActors.Add(OtherActor);
		OnWeaponHit.Broadcast(OtherActor, bFromSweep, SweepResult);

		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
		}
	}
}


void UNCombatComponent::OnWeaponStateChange(ENCombatMode InCombatMode)
{
	// TODO
	Print(GetWorld(), FString::Printf(TEXT("%s Not implemented"), *FString(__FUNCTION__)), EPrintType::Warning);
}


void UNCombatComponent::SetActiveWeaponSwap(bool InActiveWeaponSwap)
{
	bActiveWeaponChange = InActiveWeaponSwap;
	UNAbilitySystemComponent* ASC = Interface->GetNAbilitySystemComponent();
	if (ASC)
	{
		if (bActiveWeaponChange)
		{
			ASC->AddLooseGameplayTag(ActiveWeaponSwapGameplayTag);
		}
		else
		{
			ASC->RemoveLooseGameplayTag(ActiveWeaponSwapGameplayTag);
		}
		
		if (DebugCombatComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);	
		}
	}
}


bool UNCombatComponent::IsLocked() const
{
	return IsValid(Target);
}


AActor* UNCombatComponent::GetTargetActor() const
{
	return IsValid(Target) ? Target->GetOwner() : nullptr;
}



FVector UNCombatComponent::GetTargetLocation() const
{
	return IsValid(Target) ? Target->GetComponentLocation() : FVector();
}


FVector UNCombatComponent::GetEyesLocation() const
{
	FVector OutLocation;
	FRotator OutRotation;
	OwningCharacter->GetActorEyesViewPoint(OutLocation, OutRotation);
	return OutLocation;
}


void UNCombatComponent::ToggleTargetLock()
{
	if (IsLocked())
	{
		UnLock(); 
	}
	else if (FindSortedTarget())
	{
		SetComponentTickEnabled(true);
	}
	else
	{
		// Set camera rotation to be actor forward vector if no target is found
		PlayerController->SetControlRotation(GetOwner()->GetActorForwardVector().Rotation());
	}
}


void UNCombatComponent::SwitchTargetToRight()
{
	if (IsLocked())
	{
		FindSortedTarget([](float AngleToTurn, float ClosestAngele)
        {
            return AngleToTurn < 0 && -AngleToTurn < FMath::Abs(ClosestAngele);
        });
	}
}


void UNCombatComponent::SwitchTargetToLeft()
{
	if (IsLocked())
	{
		FindSortedTarget([](float AngleToTurn, float ClosestAngele)
        {
            return AngleToTurn > 0 && AngleToTurn < ClosestAngele;
        });	
	}		
}


void UNCombatComponent::UnLock()
{	
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerUnLock();
	}	

	SetTarget(nullptr);

	// Call on local player only
	if (PlayerController && PlayerController->IsLocalController())
	{
		if (SpringArm)
		{
			// Set control rotation to where the camera is currently faced so we don't get a jump when we go back to the regular camera rotation lag
			const FRotator CameraWorldRotation = GetOwner()->GetActorRotation() + SpringArm->GetRelativeSocketRotation().Rotator();
			PlayerController->SetControlRotation(CameraWorldRotation);
		}
		
		// This will update the rotation mode via OnMovementSystemUpdated()
		Interface->GetMovementSystemComponent()->BroadcastSystemState();
		
		UpdateCameraMode();
	
		// Attach target indicator widget to owner and hide it
		if (TargetIndicatorWidgetComponent)
		{
			TargetIndicatorWidgetComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetIndicatorWidgetComponent->SetHiddenInGame(true);	
		}
	}

	SetComponentTickEnabled(false);

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s"), *FString(__FUNCTION__), *GetName()), EPrintType::Log);
	}
}


void UNCombatComponent::CreateCollisionSphere()
{
	TargetingCollisionComponent = NewObject<USphereComponent>(this);
	TargetingCollisionComponent->RegisterComponent();
	TargetingCollisionComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	TargetingCollisionComponent->SetSphereRadius(MaxTargetDetectionDistance);
	TargetingCollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	TargetingCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetingCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TargetingCollisionComponent->SetCollisionResponseToChannel(ECC_Target, ECR_Overlap);
	TargetingCollisionComponent->SetCollisionObjectType(CollisionObjectType);

	if (DebugCombatComponent)
	{
		TargetingCollisionComponent->SetVisibility(true);
		TargetingCollisionComponent->ShapeColor = FColor::Red;
		TargetingCollisionComponent->bHiddenInGame = false;	
	}
}


void UNCombatComponent::CreateTargetWidget()
{
	if (TargetIndicatorWidgetClass)
	{
		TargetIndicatorWidget = CreateWidget<UUserWidget>(PlayerController, TargetIndicatorWidgetClass);
		TargetIndicatorWidgetComponent = NewObject<UWidgetComponent>(this);
		TargetIndicatorWidgetComponent->RegisterComponent();
		TargetIndicatorWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		TargetIndicatorWidgetComponent->SetWidget(TargetIndicatorWidget);
		TargetIndicatorWidgetComponent->SetHiddenInGame(true);
	}
	else
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s Missing TargetIndicatorWidgetClass - Set value in blueprint."), *FString(__FUNCTION__), *GetName()), EPrintType::ShutDown);
    }
}


void UNCombatComponent::SetTarget(UPrimitiveComponent* InTarget)
{
	if (Target != InTarget)
	{
		Target = InTarget;

		OnTargetUpdated.Broadcast(InTarget);
	}
}


void UNCombatComponent::Lock(UPrimitiveComponent* TargetToLock)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerLock(TargetToLock);
	}

	// Call on server and local player if client
	SetTarget(TargetToLock);

	// Call on local player only
	if (PlayerController && PlayerController->IsLocalController())
	{
		// This will update the rotation mode via OnMovementSystemUpdated()
		Interface->GetMovementSystemComponent()->BroadcastSystemState();
		
		UpdateCameraMode();
		
		// Attach target indicator widget to Target and show it
		if (TargetIndicatorWidgetComponent)
		{
			TargetIndicatorWidgetComponent->AttachToComponent(Target, FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetIndicatorWidgetComponent->SetHiddenInGame(false);
		}
		else
		{
			if (DebugCombatComponent)
			{
				Print(GetWorld(), FString::Printf(TEXT("%s %s TargetIndicatorWidgetComponent not set. Must call UNCombatComponent::CreateTargetWidget() in component initialization."), *FString(__FUNCTION__), *GetName()), EPrintType::Error);
			}
		}
	}

	if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s"), *FString(__FUNCTION__), *GetName()), EPrintType::Log);
	}
}


TArray<UPrimitiveComponent*> UNCombatComponent::GetAvailableTargets() const
{
	TArray<UPrimitiveComponent*> Targets;
	if (TargetingCollisionComponent)
	{
		TargetingCollisionComponent->GetOverlappingComponents(Targets);
	}
	return Targets;
}


bool UNCombatComponent::FindSortedTarget(TFunctionRef<bool(float AngleToTurn, float ClosestAngle)> Predicate)
{
	const TArray<UPrimitiveComponent*> Targets = GetAvailableTargets();

	float ClosestAngle = MaxAngleForTargeting;
	UPrimitiveComponent* NewTarget = nullptr;

	// Find the target with the lowest turning angle that is less than the MaxAngleForTargeting, configured with the Predicate
	for (UPrimitiveComponent* InTarget : Targets)
	{
		if (InTarget != Target)
		{
			const FRotator RequiredRotation = (InTarget->GetComponentLocation() - GetOwnerLocation()).Rotation();
			const FRotator AngleToTurn = (GetControlRotation() - RequiredRotation).GetNormalized(); // Makes angle -180 to 180
			 
			if (Predicate(AngleToTurn.Yaw, ClosestAngle))
			{
				ClosestAngle = AngleToTurn.Yaw;
				NewTarget = InTarget;
			}
		}
	}

	if (NewTarget)
	{
		Lock(NewTarget);
	}
	else if (DebugCombatComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s %s did not find new target."), *FString(__FUNCTION__), *GetName()), EPrintType::Log);
	}
	
	return NewTarget != nullptr;
}


FVector UNCombatComponent::GetOwnerLocation() const
{
	return GetOwner()->GetActorLocation();
}


FRotator UNCombatComponent::GetControlRotation() const
{
	return PlayerController->GetControlRotation();
}


void UNCombatComponent::OnMovementSystemUpdated(ENMovementGait InMovementMode, ENRotationMode InRotationMode, ENCombatMode InCombatMode)
{
	// If either locked melee or ranged CombatType, use looking direction rotation mode, otherwise use velocity direction rotation mode
	if (IsLocked() && ActiveCombatType == ENCombatType::Melee || ActiveCombatType == ENCombatType::Ranged)
	{
		if (InRotationMode != ENRotationMode::LookingDirection)
		{
			Interface->GetMovementSystemComponent()->SetRotationMode(ENRotationMode::LookingDirection);
		}
	}
	else if (InRotationMode != ENRotationMode::VelocityDirection)
	{
		Interface->GetMovementSystemComponent()->SetRotationMode(ENRotationMode::VelocityDirection);
	}	
}


void UNCombatComponent::ServerLock_Implementation(UPrimitiveComponent* TargetToLock)
{
	Lock(TargetToLock);
}

bool UNCombatComponent::ServerLock_Validate(UPrimitiveComponent* TargetToLock)
{
	return true;
}


void UNCombatComponent::ServerUnLock_Implementation()
{
	UnLock();
}

bool UNCombatComponent::ServerUnLock_Validate()
{
	return true;
}


void UNCombatComponent::ServerEquipWeapon_Implementation(ENItemSlotId SlotId)
{
	EquipWeapon(SlotId);
}


void UNCombatComponent::ServerHolsterWeapon_Implementation(ENItemSlotId SlotId)
{
	HolsterWeapon(SlotId);
}