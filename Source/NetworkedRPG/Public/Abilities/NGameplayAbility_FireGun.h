// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/NGameplayAbility.h"
#include "NGameplayAbility_FireGun.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility_FireGun : public UNGameplayAbility
{
	GENERATED_BODY()

public:
    UNGameplayAbility_FireGun();

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
    UAnimMontage* FireMontage;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
    TSubclassOf<UGameplayEffect> DamageGameplayEffect;

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    float Range;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    float Damage;

    UFUNCTION()
    void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

    UFUNCTION()
    void OnComplete(FGameplayTag EventTag, FGameplayEventData EventData);

    UFUNCTION()
    void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSpawnProjectile(FVector_NetQuantize SpawnVector);
};
