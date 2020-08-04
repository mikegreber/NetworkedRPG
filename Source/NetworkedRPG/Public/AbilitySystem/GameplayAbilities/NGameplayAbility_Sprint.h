// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"
#include "NGameplayAbility_Sprint.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_Sprint : public UNGameplayAbility
{
	GENERATED_BODY()

protected:

	UPROPERTY()
	bool bEndSprint = false;
	
	UPROPERTY(EditAnywhere, Category="Settings")
	TSubclassOf<UGameplayEffect> GESprint;

	UPROPERTY(EditAnywhere, Category="Settings")
	TSubclassOf<UGameplayEffect> GESprintRemoval;
	
	UPROPERTY(EditAnywhere, Category="Settings")
	FGameplayTag TagSprintEnd;

	UPROPERTY()
	FActiveGameplayEffectHandle GESprintingHandle;

	UPROPERTY()
	class UAbilityTask_WaitGameplayTagAdded* WaitSprintEndTagAddedTask;

	UPROPERTY()
	class UAbilityTask_WaitInputRelease* WaitInputReleaseTask;
	
	UPROPERTY()
	class UAbilityTask_WaitDelay* WaitDelayTask;
	
	UPROPERTY()
	class UAbilityTask_NetworkSyncPoint* SyncPoint;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void ProcessCost();

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnWaitDelayFinish();

	UFUNCTION()
	void OnSync();

	UFUNCTION()
	void OnSprintEnd();

	UFUNCTION()
	void EndAbility();

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

