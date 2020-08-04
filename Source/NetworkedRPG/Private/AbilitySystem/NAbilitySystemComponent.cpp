// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NAbilitySystemComponent.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarReplayMontageErrorThreshold(TEXT("replay.MontageErrorThreshold"), 0.5f, TEXT("Tolerance level for when montage playback position correction occurs in replays"));


UNAbilitySystemComponent::UNAbilitySystemComponent()
{
}


void UNAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UNAbilitySystemComponent, RepAnimMontageInfoForMeshes);
}


bool UNAbilitySystemComponent::GetShouldTick() const
{
    for (const FGameplayAbilityRepAnimMontageForMesh RepMontageInfo : RepAnimMontageInfoForMeshes)
    {
        const bool bHasReplicatedMontageInfoToUpdate = (IsOwnerActorAuthoritative() && RepMontageInfo.RepMontageInfo.IsStopped == false);
    
        if (bHasReplicatedMontageInfoToUpdate)
        {
            return true;
        }
    }

    return Super::GetShouldTick();
}


void UNAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (IsOwnerActorAuthoritative())
    {
        for (FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
        {
            AnimMontage_UpdateReplicatedDataForMesh(MontageInfo.Mesh);
        }
    }

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UNAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
    Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

    // LocalAnimMontageInfoForMeshes = TArray<FGameplayAbilityLocalAnimMontageForMesh>();
    // RepAnimMontageInfoForMeshes = TArray<FGameplayAbilityRepAnimMontageForMesh>();
    
    if (bPendingMontageRep)
    {
        OnRep_ReplicatedAnimMontageForMesh();
    }
}


void UNAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
    Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

    ClearAnimatingAbilityForAllMeshes(Ability);
}


UNAbilitySystemComponent* UNAbilitySystemComponent::GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent)
{
    return Cast<UNAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}


int32 UNAbilitySystemComponent::K2_GetTagCount(FGameplayTag TagToCheck) const
{
    return GetTagCount(TagToCheck);
}


FGameplayAbilitySpecHandle UNAbilitySystemComponent::FindAbilitySpecHandleForClass(TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
    ABILITYLIST_SCOPE_LOCK();
    for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
    {
        TSubclassOf<UGameplayAbility> SpecAbilityClass = Spec.Ability->GetClass();
        if (SpecAbilityClass == AbilityClass)
        {
            if (!OptionalSourceObject || (OptionalSourceObject && Spec.SourceObject == OptionalSourceObject))
            {
                return Spec.Handle;
            }
        }
    }

    return FGameplayAbilitySpecHandle();
}

void UNAbilitySystemComponent::K2_AddLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count)
{
    AddLooseGameplayTag(GameplayTag, Count);
}

void UNAbilitySystemComponent::K2_AddLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count)
{
    AddLooseGameplayTags(GameplayTags, Count);
}

void UNAbilitySystemComponent::K2_RemoveLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count)
{
    RemoveLooseGameplayTag(GameplayTag, Count);
}

void UNAbilitySystemComponent::K2_RemoveLooseGameplayTags(const FGameplayTagContainer& GameplayTags, int32 Count)
{
    RemoveLooseGameplayTags(GameplayTags, Count);
}

bool UNAbilitySystemComponent::BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately)
{
    bool AbilityActivated = false;
    if (InAbilityHandle.IsValid())
    {
        FScopedServerAbilityRPCBatcher NAbilityRPCBatcher(this, InAbilityHandle);
        AbilityActivated = TryActivateAbility(InAbilityHandle, true);

        if (EndAbilityImmediately)
        {
            FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(InAbilityHandle);
            if (AbilitySpec)
            {
                UNGameplayAbility* NAbility = Cast<UNGameplayAbility>(AbilitySpec->GetPrimaryInstance());
                NAbility->ExternalEndAbility();
            }          
        }
    }
    return AbilityActivated;
}

void UNAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters GameplayCueParameters) const
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UNAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters GameplayCueParameters) const
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
}

void UNAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters GameplayCueParameters) const
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

UAnimInstance* UNAbilitySystemComponent::GetMeshAnimInstance(USkeletalMeshComponent* InMesh) const
{
    return IsValid(InMesh) && InMesh->GetOwner() == AbilityActorInfo->AvatarActor ? InMesh->GetAnimInstance() : nullptr;
}

