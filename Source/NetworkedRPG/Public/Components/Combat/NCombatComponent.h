// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Components/Inventory/NInventoryTypes.h"
#include "NAssetManager.h"
#include "Components/ActorComponent.h"
#include "NTypes.h"
#include "NCombatComponent.generated.h"

UENUM(BlueprintType)
enum class ENCombatType : uint8
{
	None		UMETA(DisplayName = "None"),
	Melee		UMETA(DisplayName = "Melee"),
	Ranged		UMETA(DisplayName = "Ranged"),
};

UENUM(BlueprintType)
enum class ENWeaponSwapEvent : uint8
{
	None,
	EquipMelee,
	EquipRanged,
	HolsterMelee,
	HolsterRanged
};

UENUM(BlueprintType)
enum class ENHitReaction : uint8
{
	None,
	Front,
	Back,
	Right,
	Left,
};

USTRUCT(BlueprintType)
struct FHitMontages
{
	GENERATED_USTRUCT_BODY();

	FHitMontages() = default;

	UPROPERTY()
	TMap<ENHitReaction, UAnimMontage*> Montages;

	void SetMontages(TMap<ENHitReaction, UAnimMontage*> InMontages) { Montages = InMontages; }

	UAnimMontage* operator [](ENHitReaction Reaction)
	{
		{
			UAnimMontage** Montage = Montages.Find(Reaction);
			if (Montage)
			{
				return *Montage;
			}
		}
		
		{
			UAnimMontage** Montage = DefaultMontages.Find(Reaction);
			if (Montage)
			{
				return *Montage;
			}
		}

		return nullptr;
	}
	
private:

	// Fallback montages if there are no montage overrides for any particular ENHitReaction
	UPROPERTY(EditAnywhere, Category = "Settings|Animation")
	TMap<ENHitReaction, UAnimMontage*> DefaultMontages;
};


class INCombatComponentInterface;
class ANWeaponActor;
class USphereComponent;
class UNSpringArmComponent;
class UWidgetComponent;
class UNEquipmentItem;
class UNWeaponItem;


// TODO move to another file, will be useful elsewhere
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyDelegate);

/** Sections
*	1. Blueprint Settings
*	2. State
*	3. References
*	4. Overrides
*	5. Interface and Methods
*		5a. Equipments Slots
*		5b. Weapons
*		5c. Targeting
*	6. Server RPC's
*/

/**
* Combat component handles targeting, equipment slots that hold data about equipped items, and handles weapons using the
* Gameplay Ability System.
* Can be added to characters implementing the INCombatComponentInterface.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKEDRPG_API UNCombatComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UNMovementSystemComponent;
	friend class ANMeleeWeaponActor;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatHitDelegate, AActor*, OtherActor, bool, bFromSweep, const FHitResult&, Hit);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetUpdatedDelegate, UPrimitiveComponent*, InTarget);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwappingDelegate, bool, ActiveWeaponSwap);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemovedFromSlotDelegate, const FNInventorySlot&, RemovedItem);

public:
	// Sets default values for this component's properties
	UNCombatComponent();
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** The Camera Settings when on melee mode */
	UPROPERTY(EditAnywhere, Category="Settings|Camera")
	FCameraModeSettings MeleeCameraMode;

	/** The Camera Settings when on ranged aiming mode */
	UPROPERTY(EditAnywhere, Category="Settings|Camera")
	FCameraModeSettings RangedCameraMode;

	/** The UUserWidget subclass for the aiming reticle widget */ 
	UPROPERTY(EditAnywhere, Category="Settings|UI")
	TSubclassOf<UUserWidget> AimingReticleClass;

	/** For any object we want to be detected by the targeting system,
	*we must set the Collision Response -> Object Response to this to Overlap */
	UPROPERTY(VisibleAnywhere, Category = "Settings|TargetingSystem")
	TEnumAsByte<ECollisionChannel> CollisionObjectType;
	
	/** The largest angle away from where the controller is facing that will detect TargetActors */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"), Category="Settings|TargetingSystem")
	float MaxAngleForTargeting;

	/** The distance a target must be to the owning actor for detection */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "2000.0", UIMin = "0.0", UIMax = "2000.0"), Category="Settings|TargetingSystem")
	float MaxTargetDetectionDistance;

	/** The distance a target where a target will be lost */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "2000.0", UIMin = "0.0", UIMax = "2000.0"), Category= "Settings|TargetingSystem")
	float MaxTargetLockDistance;

	/** The change in the camera pitch when target locked */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"), Category= "Settings|TargetingSystem")
	float TargetingCameraPitchModifier;

	/** The Camera Settings when target locked */
	UPROPERTY(EditAnywhere, Category = "Settings|Camera")
	FCameraModeSettings TargetingCameraMode;
	
	/** The UUserWidget subclass for the visual target indicator widget */ 
	UPROPERTY(EditAnywhere, Category="Settings|TargetingSystem")
	TSubclassOf<UUserWidget> TargetIndicatorWidgetClass;

	/** The montages to play after flinch inducing hit. */
	UPROPERTY(EditAnywhere, Category = "Settings|Animation")
	TMap<ENHitReaction, UAnimMontage*> FlinchMontages;

	/** The montages to play after knock inducing hit. */
	UPROPERTY(EditAnywhere, Category = "Settings|Animation")
	TMap<ENHitReaction, UAnimMontage*> KnockMontages;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. State 
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** **Replicated** **/
	UPROPERTY(ReplicatedUsing=OnRep_ArmourSlots, EditAnywhere, Category = "State|Equipment")
	TArray<FNArmourSlot> ArmourSlots;
	
	/** **Replicated** **/
	UPROPERTY(ReplicatedUsing=OnRep_WeaponSlots, EditAnywhere, Category = "State|Equipment")
	TArray<FNWeaponSlot> WeaponSlots;
	
	/** **Replicated** **/
	UPROPERTY(ReplicatedUsing=OnRep_ItemSlots, EditAnywhere, Category = "State|Equipment")
	TArray<FNEquipmentSlot> ItemSlots;
	
