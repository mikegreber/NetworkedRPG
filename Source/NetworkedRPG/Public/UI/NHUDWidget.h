// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NHUDWidget.generated.h"

class UNProgressWidget;
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class ANPlayerController;

	UPROPERTY()
	float Health;

	UPROPERTY()
	float MaxHealth;
	
	UPROPERTY()
	float Shield;

	UPROPERTY()
	float MaxShield;

	UPROPERTY()
	float Mana;

	UPROPERTY()
	float MaxMana;

	UPROPERTY()
	float Stamina;

	UPROPERTY()
	float MaxStamina;

public:

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
	
protected:
	
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	// Attributes to listen for, must be set in blueprint.
	UPROPERTY(EditAnywhere, Category="Attributes")
	TArray<FGameplayAttribute> Attributes;

	// Listner to
	UPROPERTY()
	class UNAsyncTaskAttributeChanged* AttributeChangeListener;

	// Called whenever an attribute changes to deal with updating the UI
	UFUNCTION()
	void AttributeChanged(FGameplayAttribute Attribute, float NewValue, float OldValue);

	void SetHealth(float InHealth);
	void SetMaxHealth(float InMaxHealth);
	void SetShield(float InShield);
	void SetMaxShield(float InMaxShield);
	void SetStamina(float InStamina);
	void SetMaxStamina(float InMaxStamina);
	void SetMana(float InMana);
	void SetMaxMana(float InMaxMana);
	
	void UpdateHealthPercentage(float Current, float Max) const;
	void UpdateShieldPercentage(float Current, float Max) const;
	void UpdateManaPercentage(float Current, float Max) const;
	void UpdateStaminaPercentage(float Current, float Max) const;
	
};
