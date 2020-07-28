// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NServerWaitClientTargetDataTask.h"
#include "AbilitySystemComponent.h"

UNServerWaitClientTargetDataTask* UNServerWaitClientTargetDataTask::ServerWaitClientTargetDataTask( UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce)
{
    UNServerWaitClientTargetDataTask* MyObj = NewAbilityTask<UNServerWaitClientTargetDataTask>(OwningAbility, TaskInstanceName);
    MyObj->bTriggerOnce = TriggerOnce;
    return MyObj;
}

void UNServerWaitClientTargetDataTask::Activate()
{
    if (!Ability || !Ability->GetCurrentActorInfo()->IsNetAuthority())
    {
        return;
    }

    FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
    FPredictionKey ActivationPredicationKey = GetActivationPredictionKey();
    AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredicationKey).AddUObject(this, &UNServerWaitClientTargetDataTask::OnTargetDataReplicationCallback);
}

void UNServerWaitClientTargetDataTask::OnTargetDataReplicationCallback(const FGameplayAbilityTargetDataHandle& Data,
    FGameplayTag ActivationTag)
{
    FGameplayAbilityTargetDataHandle MutableData = Data;
    AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(MutableData);
    }

    if (bTriggerOnce)
    {
        EndTask();
    }
}

void UNServerWaitClientTargetDataTask::OnDestroy(bool bInOwnerFinished)
{
    if (AbilitySystemComponent)
    {
        FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
        FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
        AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).RemoveAll(this);
    }

    Super::OnDestroy(bInOwnerFinished);
}


