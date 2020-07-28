// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NEquipmentActor.generated.h"

UENUM()
enum class ESocketOffset
{
	Primary,
    Secondary
};

UCLASS(BlueprintType)
class NETWORKEDRPG_API ANEquipmentActor : public AActor
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatHit, AActor*, OtherActor, bool, bFromSweep, const FHitResult&, Hit);

protected:
	virtual void BeginPlay() override;
	
public:	
	// Sets default values for this actor's properties
	ANEquipmentActor();

	USkeletalMeshComponent* GetMesh() const;

	void Attach(USkeletalMeshComponent* Mesh, FName SocketName, ESocketOffset SocketOffset = ESocketOffset::Primary);
	
	/** Enables collision on the collision component - Should override in child class */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void Activate();

	/** Disables collision on the collision component and clears the HitActors list - Should override in child class */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void Deactivate();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void ClearHits();

	UPROPERTY(BlueprintAssignable)
	FOnCombatHit OnHit;

	UPROPERTY(EditAnywhere, Category = "Equipment")
	FTransform PrimaryAttachmentRelativeTransform;

	UPROPERTY(EditAnywhere, Category = "Equipment")
    FTransform SecondaryAttachmentRelativeTransform;
	
protected:	
	UPROPERTY(EditAnywhere, Category = "Equipment")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY()
	TArray<AActor*> HitActors;
	
	/** Function to bind to CollisionCapsule OnComponentBeginOverlap, Broadcasts OnHit when overlapping an actor for the first time */
	UFUNCTION()
	virtual void OnCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	

};
