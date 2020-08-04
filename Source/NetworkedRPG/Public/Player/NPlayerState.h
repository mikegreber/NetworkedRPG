// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerState.h"
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

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. References
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Holds the active ability system component for this player. */
	UPROPERTY()
	UNAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UNAttributeSetBase* AttributeSetBase;
	
	FGameplayTag DeadTag;
	
	FDelegateHandle HealthChangedDelegateHandle;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Called when the player state is created. */
	virtual void BeginPlay() override;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Sets the default values for the PlayerState. */
	ANPlayerState();
	
	// Implement IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Returns the attributeS */
	UNAttributeSetBase* GetAttributeSetBase() const; 

	/** Returns true if health > 0. */
	bool IsAlive() const;

	/** Returns Health from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetHealth() const;

	/** Returns MaxHealth from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetMaxHealth() const;

	/** Returns HealthRegenRate from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetHealthRegenRate() const;

	/** Returns Mana from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetMana() const;

	/** Returns MaxMana from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetMaxMana() const;

	/** Returns ManaRegenRate from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetManaRegenRate() const;

	/** Returns Stamina from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetStamina() const;

	/** Returns MaxStamina from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetMaxStamina() const;

	/** Returns StaminaRegenRate from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetStaminaRegenRate() const;

	/** Returns Shield from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetShield() const;

	/** Returns MaxShield from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetMaxShield() const;

	/** Returns ShieldRegenRate from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetShieldRegenRate() const;

	/** Returns Armour from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetArmor() const;

	/** Returns MoveSpeed from AttributeSet. */
	UFUNCTION(BlueprintCallable, Category = "NPlayerState|Attributes")
	float GetMoveSpeed() const;
	
protected:
	/** Called when health changes. */
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
};
