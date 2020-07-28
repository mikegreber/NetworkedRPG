// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "Characters/Components/NCombatComponent.h"
#include "Items/NWeaponItem.h"
#include "Items/Equipment/NEquipmentActor.h"
#include "NWeaponActor.generated.h"


/** Hold these variables in a struct so we can replicate them at the same time **NOT CURRENTLY REPLICATING THIS ACTOR** */
USTRUCT(BlueprintType)
struct FWeaponActorData
{
    GENERATED_USTRUCT_BODY()

    FWeaponActorData() = default;

    explicit FWeaponActorData(ACharacter* const OwningCharacter, UNWeaponItem* const WeaponData)
        : OwningCharacter(OwningCharacter),
          WeaponData(WeaponData)
    {}

    UPROPERTY(VisibleAnywhere)
    ACharacter* OwningCharacter;

    UPROPERTY(VisibleAnywhere)
    UNWeaponItem* WeaponData;

    operator bool() const { return OwningCharacter && WeaponData; }
    bool operator!() const { return !this; }
};


class UNAbilitySystemComponent;
class ANCharacterBase;

/**
 *  Base class for a weapon actor.
 *  MUST call Initialize() after spawning.
 */
UCLASS(BlueprintType, Blueprintable, config=Engine, meta=(ShortTooltip="Base class for weapon actors."))
class NETWORKEDRPG_API ANWeaponActor : public ANEquipmentActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

    // Internal State for socket attachment interpolation.
    FVector RelativeLocation;
    FVector TargetRelativeLocation;
    FRotator RelativeRotation;
    FRotator TargetRelativeRotation;
    
    bool bActiveWeaponSwap;
    bool bIsEquipped;
    FGameplayTag EquippedGameplayTag;

public:
    /** Handles interpolation for smooth socket attachment. */
    virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** IAbilitySystemInterface */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
protected:

    /** Holds the OwningCharacter and WeaponData. */
    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    FWeaponActorData Data;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY()
    UNCombatComponent* CombatComponent;

    UNAbilitySystemComponent* AbilitySystemComponent;
    
public:
    ANWeaponActor();

    /** [Server] Must call from server after spawning. */
    virtual void Initialize(FWeaponActorData Data);
    void Initialize(ACharacter* InOwningCharacter, UNWeaponItem* WeaponItem);

    /** Plays animation and equips the weapon. */
    bool Equip(bool bAnimate = true);

    /** Plays animation and holsters the weapon. */
    bool Holster(bool bAnimate = true);

    /** Returns the weapon mesh. */
    USkeletalMeshComponent* GetWeaponMesh() const;

    bool IsEquipped() const { return bIsEquipped; }
    
protected:
    /** Sets the weapon mesh and attaches the actor to the owning character */
    void SetProperties(FWeaponActorData Data);

    /** Attaches the weapon to the input SocketName with the input Offset.
     * If bSmoothAttach is true, will interpolate in tick to the final position.
     * If bSmoothAttach is false, will snap to the socket. */
    void AttachWeaponToSocket(FName& SocketName, FAttachmentOffset &Offset, bool bSmoothAttach = true);

    /** Attaches the weapon to its proper socket (equipped or holstered socket). Weapon swapping AnimMontages must have
     * AnimNotify at the time we want to attach to the socket names "E" or "H" for Equip or Holster animations, respectively. */
    UFUNCTION()
    void OnWeaponSwapAnimNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

    /** Not currently replicating this actor */
    /** Updates the visual information to clients data is replicated. */
    UFUNCTION()
    void OnRep_Data();
}; 
