// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbility_Sprint.h"
#include "Components/NCharacterMovementComponent.h"
#include "AbilitySystem/NAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"

void UNGameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    
    GESprintingHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, GESprint.GetDefaultObject(), 1.0, 1);

    if (UNCharacterMovementComponent* MC = Cast<UNCharacterMovementComponent>(ActorInfo->MovementComponent))
    {
        MC->StartSprinting();

        if (UNAbilitySystemComponent* ASC = Cast<UNAbilitySystemComponent>(ActorInfo->AbilitySystemComponent))
        {
            // ASC->AddGameplayCueLocal()
            WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
            WaitInputReleaseTask->OnRelease.AddDynamic(this, &UNGameplayAbility_Sprint::OnInputReleased);
            WaitInputReleaseTask->Activate();
            
            WaitSprintEndTagAddedTask = UAbilityTask_WaitGameplayTagAdded::WaitGameplayTagAdd(this, TagSprintEnd);
            WaitSprintEndTagAddedTask->Added.AddDynamic(this, &UNGameplayAbility_Sprint::OnSprintEnd);
            WaitSprintEndTagAddedTask->Activate();
            
            ProcessCost();
        }
    }
}

void UNGameplayAbility_Sprint::ProcessCost()
{
    UE_LOG(LogTemp, Warning, TEXT("ProcessCost()"))
    WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 0.05);
    WaitDelayTask->OnFinish.AddDynamic(this, &UNGameplayAbility_Sprint::OnWaitDelayFinish);
    WaitDelayTask->Activate();
}

void UNGameplayAbility_Sprint::OnWaitDelayFinish()
{
    UE_LOG(LogTemp, Warning, TEXT("OnWaitDelayFinish()"))
    SyncPoint = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::BothWait);
    SyncPoint->OnSync.AddDynamic(this, &UNGameplayAbility_Sprint::OnSync);
    SyncPoint->Activate();
}

void UNGameplayAbility_Sprint::OnSync()
{
    UE_LOG(LogTemp, Warning, TEXT("OnSync()"))
    if (UNCharacterMovementComponent* MC = Cast<UNCharacterMovementComponent>(GetActorInfo().MovementComponent))
    {
        if (MC->GetCurrentAcceleration().Size() > 0 && !MC->IsFalling())
        {
            if (!CommitAbilityCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo()))
            {
                EndAbility();
            }  
        }

        if (!bEndSprint)
        {
            ProcessCost();
        }
    }
}

void UNGameplayAbility_Sprint::OnInputReleased(float TimeHeld)
{
    WaitInputReleaseTask->OnReleaseCallback();
    EndAbility();
}

void UNGameplayAbility_Sprint::OnSprintEnd()
{
    SyncPoint = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::BothWait);
    SyncPoint->OnSync.AddDynamic(this, &UNGameplayAbility_Sprint::EndAbility);
    SyncPoint->Activate();
}

void UNGameplayAbility_Sprint::EndAbility()
{
    UE_LOG(LogTemp, Warning, TEXT("Sprint Ended!!"))
    bEndSprint = true;
    bool bReplicateEndAbility = true;
    bool bWasCancelled = false;
    Super::EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), bReplicateEndAbility, bWasCancelled);
}

void UNGameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    FActiveGameplayEffectHandle GESprintRemovalHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, GESprintRemoval.GetDefaultObject(), 1.0, 1);

    BP_RemoveGameplayEffectFromOwnerWithHandle(GESprintingHandle, -1);
    BP_RemoveGameplayEffectFromOwnerWithHandle(GESprintRemovalHandle, -1);

    if (UNCharacterMovementComponent* MC = Cast<UNCharacterMovementComponent>(GetActorInfo().MovementComponent))
    {
        MC->StopSprinting();
        if (IsValid(WaitSprintEndTagAddedTask))
        {
            WaitSprintEndTagAddedTask->EndTask();
        }

        if (UNAbilitySystemComponent* ASC = Cast<UNAbilitySystemComponent>(GetActorInfo().AbilitySystemComponent))
        {
            // ASC->RemoveGameplayCueLocal()
        }
    }
}


