// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Equipment/NEquipmentActor.h"


#include "Kismet/KismetSystemLibrary.h"
#include "NetworkedRPG/NetworkedRPG.h"

// Sets default values
ANEquipmentActor::ANEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMesh->SetGenerateOverlapEvents(true);  // Will toggle this with Activate() / Deactivate()
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SkeletalMesh->SetCollisionObjectType(ECC_Weapon);
	SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	SetRootComponent(SkeletalMesh);
	SkeletalMesh->OnComponentBeginOverlap.AddDynamic(this, &ANEquipmentActor::OnCollision);

	bNetUseOwnerRelevancy = true;
	SetReplicates(true);
}

void ANEquipmentActor::BeginPlay()
{
	Super::BeginPlay();

	SkeletalMesh->SetGenerateOverlapEvents(false); 
}

USkeletalMeshComponent* ANEquipmentActor::GetMesh() const
{
	return SkeletalMesh;
}

void ANEquipmentActor::Attach(USkeletalMeshComponent* Mesh, FName SocketName, ESocketOffset SocketOffset)
{
	AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	SetActorRelativeTransform(SocketOffset == ESocketOffset::Primary ? PrimaryAttachmentRelativeTransform : SecondaryAttachmentRelativeTransform);
}

void ANEquipmentActor::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("Activate"))
	SkeletalMesh->SetGenerateOverlapEvents(true);
}

void ANEquipmentActor::Deactivate()
{
	UE_LOG(LogTemp, Warning, TEXT("Deactivate"))
	SkeletalMesh->SetGenerateOverlapEvents(false);
	ClearHits();
}

void ANEquipmentActor::ClearHits()
{
	HitActors.Reset();
}

void ANEquipmentActor::OnCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UKismetSystemLibrary::PrintString(GetWorld(), "Collision!");
	if (OtherActor != GetOwner() && !HitActors.Contains(OtherActor))
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Collision!");
		HitActors.Add(OtherActor);
		OnHit.Broadcast(OtherActor, bFromSweep, SweepResult);
	}
}

