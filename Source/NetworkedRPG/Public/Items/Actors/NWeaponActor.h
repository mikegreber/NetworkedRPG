// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Items/Actors/NEquipmentActor.h"
#include "NWeaponActor.generated.h"

class UNWeaponItem;

/** Hold these variables in a struct so we can replicate them at the same time **NOT CURRENTLY REPLICATING THIS ACTOR** */
USTRUCT(BlueprintType)
struct FWeaponActorData
{
    GENERATED_USTRUCT_BODY()

    FWeaponActorData() = default;

    explicit FWeaponActorData(ACharacter* const OwningCharacter, UNWeaponItem* const WeaponData):
        OwningCharacter(OwningCharacter),
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
class UNCombatComponent;
class ANCharacterBase;
struct FAttachmentOffset;

/**
 *  Base class for a weapon actor. NOT meant to be blueprinted, should spawned within game based off of UNWeaponItem data.
 *  MUST call Initialize() after spawning.
 */
UCLASS()
class NETWORKEDRPG_API ANWeaponActor : public ANEquipmentActor, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ANWeaponActor();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 1. References and State
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    // Internal State for socket attachment interpolation.
    FVector RelativeLocation;
    FVector TargetRelativeLocation;
    FRotator RelativeRotation;
    FRotator TargetRelativeRotation;

    bool bActiveWeaponSwap;
    bool bIsEquipped;
    FGameplayTag EquippedGameplayTag;

protected:
    /** Holds the OwningCharacter and WeaponData. */
    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    FWeaponActorData Data;

    UPROPERTY()
    UNCombatComponent* CombatComponent;

    UNAbilitySystemComponent* AbilitySystemComponent;


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 2. Overrides
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Handles interpolation for smooth socket attachment. */
    virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** IAbilitySystemInterface */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 3. Interface and Methods
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** [Server] Must call from server after spawning. */
    virtual void Initialize(FWeaponActorData Data);
    void Initialize(ACharacter* InOwningCharacter, UNWeaponItem* WeaponItem);

    /** Plays animation and equips the weapon. */
    bool Equip(bool bAnimate = true);

    /** Plays animation and holsters the weapon. */
    bool Holster(bool bAnimate = true);

    /** Returns true if weapon is currently equipped (not holstered). */
    bool IsEquipped() const { return bIsEquipped; }

protected:
    /** Sets the weapon mesh and attaches the actor to the owning character */
    void SetProperties(FWeaponActorData Data);

    /** Attaches the weapon to the input SocketName with the input Offset.
     * If bSmoothAttach is true, will interpolate in tick to the final position.
     * If bSmoothAttach is false, will snap to the socket. */
    void AttachWeaponToSocket(FName& SocketName, FAttachmentOffset& Offset, bool bSmoothAttach = true);

    /** Attaches the weapon to its proper socket (equipped or holstered socket). Weapon swapping AnimMontages must have
     * AnimNotify at the time we want to attach to the socket names "E" or "H" for Equip or Holster animations, respectively. */
    UFUNCTION()
    void OnWeaponSwapAnimNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

    /** Not currently replicating this actor */
    /** Updates the visual information to clients when data is replicated. */
    UFUNCTION()
    void OnRep_Data();
}; 
