// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NCharacterBase.h"
#include "Abilities/NGameplayAbility.h"
#include "Player/NAttributeSetBase.h"
#include "Abilities/NAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/NCharacterMovementComponent.h"
#include "Player/NPlayerState.h"
#include "Sound/SoundCue.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/Components/NMovementSystemComponent.h"
#include "Characters/Components/NCombatComponent.h"


FAutoConsoleVariableRef CVarDebugCharacter(
    TEXT("NRPG.Debug.Character"),
    DebugCharacter,
    TEXT("Print debug information for ANCharacterBase actors: 0 - Off, 1 - Low, 2 - High"),
    ECVF_Cheat
    );


ANCharacterBase::ANCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	bAlwaysRelevant = true;

	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	EffectRemoveOnDeathTag = FGameplayTag::RequestGameplayTag(FName("Effect.RemoveOnDeath"));
	MaxStaminaTag = FGameplayTag::RequestGameplayTag(FName("State.Stats.MaxStamina"));

	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Weapon, ECR_Overlap);
	
	MovementSystemComponent = CreateDefaultSubobject<UNMovementSystemComponent>("MovementSystemComponent");
	CombatComponent = CreateDefaultSubobject<UNCombatComponent>("CombatComponent");
}


void ANCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCharacterBase, MovementSystemComponent);
}


UAbilitySystemComponent* ANCharacterBase::GetAbilitySystemComponent() const
{
	if (ANPlayerState* PC = GetPlayerState<ANPlayerState>())
	{
		return PC->GetAbilitySystemComponent();
	}
	return AbilitySystemComponent;
}


void ANCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}


int32 ANCharacterBase::GetAbilityLevel(ENAbilityInputID AbilityID) const
{
	
	// TODO
	return 1;
}


int32 ANCharacterBase::GetCharacterLevel() const
{
	// TODO
	return 1;
}


float ANCharacterBase::GetHealth() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetHealth();
	}

	return 0.f;
}


float ANCharacterBase::GetMaxHealth() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxHealth();
	}

	return 0.f;
}


float ANCharacterBase::GetMana() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMana();
	}

	return 0.f;
}


float ANCharacterBase::GetMaxMana() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxMana();
	}

	return 0.f;
}


float ANCharacterBase::GetStamina() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetStamina();
	}

	return 0.f;
}


float ANCharacterBase::GetMaxStamina() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxStamina();
	}

	return 0.f;
}


float ANCharacterBase::GetShield() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetShield();
	}

	return 0.f;
}


float ANCharacterBase::GetMaxShield() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMaxShield();
	}

	return 0.f;
}


float ANCharacterBase::GetMoveSpeed() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMoveSpeed();
	}

	return 0.f;
}


float ANCharacterBase::GetMoveSpeedBaseValue() const
{
	if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetMoveSpeedAttribute().GetGameplayAttributeData(AttributeSetBase)->GetBaseValue();
	}

	return 0.0f;
}

void ANCharacterBase::Die()
{
	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	RemoveCharacterAbilities();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->GravityScale = 0;
	GetCharacterMovement()->Velocity = FVector();

	OnCharacterDied.Broadcast(this);

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->AddLooseGameplayTag(DeadTag);
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
	else
	{
		FinishDying();
	}
}

void ANCharacterBase::FinishDying()
{
	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	Destroy();
}


void ANCharacterBase::AddCharacterAbilities()
{
	if (!HasAuthority())
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called on Client. Only call on server."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}

	if (!IsValid(AbilitySystemComponent))
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Invalid AbilitySystemComponent."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Character abilities were already given."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}

	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}

	// Grant all startup abilities
	for (TSubclassOf<UNGameplayAbility>& StartupAbility : CharacterAbilities)
	{
		AbilitySystemComponent->GiveAbility(
            FGameplayAbilitySpec(StartupAbility,
                GetAbilityLevel(StartupAbility.GetDefaultObject()->AbilityID),
                static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID),
                this));
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}


void ANCharacterBase::RemoveCharacterAbilities()
{
	if (!HasAuthority())
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called on Client. Only call on server."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}
	
	if (!IsValid(AbilitySystemComponent))
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Invalid AbilitySystemComponent."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (!AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Character abilities not given, nothing to remove."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}

	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	// Remove any abilities added from a previous call. This checks to make sure the ability is in the startup 'CharacterAbilities' array.
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if ((Spec.SourceObject == this) && CharacterAbilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}

	// Do in two passes so the removal happens after we have the full list
	for (int32 i = 0; i < AbilitiesToRemove.Num(); i++)
	{
		AbilitySystemComponent->ClearAbility(AbilitiesToRemove[i]);
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = false;
}


void ANCharacterBase::InitializeAttributes()
{
	if (!IsValid(AbilitySystemComponent))
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Invalid AbilitySystemComponent."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (!DefaultAttributes)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Missing DefaultAttributes for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName()), EPrintType::ShutDown);
		return;
	}

	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributes, GetCharacterLevel(), EffectContext);
	if (NewHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
	}
}


void ANCharacterBase::AddStartupEffects()
{
	if (!HasAuthority())
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called on Client. Only call on server."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}
	
	if (!IsValid(AbilitySystemComponent))
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Invalid AbilitySystemComponent."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (AbilitySystemComponent->bStartupEffectsApplied)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s StartupEffects are already applied."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}

	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> GameplayEffect : StartupEffects)
	{
		FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, GetCharacterLevel(), EffectContext);
		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), AbilitySystemComponent);
		}
	}

	AbilitySystemComponent->bStartupEffectsApplied = true;
}


void ANCharacterBase::SetHealth(float Health)
{
	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetHealth(Health);
	}
}


void ANCharacterBase::SetMana(float Mana)
{
	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetMana(Mana);
	}
}


void ANCharacterBase::SetStamina(float Stamina)
{
	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetStamina(Stamina);
	}
}


void ANCharacterBase::SetShield(float Shield)
{
	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	if (IsValid(AttributeSetBase))
	{
		AttributeSetBase->SetShield(Shield);
	}
}






