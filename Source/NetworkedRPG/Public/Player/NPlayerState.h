// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "NPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNOnGameplayAttributeValueChangedDelegate, FGameplayAttribute, Attribute, float, NewValue, float, OldValue);

class UNAbilitySystemComponent;
class UNAttributeSetBase;
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API ANPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

  /*****************************/
 /**   Reference Variables   **/
/*****************************/
protected:
	/** Holds the active ability system component for this player. */
	UPROPERTY()
	UNAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UNAttributeSetBase* AttributeSetBase;
	
	FGameplayTag DeadTag;
	
	FDelegateHandle HealthChangedDelegateHandle;

	
  /*****************************/
 /**        Overrides        **/
/*****************************/
protected:
	/** Called when the player state is created. */
	virtual void BeginPlay() override;


  /*****************************/
 /**        Interface        **/
/*****************************/
public:
	/** Sets the default values for the PlayerState. */
	ANPlayerState();
	
	// Implement IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Returns the attributeS */
	UNAttributeSetBase* GetAttributeSetBase() const; 

	/** Returns true if health > 0. */
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetHealthRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetManaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetShieldRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetArmor() const;

	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
    float GetMoveSpeed() const;

	
  /*****************************/
 /**     Internal Methods    **/
/*****************************/
protected:
	/** Called when health changes. */
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
};
