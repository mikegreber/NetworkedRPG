// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NAttributeSetBase.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Characters/NCharacter.h"
#include "Player/NPlayerController.h"
#include "Net/UnrealNetwork.h"


FAutoConsoleVariableRef CVarDebugNAbilitySystem(
    TEXT("NRPG.Debug.AbilitySystem"),
    DebugAbilitySystem,
    TEXT("Print debug information for NAbilitySystem and UNAttributeBase: 0 - Off, 1 - Low, 2 - High"),
    ECVF_Cheat
    );


UNAttributeSetBase::UNAttributeSetBase() :
    MaxStaminaTag(FGameplayTag::RequestGameplayTag("State.Stats.MaxStamina")),
    Damage(0.0),
    Health(500.0),
    MaxHealth(500.0),
    HealthRegenRate(2.0f),
    Shield(500.0),
    MaxShield(500.0),
    ShieldRegenRate(2.0f),
    Mana(300.0),
    MaxMana(300.0),
    ManaRegenRate(2.0f),
    Stamina(100.0),
    MaxStamina(100.0),
    StaminaRegenRate(2.0f),
    MoveSpeed(300.0)
{}


void UNAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
    
    // If a Max value changes, adjust current to keep Current % of Current to Max
    if (Attribute == GetMaxHealthAttribute()) // GetMaxHealthAttribute comes from the Macros defined at the top of the header
    {
        AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
    }
    else if (Attribute == GetMaxManaAttribute())
    {
        AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
    }
    else if (Attribute == GetMaxStaminaAttribute())
    {
        AdjustAttributeForMaxChange(Stamina, MaxStamina, NewValue, GetStaminaAttribute());
    }
    else if (Attribute == GetMoveSpeedAttribute())
    {
        NewValue = FMath::Clamp<float>(NewValue, 150, 1000);
    }
}


void UNAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    FGameplayTagContainer SpecAssetTags;
    Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

    // Get the target actor, should be our owner
    AActor* TargetActor = nullptr;
    AController* TargetController = nullptr;
    ANCharacter* TargetCharacter = nullptr;

    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
        TargetCharacter = Cast<ANCharacter>(TargetActor);
    }

    // Get the source actor
    AActor* SourceActor = nullptr;
    AController* SourceController = nullptr;
    ANCharacter* SourceCharacter = nullptr;
    if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
    {
        SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
        SourceController = Source->AbilityActorInfo->PlayerController.Get();
        if (!SourceController && SourceActor)
        {
            if (APawn* Pawn = Cast<APawn>(SourceActor))
            {
                SourceController = Pawn->GetController();
            }
        }

        // user the controller to find the source pawn
        if (SourceController)
        {
            SourceCharacter = Cast<ANCharacter>(SourceController->GetPawn());
        }
        else
        {
            SourceCharacter = Cast<ANCharacter>(SourceActor);
        }


        // set the causer based on context if it's set
        if (Context.GetEffectCauser())
        {
            SourceActor = Context.GetEffectCauser();
        }
        
    }

    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        const float LocalDamageDone = GetDamage();
        SetDamage(0.0f);
        
        if (LocalDamageDone > 0.0f)
        {
            bool bWasAlive = true;

            if (TargetCharacter)
            {
                bWasAlive = TargetCharacter->IsAlive();
            }

            const float OldShield = GetShield();
            if (OldShield > 0)
            {
                const float NewShield = OldShield - LocalDamageDone;
                SetShield(FMath::Max<float>(NewShield, 0.0f));
                
                if (DebugAbilitySystem)
                {
                    Print(GetWorld(), FString::Printf(TEXT("%s Shield Damaged -> Set to %f / %f"), *FString(__FUNCTION__), GetShield(), GetMaxShield()));
                }
            }

            const float DamageAfterShield = LocalDamageDone - OldShield;
            if (DamageAfterShield > 0)
            {
                const float NewHealth = GetHealth() - DamageAfterShield;
                SetHealth(FMath::Max<float>(NewHealth, 0.0f));

                if (DebugAbilitySystem)
                {
                    Print(GetWorld(), FString::Printf(TEXT("%s Health Damaged -> Set to %f / %f"), *FString(__FUNCTION__), GetHealth(), GetMaxHealth()));
                }
            }

            if (TargetCharacter && bWasAlive)
            {
                if (SourceActor != TargetActor)
                {
                    ANPlayerController* PC = Cast<ANPlayerController>(SourceController);
                    if (PC)
                    {

                        if (Data.EffectSpec.DynamicAssetTags.HasTag(WeakSpotTag))
                        {
                            // TODO add visual indicator 
                        }
                    }
                }

                if (!TargetCharacter->IsAlive())
                {
                    if (SourceController != TargetController)
                    {
                        // TODO Give rewards to source (exp, money, etc..)
                    }
                }
            }
        }
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute() && (GetHealth() < 0.0f || GetHealth() > GetMaxHealth()))
    {
        if (DebugAbilitySystem)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s Setting Health Directly -> Should typically set health loss through Damage."), *FString(__FUNCTION__)), EPrintType::Warning);
        }
        
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
   
        SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
        if (GetStamina() == GetMaxStamina())
        {
            GetOwningAbilitySystemComponent()->AddLooseGameplayTag(MaxStaminaTag);
        }
        else
        {
            GetOwningAbilitySystemComponent()->RemoveLooseGameplayTag(MaxStaminaTag);
        }
    }
    else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
    {
        SetShield(FMath::Clamp<float>(GetShield(), 0.0f, GetMaxShield()));
    }

    
}


void UNAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, Shield, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, MaxShield, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, MaxStamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, MoveSpeed, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNAttributeSetBase, SprintCost, COND_None, REPNOTIFY_Always);
}


void UNAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
    const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
    UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent();
    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilitySystemComponent)
    {
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        const float NewDelta = CurrentValue > 0.0f ? CurrentValue * NewMaxValue / CurrentMaxValue - CurrentValue : NewMaxValue;

        AbilitySystemComponent->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}


void UNAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, Health, OldHealth);
}


void UNAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, MaxHealth, OldMaxHealth);
}


void UNAttributeSetBase::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, HealthRegenRate, OldHealthRegenRate);
}


void UNAttributeSetBase::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, Shield, OldShield);
}


void UNAttributeSetBase::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, MaxShield, OldMaxShield);
}


void UNAttributeSetBase::OnRep_ShieldRegenRate(const FGameplayAttributeData& OldShieldRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, ShieldRegenRate, OldShieldRegenRate);

}


void UNAttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, Mana, OldMana);
}


void UNAttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, MaxMana, OldMaxMana);
}


void UNAttributeSetBase::OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, ManaRegenRate, OldManaRegenRate);
}


void UNAttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, Stamina, OldStamina);
}


void UNAttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, MaxStamina, OldMaxStamina);
}


void UNAttributeSetBase::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, StaminaRegenRate, OldStaminaRegenRate);
}


void UNAttributeSetBase::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, MoveSpeed, OldMoveSpeed);
}


void UNAttributeSetBase::OnRep_SprintCost(const FGameplayAttributeData& OldSprintCost)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, SprintCost, OldSprintCost);
}
