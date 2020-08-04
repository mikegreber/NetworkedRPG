// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "NHUDWidget.generated.h"

class UNProgressWidget;
class UNAsyncTaskAttributeChanged;
struct FGameplayAttribute;

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class ANPlayerController;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Attributes to listen for. Must be set to update any values when attributes change. */
	UPROPERTY(EditAnywhere, Category="Settings|Attributes")
	TArray<FGameplayAttribute> Attributes;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. References
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Listens for any attribute changes and fires */
	UPROPERTY()
	UNAsyncTaskAttributeChanged* AttributeChangeListener;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Widget Components
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(meta = (BindWidget))
	UNProgressWidget* HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	UNProgressWidget* ShieldProgressBar;

	UPROPERTY(meta = (BindWidget))
	UNProgressWidget* ManaProgressBar;

	UPROPERTY(meta = (BindWidget))
	UNProgressWidget* StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	UNProgressWidget* StaminaProgressBarRaw;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	float Health;
	float MaxHealth;
	float Shield;
	float MaxShield;
	float Mana;
	float MaxMana;
	float Stamina;
	float MaxStamina;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Initialize AttributeListener. */
	virtual void NativeConstruct() override;

	/** End AttributeListener. */
	virtual void NativeDestruct() override;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 6. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:

	/** Called whenever an attribute changes - Updates the UI. */
	UFUNCTION()
	void AttributeChanged(FGameplayAttribute Attribute, float NewValue, float OldValue);

	/** Sets new value and then updates percentage in UI. */
	void SetHealth(float InHealth);
	void SetMaxHealth(float InMaxHealth);
	void SetShield(float InShield);
	void SetMaxShield(float InMaxShield);
	void SetStamina(float InStamina);
	void SetMaxStamina(float InMaxStamina);
	void SetMana(float InMana);
	void SetMaxMana(float InMaxMana);

private:
	/** Called from within setters above to actually update the UI. */
	void UpdateHealthPercentage(float Current, float Max) const;
	void UpdateShieldPercentage(float Current, float Max) const;
	void UpdateManaPercentage(float Current, float Max) const;
	void UpdateStaminaPercentage(float Current, float Max) const;
	
};
