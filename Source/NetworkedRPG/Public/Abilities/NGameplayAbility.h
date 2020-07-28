// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/NCharacterBase.h"
#include "Interface/NGameplayAbilityActorInterface.h"
#include "NetworkedRPG/NetworkedRPG.h"

#include "NGameplayAbility.generated.h"


class USkeletalMeshComponent;
class UAnimMontage;
class UNCombatComponent;

USTRUCT()
struct NETWORKEDRPG_API FAbilityMeshMontage
{
	GENERATED_BODY()

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	UAnimMontage* Montage;

	FAbilityMeshMontage() : Mesh(nullptr), Montage(nullptr)
	{
	}

	FAbilityMeshMontage(USkeletalMeshComponent* InMesh, UAnimMontage* InMontage)
		: Mesh(InMesh), Montage(InMontage)
	{
	}
	
};

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UNGameplayAbility();

public:

	UPROPERTY(EditAnywhere, Category="Ability")
	bool bActivateOnInput;
	
	/** Returns the owning */
	UFUNCTION(BlueprintCallable)
	UNCombatComponent* GetOwnerCombatComponent() const;
	
	/** Returns the rotation of the owner actor */
	UFUNCTION(BlueprintCallable)
	FRotator GetActorRotation();

	/** Returns the control rotation of the owner actor*/
	UFUNCTION(BlueprintCallable)
	FRotator GetControlRotation();

	/** Returns the input direction from the owner movement component. If no input, will return the direction from the Actor to the camera. */
	UFUNCTION(BlueprintCallable)
	FRotator GetInputDirection();
	
	/** Abilities with this will automatically activate when the input is pressed */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ENAbilityInputID AbilityInputID = ENAbilityInputID::None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ENAbilityInputID AbilityID = ENAbilityInputID::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateAbilityOnGranted;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	bool bSourceObjectMustEqualCurrentWeaponToActivate;

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromActorArray(const TArray<AActor*> TargetActors);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromHitResult(const TArray<FHitResult> HitResults);
	
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffects")
	// TMap<FGameplayTag, FNGameplayEffectContainer> EffectContainerMap;

	// UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData"))
	// virtual FNGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FNGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);
	//
	// UFUNCTION(BlueprintCallable, Category = "Ability", meta = (AutoCreateRefTerm = "EventData"))
	// virtual FNGameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);
	//
	// UFUNCTION(BlueprintCallable, Category = "Ability")
	// virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FNGameplayEffectContainerSpec& ContainerSpec);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability", meta = (DisplayName = "Get Source Object"))
	UObject* K2_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

	virtual void ExternalEndAbility();

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FString GetCurrentPredictionKeyStatus();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	bool NCheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool NCheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void NApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
	virtual void NApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const {};

	// UFUNCTION(BlueprintCallable, Category = "Ability")
	// virtual void SetHUDReticle(TSubclassOf<class NHUDReticle> ReticleClass);

	// UFUNCTION(BlueprintCallable, Category="Ability")
	// virtual void ResetHUDReticle();


	/** Returns the currently playing montage for this ability, if any */
	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	/** Call to set/get the current montage from a montage task. Set to allow hooking up montage events to ability events */
	virtual void SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, class UAnimMontage* InCurrentMontage);

protected:

	UPROPERTY()
	// ANCharacterBase* OwningCharacter;
	ACharacter* OwningCharacter;

	UPROPERTY()
	TScriptInterface<INGameplayAbilityActorInterface> Interface;
	
	UPROPERTY()
	UNCombatComponent* CombatComponent;

	UPROPERTY()
	UPrimitiveComponent* LeftHandCollisionComponent;

	UPROPERTY()
    UPrimitiveComponent* RightHandCollisionComponent;
	
	/** Active montages being played by this ability */
	UPROPERTY()
	TArray<FAbilityMeshMontage> CurrentAbilityMeshMontages;

	bool FindAbilityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMeshMontage);

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
	
	

	
};
