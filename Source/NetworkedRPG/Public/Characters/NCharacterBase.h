// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Interface/NDamageableInterface.h"
#include "Interface/NGameplayAbilityActorInterface.h"
#include "GameFramework/Character.h"
#include "NCharacterBase.generated.h"

int32 DebugCharacter = 0;

class UNAbilitySystemComponent;
class UNAttributeSetBase;
class UGameplayEffect;
class UNMovementSystemComponent;
class UNCombatComponent;
class ANCharacterBase;
class USoundCue;
class UNGameplayAbility;

/** Sections
* 	1. Blueprint Settings
* 	2. Components
* 	3. References
* 	4. Overrides
* 	5. Interface
* 	6. Protected Methods
*/

/** Character Base class for a character using the Gameplay Ability System. Subclass this for player characters and AI characters. */
UCLASS()
class NETWORKEDRPG_API ANCharacterBase : public ACharacter, public IAbilitySystemInterface, public INGameplayAbilityActorInterface, public INDamageableInterface
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDiedDelegate, ANCharacterBase*, InCharacter);

	
protected:
  /*****************************/
 /**  1. Blueprint Settings  **/
/*****************************/
	/** Determines who can damage who if friendly fire is off */
	UPROPERTY(EditAnywhere, Category="Character")
	ENTeam Team;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character")
	FName CharacterName;

	/** The montage to play when the the character dies. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character|Animation")
	UAnimMontage* DeathMontage;

	/** The sound to play when the the character dies. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character|Audio")
	USoundCue* DeathSound;
	
	/** Default abilities for this Character. Will be removed on Character death and re-given if Character respawns. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character|Abilities")
	TArray<TSubclassOf<UNGameplayAbility>> CharacterAbilities;
 
	/** Default attributes for a character for initializing on spawn or respawn.
	  * This is an instance GE that overrides the values for attributes that get reset on spawn or respawn. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Character|Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributes;

	/* These effects are only applied one time on startup */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Character|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	
  /*****************************/
 /**     2. Components       **/
/*****************************/
	/** **Replicated** */
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, Category="Character|MovementSystemComponent")
	UNMovementSystemComponent* MovementSystemComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|CombatComponent")
	UNCombatComponent* CombatComponent;

	
  /*****************************/
 /**     3. References       **/
/*****************************/
	/** Static gameplay tags */
	FGameplayTag DeadTag;
	FGameplayTag EffectRemoveOnDeathTag;
	FGameplayTag MaxStaminaTag;
	
	/** Reference to the ASC. It will live on the PlayerState or here if the character does not have a PlayerState. */
	UPROPERTY()
	UNAbilitySystemComponent* AbilitySystemComponent;

	/** Reference to the AttributeSetBase. It will live on the PlayerState or here if the character does not have a PlayerState. */
	UPROPERTY()
	UNAttributeSetBase* AttributeSetBase;


  /*****************************/
 /**     4. Overrides        **/
/*****************************/
public:
	/** Replicates MovementSystemComponent */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** INDamageableInterface */
	UFUNCTION(BlueprintCallable)
    virtual ENTeam GetTeam() const override { return Team; }

	/** INGameplayAbilityActorInterface */
	virtual USceneComponent* GetTraceStartComponent() const override { return GetMesh(); };

protected:
    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;

	
  /*****************************/
 /**     5. Interface        **/
/*****************************/
public:
	/** Sets default values for this character's properties */
	explicit ANCharacterBase(const FObjectInitializer& ObjectInitializer);

	/** Returns the owner's MovementSystemComponent. */
	virtual UNMovementSystemComponent* GetMovementSystemComponent() const { return MovementSystemComponent; }

	/** Returns the owner's CombatComponent. */  
	UNCombatComponent* GetCombatComponent() const { return CombatComponent; }

	/** Returns true if health > 0. */
	UFUNCTION(BlueprintCallable, Category="Character")
    virtual bool IsAlive() const { return GetHealth() > 0; }

	/** Fires when Die() is called. */
	FOnCharacterDiedDelegate OnCharacterDied;

	
/** Ability System Getters **/
	
	/** Returns the owners attribute set */
	virtual UNAttributeSetBase* GetAttributeSet() const { return AttributeSetBase; }

	/** NOT IMPLEMENTED. Returns 1. Could be used for leveling of abilities of the character. */
	UFUNCTION(BlueprintCallable, Category="Character")
    virtual int32 GetAbilityLevel(ENAbilityInputID AbilityID) const;
	
	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    int32 GetCharacterLevel() const;
		
	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
    float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Attributes")
	float GetMoveSpeedBaseValue() const;


  /*****************************/
 /**  6. Protected Methods   **/
/*****************************/
protected:
	/** Called on from PlayerState on player death */
	virtual void Die();

	/** Called after death animation */
	UFUNCTION(BlueprintCallable, Category = "Character")
    virtual void FinishDying();

	/** [server] Grants all StartupAbilities the character. */
	virtual void AddCharacterAbilities();

	/** [server] Removes all character abilities. */
	virtual void RemoveCharacterAbilities();
	
	/** [server + client] Initializes the Character's attributes. Must run on Server but we run it on Client too
	  * so that we don't have to wait. The Server's replication to the Client won't matter since
	  * the values should be the same. */
	virtual void InitializeAttributes();

	/** [server] Applies the startup effects to the character */
	virtual void AddStartupEffects();

	/** Setters for Attributes. Only use these in special cases like respawning, otherwise use a GE to change Attributes. */
	virtual void SetHealth(float Health);
	virtual void SetMana(float Mana);
	virtual void SetStamina(float Stamina);
	virtual void SetShield(float Shield);
};