float UNAbilitySystemComponent::PlayMontageForMesh(UGameplayAbility* InAnimatingAbility, USkeletalMeshComponent* InMesh, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* NewAnimMontage, float InPlayRate, FName StartSectionName, bool bReplicateMontage)
{
    UNGameplayAbility* InAbility = Cast<UNGameplayAbility>(InAnimatingAbility);

    float Duration = -1.0f;

    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    if (AnimInstance && NewAnimMontage)
    {
        Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
        if (Duration > 0.0f)
        {
            FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

            if (AnimMontageInfo.LocalMontageInfo.AnimatingAbility && AnimMontageInfo.LocalMontageInfo.AnimatingAbility != InAnimatingAbility)
            {
                // The ability that was previously animating will have already gotten the 'interrupted' callback.
                // It may be a good idea to make this a global policy and 'cancel' the ability.
                // 
                // For now, we expect it to end itself when this happens.
            }


            if (NewAnimMontage->HasRootMotion() && AnimInstance->GetOwningActor())
            {
                UE_LOG(LogRootMotion, Log, TEXT("UAbilitySystemComponent::PlayMontage %s, Role %s"),
                    *GetNameSafe(NewAnimMontage), *UEnum::GetValueAsString(TEXT("Engine.ENetRole"),
                    AnimInstance->GetOwningActor()->GetLocalRole()));
            }

            AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
            AnimMontageInfo.LocalMontageInfo.AnimatingAbility = InAnimatingAbility;
            AnimMontageInfo.LocalMontageInfo.PlayBit = !AnimMontageInfo.LocalMontageInfo.PlayBit;

            // Start at given section
            if (InAbility)
            {
                InAbility->SetCurrentMontageForMesh(InMesh, NewAnimMontage);
            }

            // Replicate to non owners
            if (IsOwnerActorAuthoritative())
            {
                if (bReplicateMontage)
                {
                    // Those are static parameters, they are only set when the montage is played. They are not changed after that.
                    FGameplayAbilityRepAnimMontageForMesh& AbilityRepMontageInfo = GetGameplayAbilityRepAnimMontageInfoForMesh(InMesh);
                    AbilityRepMontageInfo.RepMontageInfo.AnimMontage = NewAnimMontage;
                    AbilityRepMontageInfo.RepMontageInfo.ForcePlayBit = !bool(AbilityRepMontageInfo.RepMontageInfo.ForcePlayBit);

                    // Update parameters that change during montage life time.
                    AnimMontage_UpdateReplicatedDataForMesh(InMesh);

                    // Force net update on out avatar actor
                    if (AbilityActorInfo->AvatarActor != nullptr)
                    {
                        AbilityActorInfo->AvatarActor->ForceNetUpdate();
                    }
                    
                }
            }
            else
            {
                // If this prediction is rejected, we need to end the preview
                FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
                if (PredictionKey.IsValidKey())
                {
                    PredictionKey.NewRejectedDelegate().BindUObject(this, &UNAbilitySystemComponent::OnPredictiveMontageRejectedForMesh, InMesh, NewAnimMontage);
                }
            }   
        }
    }
    
    return Duration;
}

float UNAbilitySystemComponent::PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* NewAnimMontage,
    float InPlayRate, FName StartSectionName)
{
    float Duration = -1.0f;
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    if (AnimInstance && NewAnimMontage)
    {
        Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
        if (Duration > 0.f)
        {
            FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
            AnimMontageInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
        }
    }

    return Duration;
}

void UNAbilitySystemComponent::CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
    UAnimMontage* MontageToStop = AnimMontageInfo.LocalMontageInfo.AnimMontage;
    bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

    if (bShouldStopMontage)
    {
        const float BlendOutTime = (OverrideBlendOutTime >= 0.0f ? OverrideBlendOutTime : MontageToStop->BlendOut.GetBlendTime());

        AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);

        if (IsOwnerActorAuthoritative())
        {
            AnimMontage_UpdateReplicatedDataForMesh(InMesh);
        }
    }
}

void UNAbilitySystemComponent::StopAllCurrentMontages(float OverrideBlendOutTime)
{
    for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
    {
        CurrentMontageStopForMesh(GameplayAbilityLocalAnimMontageForMesh.Mesh, OverrideBlendOutTime);
    }
}

