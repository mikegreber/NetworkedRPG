// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NEquipmentActor.generated.h"

UCLASS(BlueprintType)
class NETWORKEDRPG_API ANEquipmentActor : public AActor
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatHit, AActor*, OtherActor, bool, bFromSweep, const FHitResult&, Hit);

public:	
	// Sets default values for this actor's properties
	ANEquipmentActor();
	
protected:
	virtual void BeginPlay() override;
	
public:	
	USkeletalMeshComponent* GetMesh() const;
	
protected:
	/** The skeletal mesh of this EquipmentActor. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	USkeletalMeshComponent* Mesh;
};
