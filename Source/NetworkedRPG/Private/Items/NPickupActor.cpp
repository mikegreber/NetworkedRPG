// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/NPickupActor.h"
#include "Characters/Components/NInventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Items/NItem.h"
#include "Components/TimelineComponent.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

static int32 DebugPickupActor = 0;
FAutoConsoleVariableRef CVarDebugPickupActor(
    TEXT("NRPG.Debug.PickupActor"),
    DebugPickupActor,
    TEXT("Print debug information for PickupActors"),
    ECVF_Cheat);


// Sets default values
ANPickupActor::ANPickupActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>("CollisionCapsule");
	CollisionCapsule->SetSimulatePhysics(true);
	CollisionCapsule->CanCharacterStepUpOn = ECB_No;
	CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	CollisionCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionCapsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionCapsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	RootComponent = CollisionCapsule;
	
	PickupCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("PickupSphere");
	PickupCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupCapsuleComponent->SetCapsuleSize(60.f, 80.f);
	PickupCapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupCapsuleComponent->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	PickupCapsuleComponent->SetCollisionObjectType(ECC_WorldDynamic);
	PickupCapsuleComponent->SetupAttachment(RootComponent);

	AnimationRoot = CreateDefaultSubobject<USceneComponent>("AnimationRoot");
	AnimationRoot->SetupAttachment(RootComponent);
	
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMesh->SetupAttachment(AnimationRoot);
	
	bUseItemMesh = true;

	ItemLevel = 1;
	ItemCount = 1;

	SetReplicates(true);
}


// Called every frame if rotation speed or BobbingCurve set
void ANPickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AnimationRoot)
	{
		AnimationRoot->SetRelativeRotation(AnimationRoot->GetRelativeRotation() + RotationSpeed * DeltaTime);
		BobbingTimeline.TickTimeline(DeltaTime);
	}
}


// Called when the game starts or when spawned
void ANPickupActor::BeginPlay()
{
	Super::BeginPlay();

	if (!Item)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "ANPickupActor: No Item Set.", true, true, FColor::Red);
    }

	InitialOffset = AnimationRoot->GetRelativeLocation();

	if (BobbingCurve && BobbingHeight > 0.f)
	{	
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindDynamic(this, &ANPickupActor::HandleTimelineAnimation);

		BobbingTimeline.AddInterpFloat(BobbingCurve, ProgressFunction);
		BobbingTimeline.SetLooping(true);
		BobbingTimeline.PlayFromStart();
	} 
	else if (RotationSpeed == FRotator::ZeroRotator)
	{
		// No animations set so don't need tick
		SetActorTickEnabled(false);
	}
	
	if (DebugPickupActor)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "NPickupActor Spawned");
	}
}

void ANPickupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bUseItemMesh)
	{
		UpdateItemMesh();
	}
}

void ANPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANPickupActor, Item);
}


void ANPickupActor::Interact(AActor* InActor)
{
	if (APlayerController* PC =  InActor->GetInstigatorController<APlayerController>())
	{
		UNInventoryComponent* Inventory = PC->FindComponentByClass<UNInventoryComponent>();
		if (Inventory)
		{
			const int32 Overflow = Inventory->AddInventoryItem(Item, ItemCount, ItemLevel);

			if (DebugPickupActor)
			{
				UKismetSystemLibrary::PrintString(GetWorld(), FString("ANPickupActor: Pickup overflow: ") + FString::FromInt(Overflow), true, true, Overflow == 0 ? FColor::Green : FColor::Red);
			}
			
			if (Overflow)
			{
				ItemCount = Overflow;
			}
			else
			{
				Destroy();
			}
		}
	}
}


FVector ANPickupActor::GetIndicatorOffset()
{
	return InteractionIndicatorOffset;
}


USceneComponent* ANPickupActor::GetInteractableRootComponent()
{
	return GetRootComponent();
}


ENInteractableType ANPickupActor::GetInteractableType()
{
	return ENInteractableType::Item;
}


void ANPickupActor::SetItem(UNItem* InItem, int32 InCount)
{
	Item = InItem;
	ItemCount = InCount;
	UpdateItemMesh();
}


void ANPickupActor::UpdateItemMesh() const
{
	if (Item && Item->ItemMesh)
	{
		SkeletalMesh->SetSkeletalMesh(Item->ItemMesh);
	}
}


void ANPickupActor::HandleTimelineAnimation(float Value)
{
	const FVector NewLocation = FMath::Lerp(InitialOffset, InitialOffset + FVector(0.f,0.f, BobbingHeight), Value);
	AnimationRoot->SetRelativeLocation(NewLocation);
}


void ANPickupActor::OnRep_Item() const
{
	UpdateItemMesh();
}
