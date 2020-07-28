// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Equipment/NWeaponActor.h"
#include "Net/UnrealNetwork.h"
#include "Characters/NCharacterBase.h"
#include "AbilitySystemInterface.h"
#include "Characters/Components/NCombatComponent.h"

ANWeaponActor::ANWeaponActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.SetTickFunctionEnable(false);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    SetRootComponent(WeaponMesh);

    SetReplicates(false);
}


void ANWeaponActor::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
    Super::TickActor(DeltaTime,TickType, ThisTickFunction);

    bool Updated = false;
    
    if (RelativeLocation != TargetRelativeLocation)
    {
        RelativeLocation = FMath::VInterpTo(RelativeLocation, TargetRelativeLocation, DeltaTime, 10.f);
        WeaponMesh->SetRelativeLocation(RelativeLocation);
        Updated = true;
    }
    
    if (RelativeRotation != TargetRelativeRotation)
    {
        RelativeRotation = FMath::RInterpTo(RelativeRotation, TargetRelativeRotation, DeltaTime, 10.f);
        WeaponMesh->SetRelativeRotation(RelativeRotation);
        Updated = true;
    }

    if (!Updated)
    {
        SetActorTickEnabled(false);
    }
}


void ANWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}


UAbilitySystemComponent* ANWeaponActor::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}


void ANWeaponActor::Initialize(FWeaponActorData InData)
{
    // Call on the server only
    if (GetLocalRole() != ROLE_Authority)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Should only call on the server"), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        }
        
        return;
    }
    
    if (!InData.OwningCharacter)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Input Data has null OwningCharacter"), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        }
        
        return;
    }

    if (!InData.WeaponData)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Input Data has null WeaponData"), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        }
        
        return;
    }

    Data = InData;
    SetProperties(InData);
}


void ANWeaponActor::Initialize(ACharacter* InOwningCharacter, UNWeaponItem* WeaponItem)
{
    Initialize(FWeaponActorData(InOwningCharacter, WeaponItem));
}


bool ANWeaponActor::Equip(bool bAnimate)
{
    if (!Data)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Data contains a null value."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        }
        
        return false;
    }
    
    if (bActiveWeaponSwap)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Could not execute, already swapping weapon."), *FString(__FUNCTION__), *GetName()), EPrintType::Warning);
        }
        
        return false;
    }


    if (bAnimate)
    {
        // Make sure we begin equip animation with weapon in correct holstered location
        AttachWeaponToSocket(Data.WeaponData->HolsteredSocketName, Data.WeaponData->HolsteredAttachmentOffset, false);
        CombatComponent->OnWeaponSwapping.Broadcast(true);
        bActiveWeaponSwap = true;
    
        Data.OwningCharacter->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ANWeaponActor::OnWeaponSwapAnimNotify);
        Data.OwningCharacter->GetMesh()->GetAnimInstance()->Montage_Play(Data.WeaponData->EquipMontage);
    }
    else
    {
        AttachWeaponToSocket(Data.WeaponData->EquippedSocketName, Data.WeaponData->AttachmentOffset, false);
    }
    

    return true;
}


bool ANWeaponActor::Holster(bool bAnimate)
{  
    if (!Data)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Data contains a null value."), *FString(__FUNCTION__), *GetName()), EPrintType::Failure);
        }
        
        return false;
    }

    if (bActiveWeaponSwap)
    {
        if (DebugCombatComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s %s Could not execute, already swapping weapon."), *FString(__FUNCTION__), *GetName()), EPrintType::Warning);
        }
        
        return false;
    }

    if (bAnimate)
    {
        // Make sure we begin holster animation with weapon in correct equipped location
        AttachWeaponToSocket(Data.WeaponData->EquippedSocketName, Data.WeaponData->AttachmentOffset, false);

        // Broadcast prevents the player from initiating another swap until the weapon is snapped to the appropriate socket
        CombatComponent->OnWeaponSwapping.Broadcast(true);
        bActiveWeaponSwap = true;
        
        Data.OwningCharacter->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ANWeaponActor::OnWeaponSwapAnimNotify);
        Data.OwningCharacter->GetMesh()->GetAnimInstance()->Montage_Play(Data.WeaponData->HolsterMontage);
    }
    else
    {
        AttachWeaponToSocket(Data.WeaponData->HolsteredSocketName, Data.WeaponData->HolsteredAttachmentOffset, false);
    }
    

    return true;
}


USkeletalMeshComponent* ANWeaponActor::GetWeaponMesh() const
{
    return WeaponMesh;
}


void ANWeaponActor::SetProperties(FWeaponActorData InData)
{
    SetOwner(InData.OwningCharacter);
    WeaponMesh->SetSkeletalMesh(InData.WeaponData->ItemMesh);
    AttachToComponent(InData.OwningCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    EquippedGameplayTag = InData.WeaponData->EquippedGameplayTag;
    CombatComponent = InData.OwningCharacter->FindComponentByClass<UNCombatComponent>();

    IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(InData.OwningCharacter);
    if (Interface)
    {
        AbilitySystemComponent = Cast<UNAbilitySystemComponent>(Interface->GetAbilitySystemComponent());
    }
}


void ANWeaponActor::AttachWeaponToSocket(FName& SocketName, FAttachmentOffset& Offset, bool bSmoothAttach)
{
    TargetRelativeLocation = Offset.RelativeLocationOffset;;
    TargetRelativeRotation = Offset.RelativeRotationOffset;

    WeaponMesh->AttachToComponent(Data.OwningCharacter->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, SocketName);

    if (bSmoothAttach)
    {
        // Handle interpolation of location/rotation in tick
        RelativeLocation = WeaponMesh->GetRelativeLocation();
        RelativeRotation = WeaponMesh->GetRelativeRotation();
    }
    else
    {
        // Snap to target location and rotation
        WeaponMesh->SetRelativeLocationAndRotation(TargetRelativeLocation, TargetRelativeRotation);   
    }
    
    SetActorTickEnabled(bSmoothAttach);
}


void ANWeaponActor::OnWeaponSwapAnimNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{    
    if (NotifyName == "E")
    {
        AttachWeaponToSocket(Data.WeaponData->EquippedSocketName, Data.WeaponData->AttachmentOffset, true);
        AbilitySystemComponent->AddLooseGameplayTag(EquippedGameplayTag);
        bIsEquipped = true;
    }
    
    if (NotifyName == "H")
    {
        AttachWeaponToSocket(Data.WeaponData->HolsteredSocketName, Data.WeaponData->HolsteredAttachmentOffset, true);
        AbilitySystemComponent->RemoveLooseGameplayTag(EquippedGameplayTag);
        bIsEquipped = false;
    }

    bActiveWeaponSwap = false;
    CombatComponent->OnWeaponSwapping.Broadcast(false);
    Data.OwningCharacter->GetMesh()->GetAnimInstance()->OnPlayMontageNotifyBegin.RemoveAll(this);
}

void ANWeaponActor::OnRep_Data()
{
    if (Data)
    {
        SetProperties(Data);
    }
}



