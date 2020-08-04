// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NWaitTargetDataUsingActorTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTargetDataUsingActorDelegate, const FGameplayAbilityTargetDataHandle&, Data);


/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNWaitTargetDataUsingActorTask : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate ValidData;

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate Cancelled;

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UNWaitTargetDataUsingActorTask* WaitTargetDataUsingActorTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor, bool bCreateKeyIfNotValidForMorePredicting = false);

	virtual void Activate() override;

	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();

	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);

	virtual void ExternalConfirm(bool bEndTask) override;

	virtual void ExternalCancel() override;

protected:
	UPROPERTY()
	AGameplayAbilityTargetActor* TargetActor;

	bool bCreateKeyIfNotValidForMorePredicting;

	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	FDelegateHandle OnTargetDataReplicatedCallbackDelegateHandle;

	virtual void InitializeTargetActor() const;
	virtual void FinalizeTargetActor() const;

	virtual void RegisterTargetDataCallbacks();

	virtual void OnDestroy(bool bInOwnerFinished) override;

	virtual bool ShouldReplicateDataToServer() const;
};
