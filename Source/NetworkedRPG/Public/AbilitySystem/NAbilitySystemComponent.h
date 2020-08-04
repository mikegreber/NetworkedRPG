// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NAbilitySystemComponent.generated.h"

class USkeletalMeshComponent;

USTRUCT()
struct NETWORKEDRPG_API FGameplayAbilityLocalAnimMontageForMesh
{
	GENERATED_BODY();

	FGameplayAbilityLocalAnimMontageForMesh():
		Mesh(nullptr)
	{}
	
	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh):
		Mesh(InMesh)
	{}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh, FGameplayAbilityLocalAnimMontage& InLocalMontageInfo):
		Mesh(InMesh),
		LocalMontageInfo(InLocalMontageInfo)
	{}
	
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalMontageInfo;
};


USTRUCT()
struct NETWORKEDRPG_API FGameplayAbilityRepAnimMontageForMesh
{
	GENERATED_BODY();

	FGameplayAbilityRepAnimMontageForMesh():
		Mesh(nullptr)
	{}

	FGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* const Mesh):
		Mesh(Mesh)
	{}
	
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityRepAnimMontage RepMontageInfo;
};

/** Sections
*	1. State
*	2. Overrides
*	3. Interface and Methods
*	4. Server RPC's
*/

/**
 * Extended AbilitySystemComponent.
 */
UCLASS()
class NETWORKEDRPG_API UNAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UNAbilitySystemComponent();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	bool bCharacterAbilitiesGiven = false;
	bool bStartupEffectsApplied = false;

protected:
	/** Set if montage rep happens while we don't have the animinstance associated with us yet */
	UPROPERTY()
	bool bPendingMontageRepForMesh;

	/** Data structure for montages that were instigated locally (everything if server, predictive if client. replicated if simulated proxy)
	  * Will be max one element per skeletal mesh on the AvatarActor. */
	UPROPERTY()
	TArray<FGameplayAbilityLocalAnimMontageForMesh> LocalAnimMontageInfoForMeshes;

	/** **Replicated** Data structure for replicating montage info to simulated clients
	  * Will be max one element per skeletal mesh on the AvatarActor. */
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAnimMontageForMesh)
	TArray<FGameplayAbilityRepAnimMontageForMesh> RepAnimMontageInfoForMeshes;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool GetShouldTick() const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	
	/** Turn on RPC batching in ASC. Off by default. */
	virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	// Version of function in AbilitySystemGlobals that returns correct type
	static UNAbilitySystemComponent* GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = false);

	// Exposes GetTagCount to Blueprint
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Abilities", Meta = (DisplayName = "GetTagCount", ScriptName = "GetTagCount"))
	int32 K2_GetTagCount(FGameplayTag TagToCheck) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Abilities")
	FGameplayAbilitySpecHandle FindAbilitySpecHandleForClass(TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject = nullptr);

	// Exposes AddLooseGameplayTag to Blueprint. This tag is *not* replicated.
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "AddLooseGameplayTag"))
	void K2_AddLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count = 1);

	// Exposes AddLooseGameplayTags to Blueprint. These tags are *not* replicated.
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "AddLooseGameplayTags"))
	void K2_AddLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count = 1);

	// Exposes RemoveLooseGameplayTag to Blueprint. This tag is *not* replicated.
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "RemoveLooseGameplayTag"))
	void K2_RemoveLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count = 1);

	// Exposes RemoveLooseGameplayTags to Blueprint. These tags are *not* replicated.
	UFUNCTION(BlueprintCallable, Category = "Abilities", Meta = (DisplayName = "RemoveLooseGameplayTags"))
	void K2_RemoveLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count = 1);

	// Attempts to activate the given ability handle and batch all RPCs into one. This will only batch all RPCs that happen
	// in one frame. Best case scenario we batch ActivateAbility, SendTargetData, and EndAbility into one RPC instead of three.
	// Worst case we batch ActivateAbility and SendTargetData into one RPC instead of two and call EndAbility later in a separate
	// RPC. If we can't batch SendTargetData or EndAbility with ActivateAbility because they don't happen in the same frame due to
	// latent ability tasks for example, then batching doesn't help and we should just activate normally.
	// Single shots (semi auto fire) combine ActivateAbility, SendTargetData, and EndAbility into one RPC instead of three.
	// Full auto shots combine ActivateAbility and SendTargetData into one RPC instead of two for the first bullet. Each subsequent
	// bullet is one RPC for SendTargetData. We then send one final RPC for the EndAbility when we're done firing.
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

	//---------------------------------------------------
	// Gameplay Cues
	// --------------------------------------------------
	
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters GameplayCueParameters) const;
	
	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters GameplayCueParameters) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayCue", Meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters GameplayCueParameters) const;

	/** Returns the AnimInstance belonging to the owners mesh. */
	UAnimInstance* GetMeshAnimInstance(USkeletalMeshComponent* InMesh) const;

	// ----------------------------------------------------------------------------------------------------------------
	//	AnimMontage Support for multiple USkeletalMeshComponents on the AvatarActor.
	//  Only one ability can be animating at a time though?
	// ----------------------------------------------------------------------------------------------------------------	

	/** Plays a montage and handles replication and prediction based on passed in ability/activation info. */
	virtual float PlayMontageForMesh(UGameplayAbility* AnimatingAbility, USkeletalMeshComponent* InMesh, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None, bool bReplicateMontage = true);

	/* Plays a montage without updating replication/prediction structures. Used by simulated proxies when replication tells them to play a montage. */
	virtual float PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None);

	/** Stops whatever montage is currently playing. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check). */
	virtual void CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);

	/** Stops all montages currently playing. */
	virtual void StopAllCurrentMontages(float OverrideBlendOutTime = -1.0f);

	/** Stops current montage if it's the one given as the Montage param. */
	virtual void StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh, const UAnimMontage& Montage, float OverrideBlendOutTime = -1.0f);

	/** Clear the animating ability that is passed in, if it's still currently animating. */
	virtual void ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability);

	/** Jumps current montage to given section. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check). */
	virtual void CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);

	/** Sets current montages next section name. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check). */
	virtual void CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);

	/** Sets current montage's play rate. */
	virtual void CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate);

	/** Returns true if the passed in ability is the current animating ability. */
	bool IsAnimatingAbilityForAnyMesh(UGameplayAbility* Ability) const;

	/** Returns the current animating ability. */
	UGameplayAbility* GetAnimatingAbilityFromAnyMesh();

	/** Returns montages that are currently playing. */
	TArray<UAnimMontage*> GetCurrentMontages() const;

	/** Returns the montage that is playing for the mesh. */
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	/** Get SectionID of currently playing AnimMontage. */
	int32 GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh);

	/** Get SectionName of currently playing AnimMontage. */
	FName GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh);

	/** Get length in time of current section. */
	float GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh);

	/** Returns amount of time left in current section. */
	float GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh);

