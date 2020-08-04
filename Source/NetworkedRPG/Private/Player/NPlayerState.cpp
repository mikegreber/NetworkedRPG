// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NPlayerState.h"
#include "AbilitySystem/NAttributeSetBase.h"
#include "Characters/NCharacter.h"
#include "AbilitySystem/NAbilitySystemComponent.h"

ANPlayerState::ANPlayerState()
{
    // Create ability system component, and set it to be explicitly replicated
    AbilitySystemComponent = CreateDefaultSubobject<UNAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    // Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another PlayerState receives a GE,
    // we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // Create the attribute set, this replicates by default
    // Adding it as a subobject of the owning actor of an AbilitySystemComponent
    // automatically registers the AttributeSet with the AbilitySystemComponent
    AttributeSetBase = CreateDefaultSubobject<UNAttributeSetBase>(TEXT("AttributeSetBase"));

    // Net update frequency is low by default for PlayerState Since we are holding the ability system and attributes here, we must set a high value.
    NetUpdateFrequency = 100.0f;

    // Cache tags
    DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
}


void ANPlayerState::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this, &ANPlayerState::HealthChanged);
    }
}


UAbilitySystemComponent* ANPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}


UNAttributeSetBase* ANPlayerState::GetAttributeSetBase() const
{
    return AttributeSetBase;
}


bool ANPlayerState::IsAlive() const
{
    return GetHealth() > 0;
}


float ANPlayerState::GetHealth() const
{
    return AttributeSetBase->GetHealth();
}


float ANPlayerState::GetMaxHealth() const
{
    return AttributeSetBase->GetMaxHealth();
}


float ANPlayerState::GetHealthRegenRate() const
{
    return AttributeSetBase->GetHealthRegenRate();
}


float ANPlayerState::GetMana() const
{
    return AttributeSetBase->GetMana();
}


float ANPlayerState::GetMaxMana() const
{
    return AttributeSetBase->GetMaxMana();
}


float ANPlayerState::GetManaRegenRate() const
{
    return AttributeSetBase->GetManaRegenRate();

}


float ANPlayerState::GetStamina() const
{
    return AttributeSetBase->GetStamina();
}


float ANPlayerState::GetMaxStamina() const
{
    return AttributeSetBase->GetMaxStamina();

}


float ANPlayerState::GetStaminaRegenRate() const
{
    return AttributeSetBase->GetStaminaRegenRate();

}


float ANPlayerState::GetShield() const
{
    return AttributeSetBase->GetShield();
}


float ANPlayerState::GetMaxShield() const
{
    return AttributeSetBase->GetMaxShield();
}


float ANPlayerState::GetShieldRegenRate() const
{
    return AttributeSetBase->GetShieldRegenRate();
}


float ANPlayerState::GetArmor() const
{
    // TODO
    return 0.f;
}


float ANPlayerState::GetMoveSpeed() const
{
    return AttributeSetBase->GetMoveSpeed();
}


void ANPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
    if (DebugCharacter > 1)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
    }
    
    ANCharacter* Character = Cast<ANCharacter>(GetPawn());
    if (Character && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
    {
        Character->Die();
    }
}
