// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NPlayMontageAndWaitForEventTask.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNPlayMontageAndWaitForEventTask : public UAbilityTask
{
	GENERATED_BODY()

public:
    UNPlayMontageAndWaitForEventTask();

    virtual void Activate() override;
    virtual void ExternalCancel() override;
    virtual FString GetDebugString() const override;
    virtual void OnDestroy(bool AbilityEnded) override;

    UPROPERTY(BlueprintAssignable)
    FNPlayMontageAndWaitForEventDelegate OnCompleted;

    UPROPERTY(BlueprintAssignable)
    FNPlayMontageAndWaitForEventDelegate OnBlendOut;

    UPROPERTY(BlueprintAssignable)
    FNPlayMontageAndWaitForEventDelegate OnInterrupted;

    UPROPERTY(BlueprintAssignable)
    FNPlayMontageAndWaitForEventDelegate OnCancelled;

    UPROPERTY(BlueprintAssignable)
    FNPlayMontageAndWaitForEventDelegate EventReceived;

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UNPlayMontageAndWaitForEventTask* PlayMontageAndWaitForEvent(UGameplayAbility* OwningAbility,
            FName TaskInstanceName,
            UAnimMontage* MontageToPlay,
            FGameplayTagContainer EventTags,
            float Rate = 1.f,
            FName StartSection = NAME_None,
            bool bStopWhenAbilityEnds = true,
            float AnimRootMotionTranslationScale = 1.f);

    private:
    UPROPERTY()
    UAnimMontage* MontageToPlay;

    /** List of tags to match against gameplay events */
    UPROPERTY()
    FGameplayTagContainer EventTags;

    /** Playback rate */
    UPROPERTY()
    float Rate;

    /** Section to start montage from */
    UPROPERTY()
    FName StartSection;

    /** Modifies how root motion movement to apply */
    UPROPERTY()
    float AnimRootMotionTranslationScale;

    /** Rather montage should be aborted if ability ends */
    UPROPERTY()
    bool bStopWhenAbilityEnds;

    /** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
    bool StopPlayingMontage();

    /** Returns our ability system component */
    UNAbilitySystemComponent* GetTargetASC();

    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
    void OnAbilityCancelled();
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    FOnMontageEnded MontageEndedDelegate;
    FDelegateHandle CancelledHandle;
    FDelegateHandle EventHandle;
    
};
