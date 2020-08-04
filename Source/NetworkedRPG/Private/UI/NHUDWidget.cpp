// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NHUDWidget.h"
#include "Player/NPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tasks/NAsyncTaskAttributeChanged.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "AbilitySystem/NAttributeSetBase.h"
#include "UI/NProgressWidget.h"

void UNHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (Attributes.Num() == 0)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s No attributes set in BP."), *FString(__FUNCTION__)), EPrintType::Warning);
        return;
    }
    
    ANPlayerState* PS = GetOwningPlayerState<ANPlayerState>();
    if (!PS)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s PlayerState not initialized"), *FString(__FUNCTION__)), EPrintType::Error);
        return;
    }
    
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    AttributeChangeListener = UNAsyncTaskAttributeChanged::ListenForAttributesChange(ASC, Attributes);
    AttributeChangeListener->OnAttributeChanged.AddDynamic(this, &UNHUDWidget::AttributeChanged);
}


void UNHUDWidget::NativeDestruct()
{
    AttributeChangeListener->EndTask();

    Super::NativeDestruct();
}


void UNHUDWidget::AttributeChanged(FGameplayAttribute Attribute, float NewValue, float OldValue)
{
    const FString AtName = Attribute.GetName();
    if (AtName == "Health")
    {
        SetHealth(NewValue);
    }
    else if (AtName == "Shield")
    {
        SetShield(NewValue);
    }
    else if (AtName == "Mana")
    {
        SetMana(NewValue);
    }
    else if (AtName == "Stamina")
    {
        SetStamina(NewValue);
    }
    else if (AtName == "MaxHealth")
    {
       SetMaxHealth(NewValue);
    }
    else if (AtName == "MaxShield")
    {
        SetMaxShield(NewValue);
    }
    else if (AtName == "MaxMana")
    {
        SetMaxMana(NewValue);
    }
    else if (AtName == "MaxStamina")
    {
        SetMaxStamina(NewValue);
    }
    // else if (AtName == "HealthRegenRate")
    // {
    //     SetHealthRegenRate(NewValue);
    // }
}

void UNHUDWidget::SetHealth(float InHealth)
{
    Health = InHealth;
    UpdateHealthPercentage(Health, MaxHealth);
}

void UNHUDWidget::SetMaxHealth(float InMaxHealth)
{
    MaxHealth = InMaxHealth;
    UpdateHealthPercentage(Health, MaxHealth);
}

void UNHUDWidget::SetShield(float InShield)
{
    Shield = InShield;
    UpdateShieldPercentage(Shield, MaxShield);
}

void UNHUDWidget::SetMaxShield(float InMaxShield)
{
    MaxShield = InMaxShield;
    UpdateShieldPercentage(Shield, MaxShield);
}

void UNHUDWidget::SetStamina(float InStamina)
{
    Stamina = InStamina;
    UpdateStaminaPercentage(Stamina, MaxStamina);
}

void UNHUDWidget::SetMaxStamina(float InMaxStamina)
{
    MaxStamina = InMaxStamina;
    UpdateStaminaPercentage(Stamina, MaxStamina);
}

void UNHUDWidget::SetMana(float InMana)
{
    Mana = InMana;
    UpdateManaPercentage(Mana, MaxMana);
}

void UNHUDWidget::SetMaxMana(float InMaxMana)
{
    MaxMana = InMaxMana;
    UpdateManaPercentage(Mana, MaxMana);
}

void UNHUDWidget::UpdateHealthPercentage(float Current, float Max) const
{
    HealthProgressBar->SetPercentage(Current, Max);
}

void UNHUDWidget::UpdateShieldPercentage(float Current, float Max) const
{
    ShieldProgressBar->SetPercentage(Current, Max);
}

void UNHUDWidget::UpdateManaPercentage(float Current, float Max) const
{
    ManaProgressBar->SetPercentage(Current, Max);
}

void UNHUDWidget::UpdateStaminaPercentage(float Current, float Max) const
{
    StaminaProgressBar->SetPercentage(Current, Max);
    // StaminaProgressBarRaw->SetPercentage(Current, Max);
}


