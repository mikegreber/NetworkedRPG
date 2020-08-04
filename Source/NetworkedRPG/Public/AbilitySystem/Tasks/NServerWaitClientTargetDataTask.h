// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "NServerWaitClientTargetDataTask.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNServerWaitClientTargetDataTask : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataDelegate ValidData;

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UNServerWaitClientTargetDataTask* ServerWaitClientTargetDataTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool TriggerOnce);

	virtual void Activate() override;

	UFUNCTION()
	void OnTargetDataReplicationCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

protected:
	virtual void OnDestroy(bool bInOwnerFinished) override;

	bool bTriggerOnce;
};
