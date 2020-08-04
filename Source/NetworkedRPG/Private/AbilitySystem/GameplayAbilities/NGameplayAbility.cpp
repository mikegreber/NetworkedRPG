// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbilityTypes.h"
#include "AbilitySystem/Targeting/NTargetTypes.h"
#include "AbilitySystem/NAbilitySystemComponent.h"
#include "Components/Combat/NCombatComponent.h"
#include "Characters/NCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"



UNGameplayAbility::UNGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bActivateAbilityOnGranted = false;
}


void UNGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);

    if (!ActorInfo->AvatarActor.IsValid())
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s Avatar actor not valid."), *FString(__FUNCTION__), *GetName()), EPrintType::Error);
        return;
    }

    OwningCharacter = Cast<ACharacter>(ActorInfo->AvatarActor);
    if (!OwningCharacter)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s Avatar actor no of type ACharacter."), *FString(__FUNCTION__), *GetName()), EPrintType::Error);
        return;
    }

    Interface = OwningCharacter;
    CombatComponent = OwningCharacter->FindComponentByClass<UNCombatComponent>();
     
    if (!CombatComponent && ActorInfo->IsLocallyControlled())
    {
        Print(GetWorld(), FString::Printf(TEXT("%s %s CombatComponent not found."), *FString(__FUNCTION__), *GetName()), EPrintType::Error);
    }
    
    if (bActivateAbilityOnGranted)
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
    }
}


void UNGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);

    FGameplayEventData EventData;
    EventData.Instigator = OwningCharacter;

    // Create the effect container specs
    for (TPair<FGameplayTag, FNGameplayEffectContainer>& Pair : EffectContainerMap)
    {
        EffectContainerSpecMap.Emplace(Pair.Key,MakeEffectContainerSpecFromContainer(Pair.Value, EventData));
    }
}


bool UNGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
    return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) && NCheckCost(Handle, *ActorInfo);
}


void UNGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    NApplyCost(Handle, *ActorInfo, ActivationInfo);
    Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}


UNCombatComponent* UNGameplayAbility::GetOwnerCombatComponent() const
{
        return CombatComponent;  
}


FRotator UNGameplayAbility::GetActorRotation() const
{
      return GetActorInfo().AvatarActor->GetActorRotation().GetNormalized();
}


FRotator UNGameplayAbility::GetControlRotation() const
{
    return GetActorInfo().PlayerController->GetControlRotation().GetNormalized();
}


FRotator UNGameplayAbility::GetInputDirection() const
{
    const FVector InputDirection = Cast<UCharacterMovementComponent>(GetActorInfo().MovementComponent.Get())->GetCurrentAcceleration().GetSafeNormal();

    // TODO Defaults to being towards camera if no acceleration
    if (InputDirection == FVector::ZeroVector)
    {
        return FRotator(0.f,-179.f,0.f) + GetControlRotation();
    }
    
    return InputDirection.Rotation().GetNormalized();
}


void UNGameplayAbility::SetEffectLevel(const FGameplayTag& ContainerTag, const float Level)
{
    FNGameplayEffectContainerSpec* FoundContainerSpec = EffectContainerSpecMap.Find(ContainerTag);
    if (FoundContainerSpec)
    {
        for (auto& Spec : FoundContainerSpec->TargetGameplayEffectSpecs)
        {
            Spec.Data->SetLevel(Level);
        }
    }   
}


FNGameplayEffectContainerSpec UNGameplayAbility::MakeEffectContainerSpecFromContainer(const FNGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    FNGameplayEffectContainerSpec Spec = FNGameplayEffectContainerSpec(this);
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    ANCharacterBase* AvatarCharacter = Cast<ANCharacterBase>(AvatarActor);
    
    if (Container.TargetType.Get())
    {
        TArray<FHitResult> HitResults;
        TArray<AActor*> TargetActors;
        TArray<FGameplayAbilityTargetDataHandle> TargetData;
        const UNTargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();
        TargetTypeCDO->GetTargets(AvatarCharacter, AvatarActor, EventData, TargetData, HitResults, TargetActors);
        Spec.AddTargets(TargetData, HitResults, TargetActors);
    }
    
    if (OverrideGameplayLevel == INDEX_NONE)
    {
        OverrideGameplayLevel = GetAbilityLevel();
    }

    for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
    {
        Spec.TargetGameplayEffectSpecs.Add(MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel));
    }
    
    return Spec;
}


FNGameplayEffectContainerSpec UNGameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
    FNGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

    if (FoundContainer)
    {
        return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
    }

    return FNGameplayEffectContainerSpec(this);
}


