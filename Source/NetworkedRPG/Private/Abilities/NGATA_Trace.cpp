// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/NGATA_Trace.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

ANGATA_Trace::ANGATA_Trace()
{
    bDestroyOnConfirmation = false;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
    MaxHitResultsPerTrace = 1;
    NumberOfTraces = 1;
    bIgnoreBlockingHits = false;
    bTraceAffectsAimPitch = true;
    bTraceFromPlayerViewPoint = false;
    MaxRange = 999999.0f;
    bUseAimingSpreadMod = false;
    BaseSpread = 0.0f;
    AimingSpreadMod = 0.0f;
    TargetingSpreadIncrement = 0.0f;
    TargetingSpreadMax = 0.0f;
    CurrentTargetingSpread = 0.0f;
    bUsePersistentHitResults = false;
}

void ANGATA_Trace::ResetSpread()
{
    bUseAimingSpreadMod = false;
    BaseSpread = 0.0f;
    AimingSpreadMod = 0.0f;
    TargetingSpreadIncrement = 0.0f;
    TargetingSpreadMax = 0.0f;
    CurrentTargetingSpread = 0.0f;
}

float ANGATA_Trace::GetCurrentSpread() const
{
    float FinalSpread = BaseSpread + CurrentTargetingSpread;

    if (bUseAimingSpreadMod && AimingTag.IsValid() && AimingRemovalTag.IsValid())
    {
        UAbilitySystemComponent* ASC = OwningAbility->GetCurrentActorInfo()->AbilitySystemComponent.Get();
        if (ASC && (ASC->GetTagCount(AimingTag) > ASC->GetTagCount(AimingRemovalTag)))
        {
            FinalSpread *= AimingSpreadMod;
        }
    }

    return FinalSpread;
}

void ANGATA_Trace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
    StartLocation = InStartLocation;
}

void ANGATA_Trace::SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer)
{
    ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
}

void ANGATA_Trace::SetDestroyOnConfirmation(bool bInDestroyOnConfirmation)
{
    bDestroyOnConfirmation = bInDestroyOnConfirmation;
}

void ANGATA_Trace::StartTargeting(UGameplayAbility* Ability)
{
    SetActorTickEnabled(true);
    
    OwningAbility = Ability;
    SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

    DestroyReticleActors();

    if (ReticleClass)
    {
        for (int32 i = 0; i < MaxHitResultsPerTrace * NumberOfTraces; i++)
        {
            SpawnReticleActor(GetActorLocation(), GetActorRotation());
        }
    }

    if (bUsePersistentHitResults)
    {
        PersistentHitResults.Empty();
    }
}

void ANGATA_Trace::ConfirmTargetingAndContinue()
{
    check(ShouldProduceTargetData())

    if (SourceActor)
    {
        TArray<FHitResult> HitResults = PerformTrace(SourceActor);
        FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
        TargetDataReadyDelegate.Broadcast(Handle);
    }

    if (bUsePersistentHitResults)
    {
        PersistentHitResults.Empty();
    }

}

void ANGATA_Trace::CancelTargeting()
{
    const FGameplayAbilityActorInfo* ActorInfo = OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr;
    UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
    if (ASC)
    {
        ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, OwningAbility->GetCurrentAbilitySpecHandle(), OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey()).Remove(GenericCancelHandle);
    }
    else
    {
        ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor::CancelTargeting called with null ASC! Actor %s"), *GetName());
    }

    CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());

    SetActorTickEnabled(false);

    if (bUsePersistentHitResults)
    {
        PersistentHitResults.Empty();
    }
}

void ANGATA_Trace::BeginPlay()
{
    Super::BeginPlay();
    
    SetActorTickEnabled(false);
}

void ANGATA_Trace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyReticleActors();

    Super::EndPlay(EndPlayReason);
}

void ANGATA_Trace::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    TArray<FHitResult> HitResults;
    if (bDebug || bUsePersistentHitResults)
    {
        HitResults = PerformTrace(SourceActor);
    }
}

