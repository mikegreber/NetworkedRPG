// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NGameplayAbilityTypes.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "Abilities/GameplayAbility.h"
#include "NGameplayAbility.generated.h"

class USkeletalMeshComponent;
class UAnimMontage;
class INGameplayAbilityActorInterface;
class ANCharacterBase;
class ACharacter;
class UNCombatComponent;

USTRUCT()
struct NETWORKEDRPG_API FAbilityMeshMontage
{
	GENERATED_BODY()

	FAbilityMeshMontage():
		Mesh(nullptr),
		Montage(nullptr)
	{}

	FAbilityMeshMontage(USkeletalMeshComponent* InMesh, UAnimMontage* InMontage):
		Mesh(InMesh),
		Montage(InMontage)
	{}	

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	UAnimMontage* Montage;
};

/**
 * Adds additional functionality to UGameplayAbility.
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UNGameplayAbility();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Abilities with this will automatically activate when the input is pressed */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|Ability")
	ENAbilityInputID AbilityInputID;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|Ability")
	ENAbilityInputID AbilityID;
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Ability")
	bool bActivateAbilityOnGranted;

	UPROPERTY(EditAnywhere, Category="Settings|Ability")
	bool bActivateOnInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|GameplayEffects")
	TMap<FGameplayTag, FNGameplayEffectContainer> EffectContainerMap;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. References and State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Activated version of EffectsContainerMap. */
	TMap<FGameplayTag, FNGameplayEffectContainerSpec> EffectContainerSpecMap;

	UPROPERTY()
	ACharacter* OwningCharacter;

	UPROPERTY()
	TScriptInterface<INGameplayAbilityActorInterface> Interface;
	
	UPROPERTY()
	UNCombatComponent* CombatComponent;
	
	/** Active montages being played by this ability */
	UPROPERTY()
	TArray<FAbilityMeshMontage> CurrentAbilityMeshMontages;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Returns the owning */
	UFUNCTION(BlueprintCallable)
	UNCombatComponent* GetOwnerCombatComponent() const;
	
	/** Returns the rotation of the owner actor */
	UFUNCTION(BlueprintCallable)
	FRotator GetActorRotation() const;

	/** Returns the control rotation of the owner actor*/
	UFUNCTION(BlueprintCallable)
	FRotator GetControlRotation() const;

	/** Returns the input direction from the owner movement component. If no input, will return the direction from the Actor to the camera. */
	UFUNCTION(BlueprintCallable)
	FRotator GetInputDirection() const;

	/** Sets the effect level of the GameplayEffects in the EffectContainerSpecMap matching the ContainerTag. */
	void SetEffectLevel(const FGameplayTag& ContainerTag, const float Level);

	/** Creates an FNGameplayEffectContainerSpec from an FNGameplayEffectContainer. This can be used to "activate" effects to be applied later. */
	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData"))
	virtual FNGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FNGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	/** Creates an FNGameplayEffectContainerSpec from the FNGameplayEffectContainer in the EffectContainerMap matching the ContainerTag. This can be used to "activate" effects to be applied later. */
	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData"))
	virtual FNGameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	/** Applies all the GameplayEffectSpecs in the container its target data. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FNGameplayEffectContainerSpec& ContainerSpec);

	/** Applies all the GameplayEffectSpecs in the container matching the ContainerTag to its target data. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpecFromTag(const FGameplayTag& ContainerTag);

	/** Exposes GetSourceObject to BP. Returns the Source Object associated with this ability. Works on non-instanced abilities. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability", meta = (DisplayName = "Get Source Object"))
	UObject* K2_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	/** Attempts to activate the given ability handle and batch all RPCs into one. This will only batch all RPCs that happen
	  * in one frame. Best case scenario we batch ActivateAbility, SendTargetData, and EndAbility into one RPC instead of three. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

	/** Ends the ability. Meant for use with batching system to end the ability externally. */
	virtual void ExternalEndAbility();

	/** Returns the current prediction key and if it's valid for more predicting. Used for debugging ability prediction windows. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FString GetCurrentPredictionKeyStatus();

	/** Returns true if the current prediction key is valid for more predicting. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;

	/** Allows C++ and Blueprint abilities to override how cost is checked. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	bool NCheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool NCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const { return true; }

	/** Allows C++ and Blueprint abilities to override how cost is applied. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void NApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
	virtual void NApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const {};

	/** Returns the currently playing montage for this ability, if any */
	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	/** Call to set/get the current montage from a montage task. Set to allow hooking up montage events to ability events */
	virtual void SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InCurrentMontage);

protected:
	/** Finds the FAbilityMeshMontages currently playing on the input mesh. Returns true if a montage is found. */
	bool FindAbilityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& OutAbilityMeshMontage);

	/** Immediately jumps the active montage to a section */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageJumptToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);

	/** Sets pending section on active montage */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);

	/**
	* Stops the current animation montage.
	*
	* @param OverrideBlendOutTime If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
	*/
	UFUNCTION(BlueprintCallable, Category="Ability|Animation", meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);

	/**
	* Stops all currently animating montages
	*
	* @param OverrideBlendOutTime If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForAllMeshes(float OverrideBlendOutTime = -1.0f);

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Static Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION(BlueprintCallable, Category = "Ability")
	static FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromActorArray(const TArray<AActor*> TargetActors);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	static FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromHitResult(const TArray<FHitResult> HitResults);
};