private:
	/** ** Replicated ** **/
	UPROPERTY(Replicated)
	UPrimitiveComponent* Target;

	/** Temporarily holds any actors hit during a melee weapon swing. */
	UPROPERTY()
	TArray<AActor*> HitActors;

	ENCombatType ActiveCombatType;

	/** Indicates whether there is a weapon change in progress (prevents calling another weapon change mid animation). */
	bool bActiveWeaponChange;
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. References
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	UPROPERTY()
	APlayerController* PlayerController;
	USphereComponent* TargetingCollisionComponent; // Sphere in which target must be to be detected.
	UWidgetComponent* TargetIndicatorWidgetComponent;
	UNSpringArmComponent* SpringArm;
	UUserWidget* TargetIndicatorWidget;
	UUserWidget* AimingReticle;
	ACharacter* OwningCharacter;
	TScriptInterface<INCombatComponentInterface> Interface;
	FGameplayTag ActiveWeaponSwapGameplayTag;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Called every frame when we are target locked. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** Called when the game starts, calls Initialize(). */ 
	virtual void BeginPlay() override;
	
	/** Replicates WeaponSlots, ArmourSlots, and ItemSlots. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** Sets references, generates targeting system components, and initializes slots. */
	void Initialize();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5a. EquipmentSlots
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Fires when OnRep_WeaponSlots() is called */
	FNotifyDelegate OnWeaponSlotsUpdated;
	
	/** Fires when OnRep_ArmourSlots() is called */
	FNotifyDelegate OnArmourSlotsUpdated;

	/** Fires when OnRep_ItemSlots() is called */
	FNotifyDelegate OnItemSlotsUpdated;

	/** Fires when an item is removed from a slot. */
	FOnItemRemovedFromSlotDelegate OnItemRemovedFromSlot;
	
	/** [server] Slots and spawns the input item to it's appropriate slot. */
	bool SlotItem(FNInventorySlot& InventorySlot);
	
	/** [server] Looks for an appropriate empty slot for this item to go into, and slots the item if it finds one.
	 * Returns true if item was slotted. */
	bool AutoSlotItem(FNInventorySlot& InventorySlot);

	/** [server] Finds item matching the input slots SlotId an removes it. */
	bool RemoveItemFromSlot(const ENItemSlotId& SlotId);
	bool RemoveItemFromSlot(const FNInventorySlot& InventorySlot);
	
	/** [server] Finds item matching slot and changes it's slot number. */
	bool UpdateSlotNumber(const int32& OriginalSlotNumber, const int32 NewSlotNumber);
	bool UpdateSlotNumber(const FNInventorySlot& Slot, const int32 NewSlotNumber);