void ANGATA_Trace::LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World,
    const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName,
    const FCollisionQueryParams Params)
{
    check(World);

    TArray<FHitResult> HitResults;
    World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

    TArray<FHitResult> FilteredHitResults;

    FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

    for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++ HitIdx)
    {
        FHitResult& Hit = HitResults[HitIdx];

        if (!Hit.Actor.IsValid() || FilterHandle.FilterPassesForActor(Hit.Actor))
        {
            Hit.TraceStart = TraceStart;
            Hit.TraceEnd = End;

            FilteredHitResults.Add(Hit);
        }
    }

    OutHitResults = FilteredHitResults;
}

void ANGATA_Trace::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params,
    const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch)
{
    if (!OwningAbility) // Server and launching client only
    {
        return;
    }

    FVector ViewStart = TraceStart;
    FRotator ViewRot = StartLocation.GetTargetingTransform().GetRotation().Rotator();

    if (MasterPC)
    {
        MasterPC->GetPlayerViewPoint(ViewStart, ViewRot);
    }

    const FVector ViewDir = ViewRot.Vector();
    FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

    ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

    TArray<FHitResult> HitResults;
    LineTraceWithFilter(HitResults, InSourceActor->GetWorld(), Filter, ViewStart, ViewEnd, TraceProfile.Name, Params);

    CurrentTargetingSpread = FMath::Min(TargetingSpreadMax, CurrentTargetingSpread + TargetingSpreadIncrement);

    const bool bUseTraceResult = HitResults.Num() > 0 && (FVector::DistSquared(TraceStart, HitResults[0].Location) <= (MaxRange * MaxRange));

    const FVector AdjustedEnd = (bUseTraceResult) ? HitResults[0].Location : ViewEnd;

    FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
    if (AdjustedAimDir.IsZero())
    {
        AdjustedAimDir = ViewDir;
    }

    if (!bTraceAffectsAimPitch && bUseTraceResult)
    {
        FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

        if (!OriginalAimDir.IsZero())
        {
            const FRotator OriginalAimRot = OriginalAimDir.Rotation();

            FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
            AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

            AdjustedAimDir = AdjustedAimRot.Vector();
        }
    }

    const float CurrentSpread = GetCurrentSpread();

    const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);
    const int32 RandomSeed = FMath::Rand();
    FRandomStream WeaponRandomStream(RandomSeed);
    const FVector ShootDir = WeaponRandomStream.VRandCone(AdjustedAimDir, ConeHalfAngle, ConeHalfAngle);

    OutTraceEnd = TraceStart + (ShootDir * MaxRange);
}

bool ANGATA_Trace::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter,
    float AbilityRange, FVector& ClippedPosition)
{
    FVector CameraToCenter = AbilityCenter - CameraLocation;
    float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
    if (DotToCenter >= 0)
    {
        float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
        float RadiusSquared = (AbilityRange * AbilityRange);
        if (DistanceSquared <= RadiusSquared)
        {
            float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
            float DistanceAlongRay = DotToCenter + DistanceFromCamera;
            ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);
            return true;
        }
    }

    return false;
}

void ANGATA_Trace::StopTargeting()
{
    SetActorTickEnabled(false);

    DestroyReticleActors();

    // Clear added callbacks
    TargetDataReadyDelegate.Clear();
    CanceledDelegate.Clear();

    if (GenericDelegateBoundASC)
    {
        GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor::ConfirmTargeting);
        GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor::CancelTargeting);
        GenericDelegateBoundASC = nullptr;
    }
}

FGameplayAbilityTargetDataHandle ANGATA_Trace::MakeTargetData(const TArray<FHitResult>& HitResults) const
{
    FGameplayAbilityTargetDataHandle ReturnDataHandle;

    for (int32 i = 0; i < HitResults.Num(); i++)
    {
        // Note: These are cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr)
        FGameplayAbilityTargetData_SingleTargetHit* ReturnData = new FGameplayAbilityTargetData_SingleTargetHit();
        ReturnData->HitResult = HitResults[i];
        ReturnDataHandle.Add(ReturnData);
    }

    return ReturnDataHandle;
}

