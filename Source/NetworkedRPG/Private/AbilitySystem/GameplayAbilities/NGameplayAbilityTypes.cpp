// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbilityTypes.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"

bool FNGameplayEffectContainerSpec::HasValidEffects() const
{
	return TargetGameplayEffectSpecs.Num() > 0;
}


bool FNGameplayEffectContainerSpec::HasValidTargets() const
{
	return TargetData.Num() > 0;
}


void FNGameplayEffectContainerSpec::AddTargets(const TArray<FGameplayAbilityTargetDataHandle>& InTargetData,
	const TArray<FHitResult>& InHitResults, const TArray<AActor*>& InTargetActors)
{
	for (const FGameplayAbilityTargetDataHandle& TData : InTargetData)
	{
		TargetData.Append(TData);
	}

	for (const FHitResult& HitResult : InHitResults)
	{
		FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetData.Add(NewData);
	}

	if (InTargetActors.Num() > 0)
	{
		FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
		NewData->TargetActorArray.Append(InTargetActors);
		TargetData.Add(NewData);
	}
}

void FNGameplayEffectContainerSpec::SetTarget(AActor* TargetActor)
{
	FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
	NewData->SetActors({TargetActor});
	TargetData = {NewData};
}


void FNGameplayEffectContainerSpec::ClearTargets()
{
	TargetData.Clear();
}

void FNGameplayEffectContainerSpec::SetSetByCallerMagnitude(const FGameplayTag& ModifierTag, float Magnitude)
{
	for (auto& EffectSpec : TargetGameplayEffectSpecs)
	{
		EffectSpec.Data->SetSetByCallerMagnitude(ModifierTag, Magnitude);
	}
}

void FNGameplayEffectContainerSpec::SetSetByCallerMagnitude(const FName& ModifierTag, const float Magnitude)
{
	SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(ModifierTag), Magnitude);
}


void FNGameplayEffectContainerSpec::ApplyEffectToTarget(AActor* TargetActor)
{
	if (!OwningAbility)
	{
		return;
	}
	
	if (TargetActor)
	{
		FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
		NewData->SetActors({TargetActor});
		TargetData.Add(NewData);
	}
	
	OwningAbility->ApplyEffectContainerSpec(*this);
}