private:
	/** Searches all equipment slots and returns the matching slot, or a null slot if no match found. */
	FNEquipmentSlot& FindSlot(int32 SlotNumber, bool bEmptyOnly = false);
	FNEquipmentSlot& FindSlot(const ENItemSlotId SlotId, bool bEmptyOnly = false);
	FNEquipmentSlot& FindSlot(const FNInventorySlot& SlotNumber, bool bEmptyOnly = false);
	FNEquipmentSlot& FindSlot(TFunctionRef<bool(const FNEquipmentSlot& Slot)> Predicate, bool bEmptyOnly = false);

	/** Searches and returns the weapon in the slot matching the SlotId. Should only be Ranged or Melee. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FNWeaponSlot& FindWeapon(const ENItemSlotId SlotId);

	/** Calls UpdateWeaponSlot on all weapon slots. */
	UFUNCTION()
	void OnRep_WeaponSlots(TArray<FNWeaponSlot> OldWeaponSlots);

	/** Calls UpdateArmourSlot on all armour slots. */
	UFUNCTION()
	void OnRep_ArmourSlots(TArray<FNArmourSlot> OldArmourSlots);

	/** Calls UpdateItemSlot on all item slots. */
	UFUNCTION()
	void OnRep_ItemSlots(TArray<FNEquipmentSlot> OldItemSlots);

	/** Handles updating a weapon slot if it's state has changed */
	void UpdateWeaponSlot(FNWeaponSlot& WeaponSlot, FNWeaponSlot& OldWeaponSlot) const;

	/** Handles updating an armour slot if it's state has changed */
	void UpdateArmourSlot(FNArmourSlot& ArmourSlot, FNArmourSlot& OldArmourSlot) const;

	/** Handles updating an item slot if it's state has changed */
	void UpdateItemSlot(FNEquipmentSlot& ItemSlot, FNEquipmentSlot& OldItemSlot) const;



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5b. Weapons
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Changes the value of bActiveWeaponSwapping is called. */
	FOnWeaponSwappingDelegate OnWeaponSwapping;

	/** Fires when a weapon hits a new target. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnCombatHitDelegate OnWeaponHit;
	
	/** Returns the owning character mesh. */
	USkeletalMeshComponent* GetOwnerMesh() const;
	
	/** Returns the mesh of the slotted ranged weapon if there is one, else returns nullptr. */
	USkeletalMeshComponent* GetRangedWeaponMesh();

	/** Returns the mesh of the slotted melee weapon if there is one, else returns nullptr. */
	USkeletalMeshComponent* GetMeleeWeaponMesh();

	/** Sets the equipped melee weapon mesh to generate overlap events. Should be called in an anim notify / anim notify state of a weapon swing anim montage. */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	void ActivateWeapon();

	/** Sets the equipped melee weapon mesh to NOT generate overlap events and clears the list of HitActors. Should be called in an anim notify / anim notify state of a weapon swing anim montage. */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	void DeactivateWeapon();

	/** Clears the list of hit actors. Can call this if we want to be able to do multiple hits in the same weapon swing. Gets called in DeactivateWeapon(). */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ClearHits();

	/** Returns true if the WeaponSlot matching the input SlotId's weapon is Equipped, false if the weapon is Holstered or there is no weapon slotted. */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool IsWeaponEquipped(ENItemSlotId SlotId);
	
	/** Returns true if the WeaponSlot matching the input SlotId's weapon is Holstered, false if the weapon is Equipped or there is no weapon slotted. */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool IsWeaponHolstered(ENItemSlotId SlotId);

	/** Equips the weapon in the slot matching the input SlotId if there is a weapon slotted. */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void EquipWeapon(ENItemSlotId SlotId);

	/** Holsters the weapon in the slot matching the input SlotId if there is a weapon slotted. */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void HolsterWeapon(ENItemSlotId SlotId);
	