TArray<FActiveGameplayEffectHandle> UNGameplayAbility::ApplyEffectContainerSpec(const FNGameplayEffectContainerSpec& ContainerSpec)
{
    TArray<FActiveGameplayEffectHandle> AllEffects;

    for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
    {
        AllEffects.Append(K2_ApplyGameplayEffectSpecToTarget(SpecHandle, ContainerSpec.TargetData));
    }
    
    return AllEffects;
}


TArray<FActiveGameplayEffectHandle> UNGameplayAbility::ApplyEffectContainerSpecFromTag(const FGameplayTag& Tag)
{
    FNGameplayEffectContainerSpec* FoundSpec = EffectContainerSpecMap.Find(Tag);
    if (FoundSpec)
    {
        return ApplyEffectContainerSpec(*FoundSpec);
    }

    return ApplyEffectContainerSpec(FNGameplayEffectContainerSpec());
}


UObject* UNGameplayAbility::K2_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const
{
    return GetSourceObject(Handle, &ActorInfo);
}


bool UNGameplayAbility::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately)
{
    UNAbilitySystemComponent* ASC = Cast<UNAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
    if (ASC)
    {
        return ASC->BatchRPCTryActivateAbility(InAbilityHandle, EndAbilityImmediately);
    }

    return false;
}


void UNGameplayAbility::ExternalEndAbility()
{
    check(CurrentActorInfo);

    const bool bReplicateEndAbility = true;
    const bool bWasCancelled = false;
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}


FString UNGameplayAbility::GetCurrentPredictionKeyStatus()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}


bool UNGameplayAbility::IsPredictionKeyValidForMorePrediction() const
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}


UAnimMontage* UNGameplayAbility::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
    FAbilityMeshMontage AbilityMeshMontage;
    if (FindAbilityMeshMontage(InMesh, AbilityMeshMontage))
    {
        return AbilityMeshMontage.Montage;
    }

    return nullptr;
}


void UNGameplayAbility::SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* InCurrentMontage)
{
    ensure(IsInstantiated());

    FAbilityMeshMontage AbilityMeshMontage;
    if (FindAbilityMeshMontage(InMesh, AbilityMeshMontage))
    {
        AbilityMeshMontage.Montage = InCurrentMontage;
    }
    else
    {
        CurrentAbilityMeshMontages.Add(FAbilityMeshMontage(InMesh, InCurrentMontage));
    } 
}


bool UNGameplayAbility::FindAbilityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& OutAbilityMeshMontage)
{
    for (FAbilityMeshMontage& MeshMontage : CurrentAbilityMeshMontages)
    {
        if (MeshMontage.Mesh == InMesh)
        {
            OutAbilityMeshMontage = MeshMontage;
            return true;
        }
    }

    return false;
}


void UNGameplayAbility::MontageJumptToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Checked());
    if (ASC->IsAnimatingAbility(this))
    {
        ASC->CurrentMontageJumpToSectionForMesh(InMesh, SectionName);
    }
}


void UNGameplayAbility::MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Checked());
    if (ASC->IsAnimatingAbilityForAnyMesh(this))
    {
        ASC->CurrentMontageSetNextSectionNameForMesh(InMesh, FromSectionName, ToSectionName);
    }
}


void UNGameplayAbility::MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
    if (ASC)
    {
        // We should only stop the current montage if we are the animating ability
        if (ASC->IsAnimatingAbilityForAnyMesh(this))
        {
            ASC->CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
        }
    }
}


void UNGameplayAbility::MontageStopForAllMeshes(float OverrideBlendOutTime)
{
    check(CurrentActorInfo);

    UNAbilitySystemComponent* const ASC = Cast<UNAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
    if (ASC)
    {
        if (ASC->IsAnimatingAbilityForAnyMesh(this))
        {
            ASC->StopAllCurrentMontages(OverrideBlendOutTime);
        }
    }   
}


FGameplayAbilityTargetDataHandle UNGameplayAbility::MakeGameplayAbilityTargetDataHandleFromActorArray(const TArray<AActor*> TargetActors)
{
    if (TargetActors.Num() > 0)
    {
        FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
        NewData->TargetActorArray.Append(TargetActors);
        return FGameplayAbilityTargetDataHandle(NewData);
    }

    return FGameplayAbilityTargetDataHandle();
}


FGameplayAbilityTargetDataHandle UNGameplayAbility::MakeGameplayAbilityTargetDataHandleFromHitResult(const TArray<FHitResult> HitResults)
{
    FGameplayAbilityTargetDataHandle TargetData;

    for (const FHitResult& HitResult : HitResults)
    {
        FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
        TargetData.Add(NewData);
    }

    return TargetData;
}