void UNAbilitySystemComponent::StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh, const UAnimMontage& Montage,
    float OverrideBlendOutTime)
{
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
    if (&Montage == AnimMontageInfo.LocalMontageInfo.AnimMontage)
    {
        CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
    }
}

void UNAbilitySystemComponent::ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability)
{
    UNGameplayAbility* NAbility = Cast<UNGameplayAbility>(Ability);
    for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
    {
        if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility == Ability)
        {
            NAbility->SetCurrentMontageForMesh(GameplayAbilityLocalAnimMontageForMesh.Mesh, nullptr);
            GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility = nullptr;
        }
    }
}

void UNAbilitySystemComponent::CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
    if ((SectionName != NAME_None) && AnimInstance && AnimMontageInfo.LocalMontageInfo.AnimMontage)
    {
        if (IsOwnerActorAuthoritative())
        {
            AnimMontage_UpdateReplicatedDataForMesh(InMesh);
        }
        else
        {
            ServerCurrentMontageJumpToSectionNameForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage, SectionName);
        }   
    }
}

void UNAbilitySystemComponent::CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh,
    FName FromSectionName, FName ToSectionName)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
    if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance)
    {
        // set next section name
        AnimInstance->Montage_SetNextSection(FromSectionName, ToSectionName, AnimMontageInfo.LocalMontageInfo.AnimMontage);

        // Update replicated version for simulated proxies if we are on the server.
        if (IsOwnerActorAuthoritative())
        {
            AnimMontage_UpdateReplicatedDataForMesh(InMesh);
        }
        else
        {
            float CurrentPosition = AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage);
            ServerCurrentMontageSetNextSectionNameForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage, CurrentPosition, FromSectionName, ToSectionName);
        }
        
    }
}

void UNAbilitySystemComponent::CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
    if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance)
    {
        // Set play rate
        AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);

        // Update replicated version for simulated proxies if we are on server
        if (IsOwnerActorAuthoritative())
        {
            AnimMontage_UpdateReplicatedDataForMesh(InMesh);
        }
        else
        {
            ServerCurrentMontageSetPlayRateForMesh(InMesh, AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);
        }
    }
}

bool UNAbilitySystemComponent::IsAnimatingAbilityForAnyMesh(UGameplayAbility* InAbility) const
{
    for (const FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
    {
        if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility == InAbility)
        {
            return true;
        }
    }
    return false;
}

UGameplayAbility* UNAbilitySystemComponent::GetAnimatingAbilityFromAnyMesh()
{
    // Only one ability can be animating for all meshes
    for (FGameplayAbilityLocalAnimMontageForMesh& GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
    {
        if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility)
        {
            return GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimatingAbility;
        }
    }

    return nullptr;
}

TArray<UAnimMontage*> UNAbilitySystemComponent::GetCurrentMontages() const
{
    TArray<UAnimMontage*> Montages;

    for (FGameplayAbilityLocalAnimMontageForMesh GameplayAbilityLocalAnimMontageForMesh : LocalAnimMontageInfoForMeshes)
    {
        UAnimInstance* AnimInstance = GetMeshAnimInstance(GameplayAbilityLocalAnimMontageForMesh.Mesh);

        if (GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage && AnimInstance
            && AnimInstance->Montage_IsActive(GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage))
        {
            Montages.Add(GameplayAbilityLocalAnimMontageForMesh.LocalMontageInfo.AnimMontage);
        }
    }

    return Montages;
}

UAnimMontage* UNAbilitySystemComponent::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

    if (AnimMontageInfo.LocalMontageInfo.AnimMontage && AnimInstance
        && AnimInstance->Montage_IsActive(AnimMontageInfo.LocalMontageInfo.AnimMontage))
    {
        return AnimMontageInfo.LocalMontageInfo.AnimMontage;
    }

    return nullptr;
}

int32 UNAbilitySystemComponent::GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

    if (CurrentAnimMontage && AnimInstance)
    {
        const float MontagePosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
        return CurrentAnimMontage->GetSectionIndexFromPosition(MontagePosition);
    }

    return INDEX_NONE;
}