protected:

	/** Finds the existing FGameplayAbilityLocalAnimMontageForMesh for the mesh or creates one if it does not exist */
	FGameplayAbilityLocalAnimMontageForMesh& GetLocalAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh);

	/** Finds the existing FGameplayAbilityRepAnimMontageForMesh for the mesh or creates one if it does not exist */
	FGameplayAbilityRepAnimMontageForMesh& GetGameplayAbilityRepAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh);

	/** Called when a prediction key that played a montage is rejected. */
	void OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* PredictiveMontage);

	/** Copy LocalAnimMontageInfo into RepAnimMontageInfo. */
	void AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh);
	void AnimMontage_UpdateReplicatedDataForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);

	/** Copy over playing flags for duplicate animation data. */
	void AnimMontage_UpdateForcedPlayFlagsForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);

	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontageForMesh();

	/** Returns true if we are ready to handle replicated montage information. */
	virtual bool IsReadyForReplicatedMontageForMesh();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Server RPC's
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** Replicates CurrentMontageSetNextSectionNameForMesh to other clients */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName);
	void ServerCurrentMontageSetNextSectionNameForMesh_Implementation(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName);
	bool ServerCurrentMontageSetNextSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName);

	/** Replicates CurrentMontageJumpToSectionNameForMesh to other clients */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageJumpToSectionNameForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName);
	void ServerCurrentMontageJumpToSectionNameForMesh_Implementation(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName);
	bool ServerCurrentMontageJumpToSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName);

	/** Replicates CurrentMontageSetPlayRateForMesh to other clients */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate);
	void ServerCurrentMontageSetPlayRateForMesh_Implementation(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate);
	bool ServerCurrentMontageSetPlayRateForMesh_Validate(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate);	
};
