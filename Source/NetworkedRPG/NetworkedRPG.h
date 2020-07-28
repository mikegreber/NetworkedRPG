// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"

static float MAX_ATTRIBUTE_UPDATES_PER_SECOND = 20.0f;

/** Use this in construction of local players for their camera offset, that way server knows trace start point for
  * Firing Projectiles. */
static FVector AimingCameraOffset = FVector(250.f,100.f,80.f);

#define ECC_Interactable    ECC_GameTraceChannel1
#define ECC_Weapon          ECC_GameTraceChannel2
#define ECC_Targeting       ECC_GameTraceChannel3
#define ECC_Target          ECC_GameTraceChannel4

// For Actor Components
#define OwnerHasAuthority() (GetOwnerRole() == ROLE_Authority)

UENUM(BlueprintType)
enum class ENAbilityInputID : uint8
{
    // 0 None
    None				        UMETA(DisplayName = "None"),
    // 1 Confirm
    Confirm				        UMETA(DisplayName = "Confirm"),
    // 2 Cancel
    Cancel				        UMETA(DisplayName = "Cancel"),
    // 3 Sprint
    Sprint				        UMETA(DisplayName = "Sprint"),
    // 4 Jump
    Dodge				        UMETA(DisplayName = "Dodge"),
    //5 Aim
    Aim    				        UMETA(DisplayName = "Aim"),
    //6 Parry
    Parry    				    UMETA(DisplayName = "Parry"),
    // 7 Primary Weapon Ability
    PrimaryWeaponAbility        UMETA(DisplayName = "Primary Weapon Ability"),
    // 8 Secondary Weapon Ability
    SecondaryWeaponAbility      UMETA(DisplayName = "Secondary Weapon Ability"),
};

UENUM(BlueprintType)
enum class ENCameraMode : uint8
{
    // 0 No camera locking, free camera and player movement
    Unlocked				UMETA(DisplayName = "Unlocked"),
    // 1 Camera Locked to target, free player movement
    CameraLock				UMETA(DisplayName = "CameraLock"),
};

UENUM(BlueprintType)
enum class ENRotationMode: uint8
{
    // 0 VelocityDirection - Orient character rotation to the direction of their velocity
    VelocityDirection		UMETA(DisplayName = "VelocityDirection"),
    // 1 LookingDirection - Orient character rotation to the direction they are looking
    LookingDirection	    UMETA(DisplayName = "LookingDirection"),
};

UENUM(BlueprintType)
enum class ENMovementGait : uint8
{
    Walking				UMETA(DisplayName = "Walking"),
    Running				UMETA(DisplayName = "Running"),
    Sprinting			UMETA(DisplayName = "Sprinting"),
};

/** This enum is used to determine the proper stance in the anim blueprint */
UENUM(BlueprintType)
enum class ENCombatMode : uint8
{
    // 0 Non-combat stance
    None				UMETA(DisplayName = "None"),
    // 1 Ranged combat
    Ranged				UMETA(DisplayName = "Ranged"),
    // 2 Ranged combat
    Melee				UMETA(DisplayName = "Melee"),
    // 3 Sword combat
    Sword				UMETA(DisplayName = "Sword"),
    // 4 Sword and Shield combat
    SwordAndShield		UMETA(DisplayName = "SwordAndShield"),
    // 5 Two handed Weapon 
    TwoHanded           UMETA(DisplayName = "TwoHanded"),
   
    
};

UENUM(BlueprintType)
enum class ENPickupType : uint8
{
    // 0 Non-combat stance
    Item				UMETA(DisplayName = "Item"),
    // 1 Melee combat
    Weapon				UMETA(DisplayName = "Weapon"),
};

UENUM(BlueprintType)
enum class ENItemSlotId : uint8
{
    None			    UMETA(DisplayName = "None"),
    Ranged			    UMETA(DisplayName = "Ranged"),
    Melee			    UMETA(DisplayName = "Melee"),
    Head                UMETA(DisplayName = "Head"),
    Neck                UMETA(DisplayName = "Neck"),
    RightRing           UMETA(DisplayName = "RightRing"),
    LeftRing            UMETA(DisplayName = "Left"),
    Torso               UMETA(DisplayName = "Torso"),
    Waist               UMETA(DisplayName = "Waist"),
    Legs                UMETA(DisplayName = "Legs"),
    Feet                UMETA(DisplayName = "Feet"),
    Ability1            UMETA(DisplayName = "Ability Slot 1"),
    Ability2            UMETA(DisplayName = "Ability Slot 2"),
    Ability3            UMETA(DisplayName = "Ability Slot 3"),
    Ability4            UMETA(DisplayName = "Ability Slot 4"),
    Ability5            UMETA(DisplayName = "Ability Slot 5"),
    Ability6            UMETA(DisplayName = "Ability Slot 6"),
    Ability7            UMETA(DisplayName = "Ability Slot 7"),
    Ability8            UMETA(DisplayName = "Ability Slot 8"),
};