TArray<FHitResult> ANGATA_Trace::PerformTrace(AActor* InSourceActor)
{
    bool bTraceComplex = false;
    TArray<AActor*> ActorsToIgnore;

    ActorsToIgnore.Add(InSourceActor);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(ANGATA_LineTrace), bTraceComplex);
    Params.bReturnPhysicalMaterial = true;
    Params.AddIgnoredActors(ActorsToIgnore);
    Params.bIgnoreBlocks = bIgnoreBlockingHits;

    FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
    FVector TraceEnd;

    if (MasterPC)
    {
        FVector ViewStart;
        FRotator ViewRot;
        MasterPC->GetPlayerViewPoint(ViewStart, ViewRot);

        TraceStart = bTraceFromPlayerViewPoint ? ViewStart : TraceStart;
    }

    if (bUsePersistentHitResults)
    {
        // Clear any blocking hit results, invalid actors, or actors out of range
        for (int32 i = PersistentHitResults.Num() - 1; i >= 0; i--)
        {
            FHitResult& HitResult = PersistentHitResults[i];

            if (HitResult.bBlockingHit || !HitResult.Actor.IsValid() || FVector::DistSquared(TraceStart, HitResult.Actor.Get()->GetActorLocation()) > (MaxRange * MaxRange))
            {
                PersistentHitResults.RemoveAt(i);
            }
        }
    }

    TArray<FHitResult> ReturnHitResults;

    for (int32 TraceIndex = 0; TraceIndex < NumberOfTraces; TraceIndex ++)
    {
        AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);    //Effective on server and launching client only

        SetActorLocationAndRotation(TraceEnd, SourceActor->GetActorRotation());

        CurrentTraceEnd = TraceEnd;

        TArray<FHitResult> TraceHitResults;
        DoTrace(TraceHitResults, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);

        for (int32 j = TraceHitResults.Num() - 1; j >= 0; j--)
        {
            if (MaxHitResultsPerTrace >= 0 && j + 1 > MaxHitResultsPerTrace)
            {
                TraceHitResults.RemoveAt(j);
                continue;
            }

            FHitResult& HitResult = TraceHitResults[j];

            if (bUsePersistentHitResults)
            {
                if (HitResult.Actor.IsValid() && (!HitResult.bBlockingHit || PersistentHitResults.Num() < 1))
                {
                    bool bActorAlreadyInPersistentHits = false;

                    for (int32 k = 0; k < PersistentHitResults.Num(); k++)
                    {
                        FHitResult& PersistentHitResult = PersistentHitResults[k];

                        if (PersistentHitResult.Actor.Get() == HitResult.Actor.Get())
                        {
                            bActorAlreadyInPersistentHits = true;
                            break;
                        }
                    }

                    if (bActorAlreadyInPersistentHits)
                    {
                        continue;
                    }

                    if (PersistentHitResults.Num() >= MaxHitResultsPerTrace)
                    {
                        PersistentHitResults.RemoveAt(0);
                    }

                    PersistentHitResults.Add(HitResult);
                }
            }
            else
            {
                int32 ReticleIndex = TraceIndex * MaxHitResultsPerTrace + j;
                if (ReticleIndex < ReticleActors.Num())
                {
                    if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[ReticleIndex].Get())
                    {
                        const bool bHitActor = HitResult.Actor != nullptr;

                        if (bHitActor && !HitResult.bBlockingHit)
                        {
                            LocalReticleActor->SetActorHiddenInGame(false);

                            const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.Actor->GetActorLocation() : HitResult.Location;

                            LocalReticleActor->SetActorLocation(ReticleLocation);
                            LocalReticleActor->SetIsTargetAnActor(bHitActor);
                        }
                        else
                        {
                            LocalReticleActor->SetActorHiddenInGame(true);
                        }
                    }
                }
            }
        }

        if (!bUsePersistentHitResults)
        {
            for (int32 j = TraceHitResults.Num(); j < ReticleActors.Num(); j++)
            {
                if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[j].Get())
                {
                    LocalReticleActor->SetIsTargetAnActor(false);
                    LocalReticleActor->SetActorHiddenInGame(true);
                }
            }
        }

        if (TraceHitResults.Num() < 1)
        {
            // If there were no hits, add a default HitResult at the end of the trace
            FHitResult HitResult;
            // Start param could be player ViewPoint. We want hit result to always display the StartLocation.
            HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();
            HitResult.TraceEnd = TraceEnd;
            HitResult.Location = TraceEnd;
            HitResult.ImpactPoint = TraceEnd;
            TraceHitResults.Add(HitResult);

            if (bUsePersistentHitResults && PersistentHitResults.Num() < 1)
            {
                PersistentHitResults.Add(HitResult);
            }
        }

        ReturnHitResults.Append(TraceHitResults);
    } // for number of traces

    if (bUsePersistentHitResults && MaxHitResultsPerTrace > 0)
    {
        // Handle ReticleActors
        for (int32 PersistentHitResultIndex = 0; PersistentHitResultIndex < PersistentHitResults.Num(); PersistentHitResultIndex++)
        {
            FHitResult& HitResult = PersistentHitResults[PersistentHitResultIndex];

            // Update TraceStart because old persistent HitResults will have their original TraceStart and the player could have moved since then
            HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();

            if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get())
            {
                const bool bHitActor = HitResult.Actor != nullptr;

                if (bHitActor && !HitResult.bBlockingHit)
                {
                    LocalReticleActor->SetActorHiddenInGame(false);

                    const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.Actor->GetActorLocation() : HitResult.Location;

                    LocalReticleActor->SetActorLocation(ReticleLocation);
                    LocalReticleActor->SetIsTargetAnActor(bHitActor);
                }
                else
                {
                    LocalReticleActor->SetActorHiddenInGame(true);
                }
            }
        }

        if (PersistentHitResults.Num() < ReticleActors.Num())
        {
            // We have less hit results than ReticleActors, hide the extra ones
            for (int32 PersistentHitResultIndex = PersistentHitResults.Num(); PersistentHitResultIndex < ReticleActors.Num(); PersistentHitResultIndex++)
            {
                if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get())
                {
                    LocalReticleActor->SetIsTargetAnActor(false);
                    LocalReticleActor->SetActorHiddenInGame(true);
                }
            }
        }

        return PersistentHitResults;
    }
    
    return ReturnHitResults;
}

AGameplayAbilityWorldReticle* ANGATA_Trace::SpawnReticleActor(FVector Location, FRotator Rotation)
{
    if (ReticleClass)
    {
        AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, Location, Rotation);
        if (SpawnedReticleActor)
        {
            SpawnedReticleActor->InitializeReticle(this, MasterPC, ReticleParams);
            SpawnedReticleActor->SetActorHiddenInGame(true);
            ReticleActors.Add(SpawnedReticleActor);
            
            // This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
            // (In a client controlled player, this would only run on the client and therefore never replicate. If it runs
            // on a listen server, the reticle actor may replicate. We want consistency between client/listen server players.
            // Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
            // actors and there may be other targeting types that want to replicate the same reticle actor class).
            if (!ShouldProduceTargetDataOnServer)
            {
                SpawnedReticleActor->SetReplicates(false);
            }

            return SpawnedReticleActor;
        }
    }

    return nullptr;
}

void ANGATA_Trace::DestroyReticleActors()
{
    for (int32 i = ReticleActors.Num() - 1; i >= 0; i--)
    {
        if (ReticleActors[i].IsValid())
        {
            ReticleActors[i].Get()->Destroy();
        }
    }

    ReticleActors.Empty();
}

