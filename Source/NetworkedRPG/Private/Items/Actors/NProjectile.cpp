// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/NProjectile.h"
#include "AbilitySystemComponent.h"
#include "Characters/NCharacterBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbility.h"

// Sets default values
ANProjectile::ANProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2000.f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(FName("CollisionComponent"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetCollisionObjectType(ECC_Weapon);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ANProjectile::OnCollision);
	
	bReplicates = true;
}


void ANProjectile::Initialize(const FNGameplayEffectContainerSpec& IntHitEffectContainerSpec, float InRange, float InKnockStrength)
{
	HitEffectContainerSpec = IntHitEffectContainerSpec;
	KnockStrength = InKnockStrength;
	SetLifeSpan(InRange / ProjectileMovement->InitialSpeed);
}


void ANProjectile::OnCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetInstigator())
	{
		return;
	}
	
	INDamageableInterface* Damageable = Cast<INDamageableInterface>(OtherActor);
	if (Damageable)
	{
		HitEffectContainerSpec.ApplyEffectToTarget(OtherActor);
		Damageable->OnHit(KnockStrength, GetActorRotation().Yaw);
	}
	
	Destroy();
}

