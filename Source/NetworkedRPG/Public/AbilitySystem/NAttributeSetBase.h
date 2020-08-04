// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NAttributeSetBase.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeUpdated, float, Current, float, Max);

int32 DebugAbilitySystem = 0;

/** Sections
*	1. References
*	2. Components
*	3. State
*	4. Overrides
*	5. Interface and Methods
*/

/**
 * Attribute set for a character.
 */
UCLASS()
class NETWORKEDRPG_API UNAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UNAttributeSetBase();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. References
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	FGameplayTag WeakSpotTag;
	FGameplayTag MaxStaminaTag;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Attributes
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UPROPERTY(BlueprintReadOnly, Category="Damage", ReplicatedUsing=OnRep_Health)	
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, Damage)
	
	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing=OnRep_Health)	
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, Health)

	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing=OnRep_MaxHealth)	
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category="Health", ReplicatedUsing=OnRep_HealthRegenRate)	
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, HealthRegenRate)

	UPROPERTY(BlueprintReadOnly, Category="Shield", ReplicatedUsing=OnRep_Shield)	
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, Shield)

	UPROPERTY(BlueprintReadOnly, Category="Shield", ReplicatedUsing=OnRep_MaxShield)	
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, MaxShield)

	UPROPERTY(BlueprintReadOnly, Category="Shield", ReplicatedUsing=OnRep_ShieldRegenRate)	
	FGameplayAttributeData ShieldRegenRate;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, ShieldRegenRate)
	
	UPROPERTY(BlueprintReadOnly, Category="Mana", ReplicatedUsing=OnRep_Mana)	
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, Mana)

	UPROPERTY(BlueprintReadOnly, Category="Mana", ReplicatedUsing=OnRep_MaxMana)	
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, MaxMana)

	UPROPERTY(BlueprintReadOnly, Category="Mana", ReplicatedUsing=OnRep_ManaRegenRate)	
	FGameplayAttributeData ManaRegenRate;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, ManaRegenRate)
 
	UPROPERTY(BlueprintReadOnly, Category="Stamina", ReplicatedUsing=OnRep_Stamina)	
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, Stamina)

	UPROPERTY(BlueprintReadOnly, Category="Stamina", ReplicatedUsing=OnRep_MaxStamina)	
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category="Stamina", ReplicatedUsing=OnRep_StaminaRegenRate)	
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, StaminaRegenRate)

	UPROPERTY(BlueprintReadOnly, Category="Speed", ReplicatedUsing=OnRep_SprintCost)	
	FGameplayAttributeData SprintCost;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, SprintCost)
	
	UPROPERTY(BlueprintReadOnly, Category="Speed", ReplicatedUsing=OnRep_MoveSpeed)	
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UNAttributeSetBase, MoveSpeed)

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UPROPERTY()
	FOnAttributeUpdated OnHealthUpdated;
	
protected:
	/** Called in PreAttributeChange() to keep current percentage when Max of any attribute is changed. */
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewValue, const FGameplayAttribute& AffectedAttributeProperty) const;

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate);

	UFUNCTION()
	virtual void OnRep_Shield(const FGameplayAttributeData& OldShield);

	UFUNCTION()
	virtual void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

	UFUNCTION()
	virtual void OnRep_ShieldRegenRate(const FGameplayAttributeData& OldShieldRegenRate);
	
	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UFUNCTION()
	virtual void OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	virtual void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate);
	
	UFUNCTION()
	virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	virtual void OnRep_SprintCost(const FGameplayAttributeData& OldSprintCost);
};
