// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Components/NTargetingComponent.h"

#include "DrawDebugHelpers.h"
#include "Characters/Components/NSpringArmComponent.h"
#include "Characters/Components/NMovementSystemComponent.h"
#include "Characters/Components/NTargetingComponentInterface.h"
#include "Characters/Components/NTargetComponent.h"
#include "Player/NCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UNTargetingComponent::UNTargetingComponent()
{
	// Toggle tick on and off for when targeting or not targeting
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false);

	MaxAngleForTargeting = 100.f;
	MaxTargetDetectionDistance = 1000.f;
	MaxTargetLockDistance = 1000.f;
	TargetingCameraPitchModifier = 17.f;

	// Anything we want to target must have their object response to Targeting set to Overlap
	CollisionObjectType = ECC_Targeting;
	
	SetIsReplicatedByDefault(true);
}


// Called every frame when tick is active
void UNTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsLocked() && PlayerController->IsLocalController())
	{
		const FVector VectorToTarget = Target->GetComponentLocation() - Interface->GetEyesLocation();
		// DrawDebugLine(GetWorld(), Target->GetComponentLocation(), Interface->GetEyesLocation(), FColor::Red, false, 0.5f, 4, 5);
		
		if (VectorToTarget.Size() > MaxTargetLockDistance)
		{
			UnLock();
		}
		else
		{
			// Keep camera locked on target
			FRotator TargetRotation = VectorToTarget.Rotation();
			TargetRotation.Pitch -= TargetingCameraPitchModifier;

			PlayerController->SetControlRotation(TargetRotation);
		}
	}
	else
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}


// Called when the game starts or actor with this component is spawned
void UNTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNTargetingComponent::Initialize);
}


void UNTargetingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UNTargetingComponent, Target, COND_SkipOwner);
}


void UNTargetingComponent::OnMovementSystemUpdated(ENMovementGait InMovementMode, ENRotationMode InRotationMode, ENCombatMode InCombatMode)
{
	if (IsLocked() && InCombatMode != ENCombatMode::None || InCombatMode != ENCombatMode::Ranged)
	{
		if (InRotationMode != ENRotationMode::LookingDirection)
		{
			Interface->GetMovementSystemComponent()->SetRotationMode(ENRotationMode::LookingDirection);
		}
	}
	else if (InRotationMode != ENRotationMode::VelocityDirection)
	{
		Interface->GetMovementSystemComponent()->SetRotationMode(ENRotationMode::VelocityDirection);
	}
}


bool UNTargetingComponent::IsLocked() const
{
	return Target != nullptr;
}


AActor* UNTargetingComponent::GetTargetActor() const
{
	return Target ? Target->GetOwner() : nullptr;
}


FVector UNTargetingComponent::GetTargetLocation() const
{
	return Target ? Target->GetComponentLocation() : FVector();
}