FName UNAbilitySystemComponent::GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

    if (CurrentAnimMontage && AnimInstance)
    {
        float MontagePosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
        int32 CurrentSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(MontagePosition);

        return CurrentAnimMontage->GetSectionName(CurrentSectionID);
    }

    return NAME_None;
}

float UNAbilitySystemComponent::GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

    if (CurrentAnimMontage && AnimInstance)
    {
        int32 CurrentSectionID = GetCurrentMontageSectionIDForMesh(InMesh);
        if (CurrentSectionID != INDEX_NONE)
        {
            TArray<FCompositeSection>& CompositeSections = CurrentAnimMontage->CompositeSections;

            // If we have another section after us, then take delta between both start times.
            if (CurrentSectionID < (CompositeSections.Num() - 1))
            {
                return (CompositeSections[CurrentSectionID + 1].GetTime() - CompositeSections[CurrentSectionID].GetTime());
            }
            // otherwise we are the last section, so take delta with montage total time.
            else
            {
                return (CurrentAnimMontage->SequenceLength - CompositeSections[CurrentSectionID].GetTime());
            }
        }

        // if we have no sections, just return total length of Montage
        return CurrentAnimMontage->SequenceLength;
    }

    return 0.f;
}

float UNAbilitySystemComponent::GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    UAnimMontage* CurrentAnimMontage = GetCurrentMontageForMesh(InMesh);

    if (CurrentAnimMontage && AnimInstance && AnimInstance->Montage_IsActive(CurrentAnimMontage))
    {
        const float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
        return CurrentAnimMontage->GetSectionTimeLeftFromPos(CurrentPosition);
    }

    return -1.f;
}

FGameplayAbilityLocalAnimMontageForMesh& UNAbilitySystemComponent::GetLocalAnimMontageInfoForMesh(
    USkeletalMeshComponent* InMesh)
{
    for (FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
    {
        if (MontageInfo.Mesh == InMesh)
        {
            return MontageInfo;
        }
    }

    FGameplayAbilityLocalAnimMontageForMesh MontageInfo = FGameplayAbilityLocalAnimMontageForMesh(InMesh);
    LocalAnimMontageInfoForMeshes.Add(MontageInfo);
    return LocalAnimMontageInfoForMeshes.Last();
}

FGameplayAbilityRepAnimMontageForMesh& UNAbilitySystemComponent::GetGameplayAbilityRepAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh)
{
    for (FGameplayAbilityRepAnimMontageForMesh& RepMontageInfo : RepAnimMontageInfoForMeshes)
    {
        if (RepMontageInfo.Mesh == InMesh)
        {
            return RepMontageInfo;
        }
    }

    FGameplayAbilityRepAnimMontageForMesh RepMontageInfo = FGameplayAbilityRepAnimMontageForMesh(InMesh);
    RepAnimMontageInfoForMeshes.Add(RepMontageInfo);
    return RepAnimMontageInfoForMeshes.Last();
}

void UNAbilitySystemComponent::OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh,
    UAnimMontage* PredictiveMontage)
{
    static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    if (AnimInstance && PredictiveMontage)
    {
        // If this montage is still playing: kill it
        if (AnimInstance->Montage_IsPlaying(PredictiveMontage))
        {
            AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, PredictiveMontage);
        }
    }
}

void UNAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh)
{
    check(IsOwnerActorAuthoritative());

    AnimMontage_UpdateReplicatedDataForMesh(GetGameplayAbilityRepAnimMontageInfoForMesh(InMesh));
}

void UNAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(
    FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(OutRepAnimMontageInfo.Mesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(OutRepAnimMontageInfo.Mesh);

    if (AnimInstance && AnimMontageInfo.LocalMontageInfo.AnimMontage)
    {
        OutRepAnimMontageInfo.RepMontageInfo.AnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;

        // Compress Flags
        const bool bIsStopped = AnimInstance->Montage_GetIsStopped(AnimMontageInfo.LocalMontageInfo.AnimMontage);

        if (!bIsStopped)
        {
            OutRepAnimMontageInfo.RepMontageInfo.PlayRate = AnimInstance->Montage_GetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage);
            OutRepAnimMontageInfo.RepMontageInfo.Position = AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage);
            OutRepAnimMontageInfo.RepMontageInfo.BlendTime = AnimInstance->Montage_GetBlendTime(AnimMontageInfo.LocalMontageInfo.AnimMontage);
        }

        if (OutRepAnimMontageInfo.RepMontageInfo.IsStopped != bIsStopped)
        {
            // Set this prior to calling UpdateShouldTick, so we start ticking if we are playing a Montage
            OutRepAnimMontageInfo.RepMontageInfo.IsStopped = bIsStopped;

            // When we start or stop an animation, update the clients right away for the Avatar Actor
            if (AbilityActorInfo->AvatarActor != nullptr)
            {
                AbilityActorInfo->AvatarActor->ForceNetUpdate();
            }

            // When this changes, we should update whether or not we should be ticking
            UpdateShouldTick();
        }

        // Replicate NextSectionID to keep it in sync.
        // We actually replicate NextSectionID+1 on a BYTE to put INDEX_NONE in there.
        const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(OutRepAnimMontageInfo.RepMontageInfo.Position);
        if (CurrentSectionID != INDEX_NONE)
        {
            const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(AnimMontageInfo.LocalMontageInfo.AnimMontage, CurrentSectionID);
            if (NextSectionID >= (256 - 1))
            {
                ABILITY_LOG(Error, TEXT("AnimMontage_UpdateReplicatedData. NextSectionID = %d. RepAnimMontageInfo.Position: %.2f, CurrentSectionID: %d. LocalAnimMontageInfo.AnimMontage %s"),
                    NextSectionID, OutRepAnimMontageInfo.RepMontageInfo.Position, CurrentSectionID, *GetNameSafe(AnimMontageInfo.LocalMontageInfo.AnimMontage));
                ensure(NextSectionID < (256 - 1));
            }

            OutRepAnimMontageInfo.RepMontageInfo.NextSectionID = uint8(NextSectionID + 1);
        }
        else
        {
            OutRepAnimMontageInfo.RepMontageInfo.NextSectionID = 0;
        }
    }
}

void UNAbilitySystemComponent::AnimMontage_UpdateForcedPlayFlagsForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo)
{
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(OutRepAnimMontageInfo.Mesh);

    OutRepAnimMontageInfo.RepMontageInfo.ForcePlayBit = AnimMontageInfo.LocalMontageInfo.PlayBit;
}

void UNAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh()
{
    for (FGameplayAbilityRepAnimMontageForMesh& NewRepMontageInfoForMesh : RepAnimMontageInfoForMeshes)
    {
        FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(NewRepMontageInfoForMesh.Mesh);

        UWorld* World = GetWorld();

        if (NewRepMontageInfoForMesh.RepMontageInfo.bSkipPlayRate)
        {
            NewRepMontageInfoForMesh.RepMontageInfo.PlayRate = 1.f;
        }

        const bool bIsPlayingReplay = World && World->IsPlayingReplay();

        const float MONTAGE_REP_POS_ERR_THRESH = bIsPlayingReplay ? CVarReplayMontageErrorThreshold.GetValueOnGameThread() : 0.1f;

        UAnimInstance* AnimInstance = GetMeshAnimInstance(NewRepMontageInfoForMesh.Mesh);
        if (!AnimInstance || !IsReadyForReplicatedMontageForMesh())
        {
            bPendingMontageRep = true;
            return;
        }
        bPendingMontageRep = false;

        if (!AbilityActorInfo->IsLocallyControlled())
        {
            static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.Montage.Debug"));
            bool DebugMontage = (CVar && CVar->GetValueOnGameThread() == 1);
            if (DebugMontage)
            {
                ABILITY_LOG(Warning, TEXT("\n\nOnRep_ReplicatedAnimMontage, %s"), *GetNameSafe(this));
                ABILITY_LOG(Warning, TEXT("\tAnimMontage: %s\n\tPlayRate: %f\n\tPosition: %f\n\tBlendTime: %f\n\tNextSectionID: %d\n\tIsStopped: %d\n\tForcePlayBit: %d"),
                    *GetNameSafe(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage),
                    NewRepMontageInfoForMesh.RepMontageInfo.PlayRate,
                    NewRepMontageInfoForMesh.RepMontageInfo.Position,
                    NewRepMontageInfoForMesh.RepMontageInfo.BlendTime,
                    NewRepMontageInfoForMesh.RepMontageInfo.NextSectionID,
                    NewRepMontageInfoForMesh.RepMontageInfo.IsStopped,
                    NewRepMontageInfoForMesh.RepMontageInfo.ForcePlayBit);
                ABILITY_LOG(Warning, TEXT("\tLocalAnimMontageInfo.AnimMontage: %s\n\tPosition: %f"),
                    *GetNameSafe(AnimMontageInfo.LocalMontageInfo.AnimMontage), AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage));
            }

            if (NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage)
            {
                // New Montage to play
                const bool ReplicatedPlayBit = bool(NewRepMontageInfoForMesh.RepMontageInfo.ForcePlayBit);
                if ((AnimMontageInfo.LocalMontageInfo.AnimMontage != NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage) || (AnimMontageInfo.LocalMontageInfo.PlayBit != ReplicatedPlayBit))
                {
                    AnimMontageInfo.LocalMontageInfo.PlayBit = ReplicatedPlayBit;
                    PlayMontageSimulatedForMesh(NewRepMontageInfoForMesh.Mesh, NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage, NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
                }

                if (AnimMontageInfo.LocalMontageInfo.AnimMontage == nullptr)
                {
                    ABILITY_LOG(Warning, TEXT("OnRep_ReplicatedAnimMontage: PlayMontageSimulated failed. Name: %s, AnimMontage: %s"), *GetNameSafe(this), *GetNameSafe(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage));
                    return;
                }

                // Play rate has changed
                if (AnimInstance->Montage_GetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage) != NewRepMontageInfoForMesh.RepMontageInfo.PlayRate)
                {
                    AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, NewRepMontageInfoForMesh.RepMontageInfo.PlayRate);
                }

                // Compressed flags
                const bool bIsStopped = AnimInstance->Montage_GetIsStopped(AnimMontageInfo.LocalMontageInfo.AnimMontage);
                const bool bReplicatedIsStopped = bool(NewRepMontageInfoForMesh.RepMontageInfo.IsStopped);

                // Process stopping first, so we don't change sections and cause blending to pop
                if (bReplicatedIsStopped)
                {
                    if (!bIsStopped)
                    {
                        CurrentMontageStopForMesh(NewRepMontageInfoForMesh.Mesh, NewRepMontageInfoForMesh.RepMontageInfo.BlendTime);
                    }
                }
                else if (!NewRepMontageInfoForMesh.RepMontageInfo.SkipPositionCorrection)
                {
                    const int32 RepSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(NewRepMontageInfoForMesh.RepMontageInfo.Position);
                    const int32 RepNextSectionID = int32(NewRepMontageInfoForMesh.RepMontageInfo.NextSectionID) - 1;

                    // And NextSectionID for the replicated SectionID
                    if (RepSectionID != INDEX_NONE)
                    {
                        const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(AnimMontageInfo.LocalMontageInfo.AnimMontage, RepSectionID);

                        // If NextSectionID is different than replicated one, then set it.
                        if (NextSectionID != RepNextSectionID)
                        {
                            AnimInstance->Montage_SetNextSection(AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepSectionID), AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepNextSectionID), AnimMontageInfo.LocalMontageInfo.AnimMontage); 
                        }

                        // Make sure we haven't received that update too late and the client hasn't already jumped to another section.
                        const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage));
                        if ((CurrentSectionID != RepSectionID) && (CurrentSectionID != RepNextSectionID))
                        {
                            // Client is in a wrong section, telaport him into the beginning of the right section
                            const float SectionStartTime = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetAnimCompositeSection(RepSectionID).GetTime();
                            AnimInstance->Montage_SetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage, SectionStartTime);
                        }
                    }

                    // Update Position. If error is too great, jump to replicated position.
                    const float CurrentPosition = AnimInstance->Montage_GetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage);
                    const int32 CurrentSectionID = AnimMontageInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(CurrentPosition);
                    const float DeltaPosition = NewRepMontageInfoForMesh.RepMontageInfo.Position - CurrentPosition;

                    // Only check threshold if we are located in the same section. Different sections require a bit more work as we could be jumping around the timeline.
                    // And therefor DeltaPosition is not as trivial to determine.
                    if ((CurrentSectionID == RepSectionID) && (FMath::Abs(DeltaPosition) > MONTAGE_REP_POS_ERR_THRESH) && (NewRepMontageInfoForMesh.RepMontageInfo.PlayRate == 0))
                    {
                        // fast forward to server position and trigger notifies
                        if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(NewRepMontageInfoForMesh.RepMontageInfo.AnimMontage))
                        {
                            const float DeltaTime = !FMath::IsNearlyZero(NewRepMontageInfoForMesh.RepMontageInfo.PlayRate) ? (DeltaPosition / NewRepMontageInfoForMesh.RepMontageInfo.PlayRate) : 0.f;
                            if (DeltaTime >= 0.f)
                            {
                                MontageInstance->UpdateWeight(DeltaTime);
                                MontageInstance->HandleEvents(CurrentPosition, NewRepMontageInfoForMesh.RepMontageInfo.Position, nullptr);
                                AnimInstance->TriggerAnimNotifies(DeltaTime);
                            }
                        }
                        AnimInstance->Montage_SetPosition(AnimMontageInfo.LocalMontageInfo.AnimMontage, NewRepMontageInfoForMesh.RepMontageInfo.Position);
                    }
                }
            }
        }
    }
}

