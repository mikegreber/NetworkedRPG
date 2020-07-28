// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NGameplayAbility.h"
#include "AbilitySystemComponent.h"
// #include "DrawDebugHelpers.h"
#include "Abilities/NAbilitySystemComponent.h"
#include "Player/NCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
// #include "Interface/NGameplayAbilityActorInterface.h"

UNGameplayAbility::UNGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    bActivateAbilityOnGranted = false;
    bSourceObjectMustEqualCurrentWeaponToActivate = false;
}

UNCombatComponent* UNGameplayAbility::GetOwnerCombatComponent() const
{
        return CombatComponent;  
}

FRotator UNGameplayAbility::GetActorRotation()
{
      return GetActorInfo().AvatarActor->GetActorRotation().GetNormalized();
}

FRotator UNGameplayAbility::GetControlRotation()
{
    return GetActorInfo().PlayerController->GetControlRotation().GetNormalized();
}

FRotator UNGameplayAbility::GetInputDirection()
{
    FVector InputDirection = Cast<UCharacterMovementComponent>(GetActorInfo().MovementComponent.Get())->GetCurrentAcceleration().GetSafeNormal();

    // TODO Defaults to being towards camera if no acceleration, should 
    if (InputDirection == FVector::ZeroVector)
    {
        return FRotator(0.f,-179.f,0.f) + GetControlRotation();
    }
    return InputDirection.Rotation().GetNormalized();
}

void UNGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);

    
    OwningCharacter = Cast<ACharacter>(ActorInfo->AvatarActor);
    Interface = OwningCharacter;
    if (OwningCharacter)
    {
        CombatComponent = OwningCharacter->FindComponentByClass<UNCombatComponent>();
    }
    else
    {
        UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s %s null OwningCharacter, CombatComponent not set."), *FString(__FUNCTION__), *GetName()), true, true, FColor::Red);
    }
    
    
    if (!CombatComponent && ActorInfo->IsLocallyControlled())
    {
        UKismetSystemLibrary::PrintString(GetWorld(), "NGameplayAbility: CombatComponent not found. NGameplayAbilities can only be used on ", true, true, FColor::Red);
    }
    
    if (bActivateAbilityOnGranted)
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
    }
}

FGameplayAbilityTargetDataHandle UNGameplayAbility::MakeGameplayAbilityTargetDataHandleFromActorArray(const TArray<AActor*> TargetActors)
{
    if (TargetActors.Num() > 0)
    {
        FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
        NewData->TargetActorArray.Append(TargetActors);
        return FGameplayAbilityTargetDataHandle(NewData);
    }

    return FGameplayAbilityTargetDataHandle();
}

FGameplayAbilityTargetDataHandle UNGameplayAbility::MakeGameplayAbilityTargetDataHandleFromHitResult(const TArray<FHitResult> HitResults)
{
    FGameplayAbilityTargetDataHandle TargetData;

    for (const FHitResult& HitResult : HitResults)
    {
        FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
        TargetData.Add(NewData);
    }

    return TargetData;
}

UObject* UNGameplayAbility::K2_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const
{
    return GetSourceObject(Handle, &ActorInfo);
}

bool UNGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle,
    bool EndAbilityImmediately)
{
    UNAbilitySystemComponent* ASC = Cast<UNAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
    if (ASC)
    {
        return ASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
    }

    return false;
}

void UNGameplayAbility::ExternalEndAbility()
{
    check(CurrentActorInfo);

    const bool bReplicateEndAbility = true;
    const bool bWasCancelled = false;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FString UNGameplayAbility::GetCurrentPredictionKeyStatus()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

bool UNGameplayAbility::IsPredictionKeyValidForMorePrediction() const
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}

bool UNGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    // if (bSourceObjectMustEqualCurrentWeaponToActivate)
    // {
    //     ANCharacter* Character = Cast<ANCharacter>(ActorInfo->AvatarActor);
    //     if (Character && Character->GetCurrentWeapon() && (UObject*)Hero->GetCurrentWeapon() == GetSourceObject(Handle, ActorInfo))
    //     {
    //         return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
    //     }
    //     else
    //     {
    //         return false;
    //     }
    // }

    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
    
}

bool UNGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
    return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) && NCheckCost(Handle, *ActorInfo);
}

bool UNGameplayAbility::NCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo& ActorInfo) const
{
    return true;
}

void UNGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
    NApplyCost(Handle, *ActorInfo, ActivationInfo);
    Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

UAnimMontage* UNGameplayAbility::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
    FAbilityMeshMontage AbilityMeshMontage;
    if (FindAbilityMeshMontage(InMesh, AbilityMeshMontage))
    {
        return AbilityMeshMontage.Montage;
    }

    return nullptr;
}

void UNGameplayAbility::SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InCurrentMontage)
{
    ensure(IsInstantiated());

    FAbilityMeshMontage AbilityMeshMontage;
    if (FindAbilityMeshMontage(InMesh, AbilityMeshMontage))
    {
        AbilityMeshMontage.Montage = InCurrentMontage;
    }
    else
    {
        CurrentAbilityMeshMontages.Add(FAbilityMeshMontage(InMesh, InCurrentMontage));
    }

    
}

bool UNGameplayAbility::FindAbilityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMeshMontage)
{
    for (FAbilityMeshMontage& MeshMontage : CurrentAbilityMeshMontages)
    {
        if (MeshMontage.Mesh == InMesh)
        {
            InAbilityMeshMontage = MeshMontage;
            return true;
        }
    }

    return false;
}

void UNGameplayAbility::MontageJumptToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Checked());
    if (ASC->IsAnimatingAbility(this))
    {
        ASC->CurrentMontageJumpToSectionForMesh(InMesh, SectionName);
    }
}

void UNGameplayAbility::MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName,
    FName ToSectionName)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Checked());
    if (ASC->IsAnimatingAbilityForAnyMesh(this))
    {
        ASC->CurrentMontageSetNextSectionNameForMesh(InMesh, FromSectionName, ToSectionName);
    }
}

void UNGameplayAbility::MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
    if (ASC)
    {
        // We should only stop the current montage if we are the animating ability
        if (ASC->IsAnimatingAbilityForAnyMesh(this))
        {
            ASC->CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
        }
    }
}

void UNGameplayAbility::MontageStopForAllMeshes(float OverrideBlendOutTime)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
    if (ASC)
    {
        if (ASC->IsAnimatingAbilityForAnyMesh(this))
        {
            ASC->StopAllCurrentMontages(OverrideBlendOutTime);
        }
    }   
}
