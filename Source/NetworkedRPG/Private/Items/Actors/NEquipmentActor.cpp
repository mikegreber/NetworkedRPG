// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/NEquipmentActor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NetworkedRPG/NetworkedRPG.h"

// Sets default values
ANEquipmentActor::ANEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	// SkeletalMesh->SetGenerateOverlapEvents(true);  // Will toggle this with Activate() / Deactivate()
	// SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// SkeletalMesh->SetCollisionObjectType(ECC_Weapon);
	// SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	SetRootComponent(Mesh);

	bNetUseOwnerRelevancy = true;
	SetReplicates(true);
}


void ANEquipmentActor::BeginPlay()
{
	Super::BeginPlay();

	// SkeletalMesh->SetGenerateOverlapEvents(false); 
}


USkeletalMeshComponent* ANEquipmentActor::GetMesh() const
{
	return Mesh;
}