private:
	/** [local] Generates the default aiming reticle for ranged weapons. */
	void CreateAimingReticle();

	/** Updates the current combat state of the owning actor (Will update the stance). */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void UpdateCombatState(ENCombatType InCombatType, ENStance InStance);

	/** Updates the camera mode based on the if we are target locked and the ActiveCombatType. */
	void UpdateCameraMode() const;

	/** Callback to bind to melee weapon OnComponentBeginOverlap, Broadcasts OnHit when overlapping an actor for the first time */
	UFUNCTION()
	virtual void OnWeaponCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/** Callback for when OnWeaponStateChangeDelegate. */
	UFUNCTION()
	void OnWeaponStateChange(ENStance InStance);

	/** Callback for OnWeaponSwapping - Adds/Removes ActiveWeaponSwapGameplayTag. Delegate is fired during weapon change animation in ANWeaponActor. */
	UFUNCTION()
	void SetActiveWeaponSwap(bool InActiveWeaponSwap);
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5c. Targeting
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Fires when a new Target is set or Target is set to nullptr. */
	FOnTargetUpdatedDelegate OnTargetUpdated;

	/** Returns true if currently target locked */
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	bool IsLocked() const;
	
	/** Returns the current TargetedActor if there is one */
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	AActor* GetTargetActor() const;
	
	/** Returns the location of the TargetedActor or a zero vector if no target is locked */
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	FVector GetTargetLocation() const;

	/** Returns the OwningCharacter's EyeViewpoint Location. */
	FVector GetEyesLocation() const;
	
	/** Locks a TargetActor if possible, or turns off Target Locking if a target is already locked  */
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void ToggleTargetLock();
	
	/** Moves to the next available target to the right if there is one */
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void SwitchTargetToRight();
	
	/** Moves to the next available target to the left if there is one */
	UFUNCTION(BlueprintCallable, Category = "TargetingSystem")
	void SwitchTargetToLeft();

	/** [local + sever] Removes target lock */
	void UnLock();
	
private:
	
	/** [local] Generates the CollisionSphere that will be used to detect Targetable Actors - call in Initialize(). */
	void CreateCollisionSphere();

	/** [local] Generates the TargetWidget - call in Initialize(). */
	void CreateTargetWidget();

	/** Sets the new targeted actor and broadcasts. */
	void SetTarget(UPrimitiveComponent* InTarget);

	/** [local + sever] Sets lock on input TargetToLock */
	void Lock(UPrimitiveComponent* TargetToLock);

	/** [local] Finds all targets within the targeting sphere (within MaxTargetDetectionDistance) and within the MaxAngleForTargeting */
	TArray<UPrimitiveComponent*> GetAvailableTargets() const;
	
	/** [local] Finds the actor with the closest angle to the player based on the input predicate
	 * By with no Predicate argument, locks the target that required the smallest turn to target
	 * For use in both Initial Targeting and Switching Targets
	 * Returns true if a new target is found. */
	bool FindSortedTarget(TFunctionRef<bool(float AngleToTurn, float ClosestAngele)> Predicate =
		[](float AngleToTurn, float ClosestAngele){return FMath::Abs(AngleToTurn) < FMath::Abs(ClosestAngele);});
	
	/** Helper functions */
	FVector GetOwnerLocation() const;
	FRotator GetControlRotation() const;

	/** [local] Called whenever there is an update to the MovementSystemComponent's state. */
	UFUNCTION()
	void OnMovementSystemUpdated(ENMovementGait InMovementMode, ENRotationMode InRotationMode, ENStance InStance);

public:
	void ReceivedHit(float HitStrength, float ZRotation);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 6. Server RPC's
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** Adds target lock - Needed to keep TargetActor reference properly replicated */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLock(UPrimitiveComponent* TargetToLock);
	void ServerLock_Implementation(UPrimitiveComponent* TargetToLock);
	bool ServerLock_Validate(UPrimitiveComponent* TargetToLock);

	/** Removes target lock - Needed to keep TargetActor reference properly replicated */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUnLock();
	void ServerUnLock_Implementation();
	bool ServerUnLock_Validate();

	/** Calls HolsterWeapon() */
	UFUNCTION(Server, Reliable)
	void ServerHolsterWeapon(ENItemSlotId SlotId);
	void ServerHolsterWeapon_Implementation(ENItemSlotId SlotId);

	/** Calls EquipWeapon() */
	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(ENItemSlotId SlotId);
	void ServerEquipWeapon_Implementation(ENItemSlotId SlotId);
};