bool UNAbilitySystemComponent::IsReadyForReplicatedMontageForMesh()
{
    // Children may want to override this for additional checks
    return true;
}

void UNAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Implementation(
    USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName,FName NextSectionName)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    
    if (AnimInstance)
    {
        FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);
        UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
        if (ClientAnimMontage == CurrentAnimMontage)
        {

            // Set NextSectionName
            AnimInstance->Montage_SetNextSection(SectionName, NextSectionName, CurrentAnimMontage);

            // Correct position if we are in an invalid section
            const float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentAnimMontage);
            const int32 CurrentSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(CurrentPosition);
            const FName CurrentSectionName = CurrentAnimMontage->GetSectionName(CurrentSectionID);

            const int32 ClientSectionID = CurrentAnimMontage->GetSectionIndexFromPosition(ClientPosition);
            const FName ClientCurrentSectionName = CurrentAnimMontage->GetSectionName(ClientSectionID);
            if ((CurrentSectionName != ClientCurrentSectionName) || (CurrentSectionName != SectionName))
            {
                // We are in an invalid section, jump to client's position
                AnimInstance->Montage_SetPosition(CurrentAnimMontage, ClientPosition);
            }

            // Update replicated version for Simulated Proxies if we are on the server.
            if (IsOwnerActorAuthoritative())
            {
                AnimMontage_UpdateReplicatedDataForMesh(InMesh);
            }
        }
    }
}

bool UNAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh,
    UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName)
{
    return true;
}

void UNAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Implementation(
    USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

    if (AnimInstance)
    {
        UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
        if (ClientAnimMontage == CurrentAnimMontage)
        {
            // Set NextSectionName
            AnimInstance->Montage_JumpToSection(SectionName, CurrentAnimMontage);

            // Update replicated version for Simulated Proxies if we are on the server.
            if (IsOwnerActorAuthoritative())
            {
                AnimMontage_UpdateReplicatedDataForMesh(InMesh);
            }
        }
    }
}

bool UNAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Validate(USkeletalMeshComponent* InMesh,
    UAnimMontage* ClientAnimMontage, FName SectionName)
{
    return true;
}

void UNAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Implementation(USkeletalMeshComponent* InMesh,
    UAnimMontage* ClientAnimMontage, float InPlayRate)
{
    UAnimInstance* AnimInstance = GetMeshAnimInstance(InMesh);
    FGameplayAbilityLocalAnimMontageForMesh& AnimMontageInfo = GetLocalAnimMontageInfoForMesh(InMesh);

    if (AnimInstance)
    {
        UAnimMontage* CurrentAnimMontage = AnimMontageInfo.LocalMontageInfo.AnimMontage;
        if (ClientAnimMontage == CurrentAnimMontage)
        {
            // Set PlayRate
            AnimInstance->Montage_SetPlayRate(AnimMontageInfo.LocalMontageInfo.AnimMontage, InPlayRate);

            // Update replicated version for Simulated Proxies if we are on the server.
            if (IsOwnerActorAuthoritative())
            {
                AnimMontage_UpdateReplicatedDataForMesh(InMesh);
            }
        }
    }
}

bool UNAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Validate(USkeletalMeshComponent* InMesh,
    UAnimMontage* ClientAnimMontage, float InPlayRate)
{
    return true;
}




