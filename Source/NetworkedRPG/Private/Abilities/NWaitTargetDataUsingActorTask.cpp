// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NWaitTargetDataUsingActorTask.h"
#include "AbilitySystemComponent.h"
#include "Abilities/NGATA_Trace.h"

UNWaitTargetDataUsingActorTask* UNWaitTargetDataUsingActorTask::WaitTargetDataUsingActorTask(
    UGameplayAbility* OwningAbility, FName TaskInstanceName,
    TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor,
    bool bCreateKeyIfNotValidForMorePredicting)
{
    UNWaitTargetDataUsingActorTask* MyObj = NewAbilityTask<UNWaitTargetDataUsingActorTask>(OwningAbility, TaskInstanceName);    //Register for task list here, providing a given FName as a key
    MyObj->TargetActor = InTargetActor;
    MyObj->ConfirmationType = ConfirmationType;
    MyObj->bCreateKeyIfNotValidForMorePredicting = bCreateKeyIfNotValidForMorePredicting;
    return MyObj;
}

void UNWaitTargetDataUsingActorTask::Activate()
{
    if (IsPendingKill())
    {
        return;
    }

    if (Ability && TargetActor)
    {
        InitializeTargetActor();
        RegisterTargetDataCallbacks();
        FinalizeTargetActor();
    }
    else
    {
        EndTask();
    }
}

void UNWaitTargetDataUsingActorTask::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data,
    FGameplayTag ActivationTag)
{
    check(AbilitySystemComponent);

    FGameplayAbilityTargetDataHandle MutableData = Data;
    AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

    /**
    *  Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting
    *	the replicated target data and will treat this as a cancel.
    *
    *	This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could do an actual
    *	trace/check/whatever server side and use that data. So rather than having the client send that data
    *	explicitly, the client is basically just sending a 'confirm' and the server is now going to do the work
    *	in OnReplicatedTargetDataReceived.
    */
    if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            Cancelled.Broadcast(MutableData);
        }
    }
    else
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            ValidData.Broadcast(MutableData);
        }
    }

    if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
    {
        EndTask();
    }
}

void UNWaitTargetDataUsingActorTask::OnTargetDataReplicatedCancelledCallback()
{
    check(AbilitySystemComponent);
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
    }
    EndTask();
}

void UNWaitTargetDataUsingActorTask::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data)
{
    check(AbilitySystemComponent);
    if (!Ability)
    {
        return;
    }

    FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent, ShouldReplicateDataToServer() && (bCreateKeyIfNotValidForMorePredicting && !AbilitySystemComponent->ScopedPredictionKey.IsValidForMorePrediction()));

    const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
    if (IsPredictingClient())
    {
        if (!TargetActor->ShouldProduceTargetDataOnServer)
        {
            FGameplayTag ApplicationTag;
            AbilitySystemComponent->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, AbilitySystemComponent->ScopedPredictionKey);
        }
        else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
        {
            // We aren't going to send the target data, but we will send a generic confirmed message.
            AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
        }
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(Data);
    }

    if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
    {
        EndTask();
    }
}

void UNWaitTargetDataUsingActorTask::OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data)
{
    check(AbilitySystemComponent);
    
    FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent, IsPredictingClient());

    if (IsPredictingClient())
    {
        if (!TargetActor->ShouldProduceTargetDataOnServer)
        {
            AbilitySystemComponent->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
        }
        else
        {
            AbilitySystemComponent->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey);
        }
    }

    Cancelled.Broadcast(Data);
    EndTask();
}

void UNWaitTargetDataUsingActorTask::ExternalConfirm(bool bEndTask)
{
    check(AbilitySystemComponent);
    if (TargetActor)
    {
        if (TargetActor->ShouldProduceTargetData())
        {
            TargetActor->ConfirmTargetingAndContinue();
        }
    }
    Super::ExternalConfirm(bEndTask);
}

void UNWaitTargetDataUsingActorTask::ExternalCancel()
{
    check(AbilitySystemComponent);
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
    }

    Super::ExternalCancel();
}

void UNWaitTargetDataUsingActorTask::InitializeTargetActor() const
{
    check(TargetActor);
    check(Ability);

    TargetActor->MasterPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

    // If we spawned the target actor, always register the callbacks for when the data is ready.
    TargetActor->TargetDataReadyDelegate.AddUObject(const_cast<UNWaitTargetDataUsingActorTask*>(this), &UNWaitTargetDataUsingActorTask::OnTargetDataReadyCallback);
    TargetActor->CanceledDelegate.AddUObject(const_cast<UNWaitTargetDataUsingActorTask*>(this), &UNWaitTargetDataUsingActorTask::OnTargetDataCancelledCallback);
}

void UNWaitTargetDataUsingActorTask::FinalizeTargetActor() const
{
    check(TargetActor);
    check(Ability);

    TargetActor->StartTargeting(Ability);

    if (TargetActor->ShouldProduceTargetData())
    {
        // If instant confirm, then stop targeting immediately.
        if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
        {
            TargetActor->ConfirmTargeting();
        }
        else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
        {
            // Bind to Confirm/Cancel Delegates (called from local confirm or from replicated confirm)
            TargetActor->BindToConfirmCancelInputs();
        }
    }
}

void UNWaitTargetDataUsingActorTask::RegisterTargetDataCallbacks()
{
    if (!ensure(IsPendingKill()) == false)
    {
        return;
    }

    check(Ability);

    const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
    const bool bShouldProduceTargetDataOnServer = TargetActor->ShouldProduceTargetDataOnServer;

    if (!bIsLocallyControlled)
    {

        if (!bShouldProduceTargetDataOnServer)
        {
            FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
            FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

            //Since multifire is supported, we still need to hook up the callbacks
            AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UNWaitTargetDataUsingActorTask::OnTargetDataReplicatedCallback);
            AbilitySystemComponent->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UNWaitTargetDataUsingActorTask::OnTargetDataReplicatedCancelledCallback);

            AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

            SetWaitingOnRemotePlayerData();
        }
    }
}

void UNWaitTargetDataUsingActorTask::OnDestroy(bool bInOwnerFinished)
{
    if (TargetActor)
    {
        ANGATA_Trace* TraceTargetActor = Cast<ANGATA_Trace>(TargetActor);
        if (TraceTargetActor)
        {
            TraceTargetActor->StopTargeting();
        }
        else
        {
            // TargetActor doesn't have a StopTargeting function
            TargetActor->SetActorTickEnabled(false);

            // Clear added callbacks
            TargetActor->TargetDataReadyDelegate.RemoveAll(this);
            TargetActor->CanceledDelegate.RemoveAll(this);

            AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
            AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
            TargetActor->GenericDelegateBoundASC = nullptr;
        }
    }

    Super::OnDestroy(bInOwnerFinished);
}

bool UNWaitTargetDataUsingActorTask::ShouldReplicateDataToServer() const
{
    if (!Ability || !TargetActor)
    {
        return false;
    }

    const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
    if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
    {
        return true;
    }

    return false;
}
