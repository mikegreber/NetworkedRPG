// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayAbilities/NGameplayAbility_Damaging.h"
#include "Interface/NDamageableInterface.h"

UNGameplayAbility_Damaging::UNGameplayAbility_Damaging()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	EffectHitTag = FGameplayTag::RequestGameplayTag("Effect.Hit");
	DamageModifierTag = FGameplayTag::RequestGameplayTag("Modifier.Damage");
	EffectContainerMap.Emplace(EffectHitTag);

	Damage = 12.f;
}

bool UNGameplayAbility_Damaging::CanDamage(AActor* TargetActor) const
{
	const INDamageableInterface* Owner = Cast<INDamageableInterface>(GetAvatarActorFromActorInfo());
	const INDamageableInterface* Target = Cast<INDamageableInterface>(TargetActor);

	if (Owner && Target)
	{
		return Owner->GetTeam() != Target->GetTeam();
	}

	return false;
}

void UNGameplayAbility_Damaging::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (!EffectContainerMap.Contains(EffectHitTag))
	{
		Print(GetWorld(), "%s Must have EffectContainer in EffectContainerMap with Key Effect.Hit.", EPrintType::ShutDown);
		return;
	}

	EffectContainerSpecMap[EffectHitTag].SetSetByCallerMagnitude("Modifier.Damage", Damage);
}