UENUM(BlueprintType)
enum class ENTeam : uint8
{
    None                UMETA(DisplayName = "None"),
    Friendly		    UMETA(DisplayName = "Friendly"),
    Neutral    			UMETA(DisplayName = "Neutral"),
    Enemy    			UMETA(DisplayName = "Enemy"),
};

UENUM(BlueprintType)
enum class ENInteractableType : uint8
{
    Item                UMETA(DisplayName = "Item"),
    Switch		        UMETA(DisplayName = "Switch"),
};

enum class EPrintType : uint8
{
    Log,
    Warning,
    Error,
    Success,
    Failure,
    ShutDown
};

static void Print(UObject* WorldContextObject, const FString Text, EPrintType PrintType = EPrintType::Log)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    FString Prefix;
    if (!World)
    {
        World = GEngine->GetWorld();
    }
    
    if (World)
    {
        if (World->WorldType == EWorldType::PIE)
        {
            switch(World->GetNetMode())
            {
            case NM_Client:
                Prefix = FString::Printf(TEXT("Client %d: "), GPlayInEditorID - 1);
                break;
            case NM_DedicatedServer:
            case NM_ListenServer:
                Prefix = FString::Printf(TEXT("Server: "));
                break;
            case NM_Standalone:
                break;
            default: ;
            }
        }
    }
    
    switch(PrintType)
    {
    case EPrintType::Log:
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::White);
        UE_LOG(LogTemp, Display, TEXT("%s %s"), *Prefix, *Text)
        break;
    case EPrintType::Warning:
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Yellow);
        UE_LOG(LogTemp, Warning, TEXT("%s %s"), *Prefix, *Text)
        break;
    case EPrintType::Success:  
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Green);
        UE_LOG(LogTemp, Display, TEXT("%s %s"), *Prefix, *Text)
        break;
    case EPrintType::Error:
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::FromHex("#8c0000"));
        UE_LOG(LogTemp, Error, TEXT("%s %s"), *Prefix, *Text)
        break;
    case EPrintType::Failure:
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Red);
        UE_LOG(LogTemp, Error, TEXT("%s %s"), *Prefix, *Text)
        break;
    case EPrintType::ShutDown:
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Purple);
        UE_LOG(LogTemp, Error, TEXT("%s %s Shutting Down."), *Prefix, *Text)
        UKismetSystemLibrary::QuitGame(World, nullptr, EQuitPreference::Quit, true);
        break;
    default: ;
    } 
}

static void PrintDebug(int32 DebugLevel, UObject* WorldContextObject, const FString Text, EPrintType PrintType = EPrintType::Log)
{
    if (PrintType != EPrintType::ShutDown && DebugLevel == 0)
    {
        return;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    FString Prefix;
    if (!World)
    {
        World = GEngine->GetWorld();
    }
    
    if (World)
    {
        if (World->WorldType == EWorldType::PIE)
        {
            switch(World->GetNetMode())
            {
            case NM_Client:
                Prefix = FString::Printf(TEXT("Client %d: "), GPlayInEditorID - 1);
                break;
            case NM_DedicatedServer:
            case NM_ListenServer:
                Prefix = FString::Printf(TEXT("Server: "));
                break;
            case NM_Standalone:
                break;
            default: ;
            }
        }
    }

    if (DebugLevel >= 2)
    {
        if (PrintType == EPrintType::Log)
        {
            UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::White);
            UE_LOG(LogTemp, Display, TEXT("%s %s"), *Prefix, *Text)
        }
    }

    if (DebugLevel >= 1)
    {
        switch(PrintType)
        {
        case EPrintType::Warning:
            UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Yellow);
            UE_LOG(LogTemp, Warning, TEXT("%s %s"), *Prefix, *Text)
            break;
            
        case EPrintType::Success:
            UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Green);
            UE_LOG(LogTemp, Display, TEXT("%s %s"), *Prefix, *Text)
            break;
            
        case EPrintType::Error:
            UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::FromHex("#8c0000"));
            UE_LOG(LogTemp, Error, TEXT("%s %s"), *Prefix, *Text)
            break;
        case EPrintType::Failure:
            UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::Red);
            UE_LOG(LogTemp, Error, TEXT("%s %s"), *Prefix, *Text)
            break;
        default: ;
        } 
    }

    if (PrintType == EPrintType::ShutDown)
    {
        UKismetSystemLibrary::PrintString(WorldContextObject, Text, true, false, FColor::White);
        UE_LOG(LogTemp, Display, TEXT("%s %s"), *Prefix, *Text)
    }
}