void UNTargetingComponent::ToggleTargetLock()
{
	if (IsLocked())
	{
		UnLock();
	}
	else if (FindSortedTarget())
	{
		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UNTargetingComponent::SwitchTargetToRight()
{
	if (IsLocked())
	{
		FindSortedTarget([](float AngleToTurn, float ClosestAngele)
        {
            return AngleToTurn < 0 && -AngleToTurn < FMath::Abs(ClosestAngele);
        });
	}
}

void UNTargetingComponent::SwitchTargetToLeft()
{
	if (IsLocked())
	{
		FindSortedTarget([](float AngleToTurn, float ClosestAngele)
        {
            return AngleToTurn > 0 && AngleToTurn < ClosestAngele;
        });	
	}		
}

void UNTargetingComponent::Initialize()
{
	Interface = GetOwner();
	if (Interface && Interface->GetPlayerController() && Interface->GetPlayerController()->IsLocalController())
	{
		Interface->GetMovementSystemComponent()->OnSystemUpdated.AddDynamic(this, &UNTargetingComponent::OnMovementSystemUpdated);
		PlayerController = Interface->GetPlayerController();
		SpringArm = Interface->GetSpringArm();
		CreateCollisionSphere();
		CreateTargetWidget();
	}

	if (!ensureAlwaysMsgf(MaxTargetLockDistance >= MaxTargetDetectionDistance, TEXT("MaxTargetLockDistance should be greater or equal to MaxTargetDetectionDistance")))
	{
		MaxTargetLockDistance = MaxTargetDetectionDistance;
	}
}

void UNTargetingComponent::CreateCollisionSphere()
{
	TargetingCollisionComponent = NewObject<USphereComponent>(this);
	TargetingCollisionComponent->RegisterComponent();
	TargetingCollisionComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	TargetingCollisionComponent->SetSphereRadius(MaxTargetDetectionDistance);
	TargetingCollisionComponent->SetVisibility(true);
	TargetingCollisionComponent->ShapeColor = FColor::Red;
	TargetingCollisionComponent->bHiddenInGame = false;	
	TargetingCollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	TargetingCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetingCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TargetingCollisionComponent->SetCollisionResponseToChannel(ECC_Target, ECR_Overlap);
	TargetingCollisionComponent->SetCollisionObjectType(CollisionObjectType);
}

void UNTargetingComponent::CreateTargetWidget()
{
	if (TargetIndicatorWidgetClass)
	{
		TargetIndicatorWidget = CreateWidget<UUserWidget>(PlayerController, TargetIndicatorWidgetClass);
		TargetIndicatorWidgetComponent = NewObject<UWidgetComponent>(this);
		TargetIndicatorWidgetComponent->RegisterComponent();
		TargetIndicatorWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		TargetIndicatorWidgetComponent->SetWidget(TargetIndicatorWidget);
		TargetIndicatorWidgetComponent->SetHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing TargetIndicatorWidgetClass for %s. Please fill in the character's Blueprint."), *FString(__FUNCTION__), *GetName());
    }
}

void UNTargetingComponent::SetTarget(UPrimitiveComponent* InTarget)
{
	if (Target != InTarget)
	{
		Target = InTarget;
		OnTargetUpdated.Broadcast(InTarget);
	}
}

void UNTargetingComponent::Lock(UPrimitiveComponent* TargetToLock)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerLock(TargetToLock);
	}
	
	SetTarget(TargetToLock);

	if (PlayerController && PlayerController->IsLocalController())
	{
		// Set the proper camera settings
		Interface->GetMovementSystemComponent()->OnSystemUpdated.AddDynamic(this, &UNTargetingComponent::OnMovementSystemUpdated);
		Interface->GetMovementSystemComponent()->BroadcastSystemState();
	
		// Attach target indicator widget to Target and show it
		if (ensureAlways(TargetIndicatorWidgetComponent))
		{
			TargetIndicatorWidgetComponent->AttachToComponent(Target, FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetIndicatorWidgetComponent->SetHiddenInGame(false);
		}
	}
}


void UNTargetingComponent::UnLock()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerUnLock();
	}	

	SetTarget(nullptr);

	if (PlayerController && PlayerController->IsLocalController())
	{
		// Set control rotation to where the camera is currently faced so we don't get a jump when we go back to the regular camera rotation lag
		const FRotator CameraWorldRotation = GetOwner()->GetActorRotation() + SpringArm->GetRelativeSocketRotation().Rotator();
		PlayerController->SetControlRotation(CameraWorldRotation);

		Interface->GetMovementSystemComponent()->SetRotationMode(ENRotationMode::VelocityDirection, false);
		// Interface->GetMovementSystemComponent()->SetCameraMode(ENCameraMode::Unlocked, false);
		Interface->GetMovementSystemComponent()->BroadcastSystemState();
		Interface->GetMovementSystemComponent()->OnSystemUpdated.RemoveAll(this);
	
		// Attach target indicator widget to owner and hide it
		if (ensureAlways(TargetIndicatorWidgetComponent))
		{
			TargetIndicatorWidgetComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			TargetIndicatorWidgetComponent->SetHiddenInGame(true);	
		}
	}
}


TArray<UPrimitiveComponent*> UNTargetingComponent::GetAvailableTargets() const
{
	TArray<UPrimitiveComponent*> Targets;
	if (TargetingCollisionComponent)
	{
		TargetingCollisionComponent->GetOverlappingComponents(Targets);
	}
	return Targets;
}


bool UNTargetingComponent::FindSortedTarget(TFunctionRef<bool(float AngleToTurn, float ClosestAngle)> Predicate)
{
	const TArray<UPrimitiveComponent*> Targets = GetAvailableTargets();

	float ClosestAngle = MaxAngleForTargeting;
	UPrimitiveComponent* NewTarget = nullptr;

	// Find the target with the lowest turning angle that is less than the MaxAngleForTargeting, configured with the Predicate
	for (UPrimitiveComponent* InTarget : Targets)
	{
		if (InTarget != Target)
		{
			const FRotator RequiredRotation = (InTarget->GetComponentLocation() - GetOwnerLocation()).Rotation();
			const FRotator AngleToTurn = (GetControlRotation() - RequiredRotation).GetNormalized(); // Makes angle -180 to 180
			 
			if (Predicate(AngleToTurn.Yaw, ClosestAngle))
			{
				ClosestAngle = AngleToTurn.Yaw;
				NewTarget = InTarget;
			}
		}
	}

	if (NewTarget)
	{
		Lock(NewTarget);
	}
	
	return NewTarget != nullptr;
}


FRotator UNTargetingComponent::GetControlRotation() const
{
	return PlayerController->GetControlRotation();
}


FVector UNTargetingComponent::GetOwnerLocation() const
{
	return GetOwner()->GetActorLocation();
}


void UNTargetingComponent::ServerLock_Implementation(UPrimitiveComponent* TargetToLock)
{
	Lock(TargetToLock);
}

bool UNTargetingComponent::ServerLock_Validate(UPrimitiveComponent* TargetToLock)
{
	return true;
}


void UNTargetingComponent::ServerUnLock_Implementation()
{
	UnLock();
}

bool UNTargetingComponent::ServerUnLock_Validate()
{
	return true;
}
