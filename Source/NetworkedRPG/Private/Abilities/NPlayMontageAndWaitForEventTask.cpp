// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NPlayMontageAndWaitForEventTask.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"

UNPlayMontageAndWaitForEventTask::UNPlayMontageAndWaitForEventTask()
{
}

void UNPlayMontageAndWaitForEventTask::Activate()
{
    if (!Ability)
    {
        return;
    }
    bool bPlayedMontage = false;
    UNAbilitySystemComponent* NAbilitySystemComponent = GetTargetASC();

    if (NAbilitySystemComponent)
    {
        const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
        UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
        if (AnimInstance)
        {
            EventHandle = NAbilitySystemComponent->AddGameplayEventTagContainerDelegate(EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UNPlayMontageAndWaitForEventTask::OnGameplayEvent));

            if (NAbilitySystemComponent->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) > 0.f)
            {
                if (ShouldBroadcastAbilityTaskDelegates() == false)
                {
                    return;
                }

                CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UNPlayMontageAndWaitForEventTask::OnAbilityCancelled);

                BlendingOutDelegate.BindUObject(this, &UNPlayMontageAndWaitForEventTask::OnMontageBlendingOut);
                AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

                MontageEndedDelegate.BindUObject(this, &UNPlayMontageAndWaitForEventTask::OnMontageEnded);
                AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

                ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
                if (Character && (Character->GetLocalRole() == ROLE_Authority ||
                    Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted))
                {
                    Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
                }

                bPlayedMontage = true;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayMontageAndWaitForEventTask call to PlayMontage failed!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayMontageAndWaitForEventTask called on invalid UNAbilitySystemComponent!"));
    }

    if (!bPlayedMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayMontageAndWaitForEventTask called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }

    SetWaitingOnAvatar();
}

void UNPlayMontageAndWaitForEventTask::ExternalCancel()
{
    check(AbilitySystemComponent);

    OnAbilityCancelled();

    Super::ExternalCancel();
}

FString UNPlayMontageAndWaitForEventTask::GetDebugString() const
{
    UAnimMontage* PlayingMontage = nullptr;
    if (Ability)
    {
        const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
        UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

        if (AnimInstance != nullptr)
        {
            PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
        }
    }

    return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

void UNPlayMontageAndWaitForEventTask::OnDestroy(bool AbilityEnded)
{
    // Clear delegate since it is a multicast
    if (Ability)
    {
        Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
        if (AbilityEnded && bStopWhenAbilityEnds)
        {
            StopPlayingMontage();
        }
    }

    UNAbilitySystemComponent* NAbilitySystemComponent = GetTargetASC();
    if (NAbilitySystemComponent)
    {
        NAbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
    }

    Super::OnDestroy(AbilityEnded);
}

UNPlayMontageAndWaitForEventTask* UNPlayMontageAndWaitForEventTask::PlayMontageAndWaitForEvent(
    UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay,
    FGameplayTagContainer EventTags, float Rate, FName StartSection, bool bStopWhenAbilityEnds,
    float AnimRootMotionTranslationScale)
{
    UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

    UNPlayMontageAndWaitForEventTask* MyObj = NewAbilityTask<UNPlayMontageAndWaitForEventTask>(OwningAbility, TaskInstanceName);
    MyObj->MontageToPlay = MontageToPlay;
    MyObj->EventTags = EventTags;
    MyObj->Rate = Rate;
    MyObj->StartSection = StartSection;
    MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
    MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;

    return MyObj;
}

bool UNPlayMontageAndWaitForEventTask::StopPlayingMontage()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    if (!ActorInfo)
    {
        return false;
    }

    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    if (!AnimInstance)
    {
        return false;
    }

    if (AbilitySystemComponent && Ability)
    {
        if (AbilitySystemComponent->GetAnimatingAbility() == Ability && AbilitySystemComponent->GetCurrentMontage() == MontageToPlay)
        {
            FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
            if (MontageInstance)
            {
                MontageInstance->OnMontageBlendingOutStarted.Unbind();
                MontageInstance->OnMontageEnded.Unbind();
            }

            AbilitySystemComponent->CurrentMontageStop();
            return true;
        }
    }

    return false;
}

UNAbilitySystemComponent* UNPlayMontageAndWaitForEventTask::GetTargetASC()
{
    return Cast<UNAbilitySystemComponent>(AbilitySystemComponent);
}

void UNPlayMontageAndWaitForEventTask::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
    {
        if (Montage == MontageToPlay)
        {
            AbilitySystemComponent->ClearAnimatingAbility(Ability);
        }

        ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
        if (Character && (Character->GetLocalRole() == ROLE_Authority ||
            (Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
        {
            Character->SetAnimRootMotionTranslationScale(1.f);
        }
    }

    if (bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }
    else
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }
}

void UNPlayMontageAndWaitForEventTask::OnAbilityCancelled()
{
    if (StopPlayingMontage())
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }
}

void UNPlayMontageAndWaitForEventTask::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
        }
    }

    EndTask();
}

void UNPlayMontageAndWaitForEventTask::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        FGameplayEventData TempData = *Payload;
        TempData.EventTag = EventTag;

        EventReceived.Broadcast(EventTag, TempData);
    }
}
