// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/GameplayAbilities/NGameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "NProjectile.generated.h"

class UNGameplayAbility;
class UProjectileMovementComponent;
class USphereComponent;

/**
 * A Spawnable projectile. Applies InHitEffectContainerSpec on collision with Actor that implements INDamageable Interface and overlaps Weapon collision.
 * **MUST** call Initialize() after spawn. 
 */
UCLASS()
class NETWORKEDRPG_API ANProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANProjectile();

	/** Should call after spawning projectile.  */
	virtual void Initialize(const FNGameplayEffectContainerSpec& IntHitEffectContainerSpec, float InRange, float InKnockStrength);

	/** ProjectileMovementComponent - Should set projectile settings in BP. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	UProjectileMovementComponent* ProjectileMovement;

	/** CollisionComponent - Should set size in BP. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	USphereComponent* CollisionComponent;

protected:
	/** Properties set in Initialize. */
	float KnockStrength;
	FNGameplayEffectContainerSpec HitEffectContainerSpec;
		
	UFUNCTION()
	virtual void OnCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
