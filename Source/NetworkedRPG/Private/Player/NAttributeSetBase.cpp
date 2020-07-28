// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NAttributeSetBase.h"
#include "Net/UnrealNetwork.h"
// #include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Player/NCharacter.h"
#include "Player/NPlayerController.h"

UNAttributeSetBase::UNAttributeSetBase()
:
    Health(500.0),
    MaxHealth(500.0),
    Shield(500.0),
    MaxShield(500.0),
    Mana(300.0),
    MaxMana(300.0),
    Stamina(100.0),
    MaxStamina(100.0),
    MoveSpeed(300.0),
    StaminaRegenRate(2.0f),
    ShieldRegenRate(2.0f),
    HealthRegenRate(2.0f),
    ManaRegenRate(2.0f)
{
    MaxStaminaTag = FGameplayTag::RequestGameplayTag(FName("State.Stats.MaxStamina"));
}

void UNAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // UE_LOG(LogTemp, Warning, TEXT("Pre Attribute Change on %d"), GetOwningActor()->HasAuthority())
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
        // Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
        NewValue = FMath::Clamp<float>(NewValue, 150, 1000);
    }
}

void UNAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
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
                UE_LOG(LogTemp, Error, TEXT("Shield Damaged -> Set to %f / %f"), GetShield(), GetMaxShield())
            }

            const float DamageAfterShield = LocalDamageDone - OldShield;
            if (DamageAfterShield > 0)
            {
                const float NewHealth = GetHealth() - DamageAfterShield;
                SetHealth(FMath::Max<float>(NewHealth, 0.0f));
                UE_LOG(LogTemp, Error, TEXT("Health Damaged -> Set to %f / %f"), GetHealth(), GetMaxHealth())

            }

            if (TargetCharacter && bWasAlive)
            {
                if (SourceActor != TargetActor)
                {
                    if (ANPlayerController* PC = Cast<ANPlayerController>(SourceController))
                    {
                        FGameplayTagContainer DamageNumberTags;

                        if (Data.EffectSpec.DynamicAssetTags.HasTag(WeakSpotTag))
                        {
                            DamageNumberTags.AddTagFast(WeakSpotTag);
                        }

                        // PC->ShowDamageDone
                    }
                }

                if (!TargetCharacter->IsAlive())
                {
                    if (SourceController != TargetController)
                    {
                        // Give rewards to source (exp etc)
                    }
                }
            }
            
        }
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute() && (GetHealth() < 0.0f || GetHealth() > GetMaxHealth()))
    {
        // Should process health losses through damage
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
    const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
    UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent();
    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilitySystemComponent)
    {
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        float NewDelta = CurrentValue > 0.0f ? CurrentValue * NewMaxValue / CurrentMaxValue - CurrentValue : NewMaxValue;

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

void UNAttributeSetBase::OnRep_SprintCostCommitsPerSecond(const FGameplayAttributeData& OldMoveSpeed)
{
    // GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, SprintCostCommitsPerSecond, OldMoveSpeed);
}

void UNAttributeSetBase::OnRep_SprintCost(const FGameplayAttributeData& OldSprintCost)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNAttributeSetBase, SprintCost, OldSprintCost);
}
