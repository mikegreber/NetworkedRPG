// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Equipment/NProjectile.h"


#include "DrawDebugHelpers.h"
#include "Characters/NCharacterBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ANProjectile::ANProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2000.f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(FName("CollisionComponent"));
	CollisionComponent->SetGenerateOverlapEvents(false);
	CollisionComponent->SetCollisionObjectType(ECC_Weapon);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ANProjectile::OnCollision);
	
	bReplicates = true;
}


void ANProjectile::PostActorCreated()
{
	Super::PostActorCreated();

	if (HasAuthority())
	{
		CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
		CollisionComponent->SetGenerateOverlapEvents(true);
	}
}


void ANProjectile::OnCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
		if (ANCharacterBase* Actor = Cast<ANCharacterBase>(OtherActor))
		{
			UKismetSystemLibrary::PrintString(GetWorld(), FString::FromInt(OtherActor != GetInstigator()));
			if (UAbilitySystemComponent* ASC = Actor->GetAbilitySystemComponent())
			{
				FHitResult Hit;
				Hit.Normal = FVector_NetQuantizeNormal(GetActorRotation().Vector());
				DrawDebugSphere(GetWorld(), GetActorLocation(), 20.f, 10, FColor::Red, false, 2);
				DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorRotation().Vector() * 200, FColor::Blue, false, 3.f);
				UKismetSystemLibrary::PrintString(GetWorld(), GetActorRotation().Vector().ToString());
				DamageEffectSpecHandle.Data.Get()->GetContext().AddHitResult(Hit);
				ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
				Destroy();
			}
		}
}

