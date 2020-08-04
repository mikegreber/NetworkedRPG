// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NMovementSystemComponent.h"
#include "Components/Combat/NCombatComponent.h"
#include "Components/NSpringArmComponent.h"
#include "Characters/NCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"


static int32 DebugMovementSystemComponent = 0;
FAutoConsoleVariableRef CVarDebugMovementSystemComponent(
    TEXT("NRPG.Debug.MovementSystemComponent"),
    DebugMovementSystemComponent,
    TEXT("Show movement system debug arrows.")
    TEXT("Blue - Actor forward rotator")
    TEXT("Green - Camera forward rotator")
    TEXT("Yellow - Character Look rotator")
    TEXT("Red - Velocity rotator"),
    ECVF_Cheat
    );



// Sets default values for this component's properties
UNMovementSystemComponent::UNMovementSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false); // Will turn on after initialization
	
	CameraSocketOffsetInterpSpeed = 4.f;
	AimOffsetNetUpdatesPerSecond = 3.f;

	DefaultCameraModeSettings = FCameraModeSettings(FCameraMode({0.f, 0.f, 80.f}, 4.f, 20.f, 200.f));

	SetIsReplicatedByDefault(true);
}


// Called every frame while ComponentTick is enabled
void UNMovementSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Owner)
	{
		return;
	}

	// Interpolate between camera offset positions
	if (Owner->IsLocallyControlled())
	{
		if (CameraSpringArmComponent && DesiredCameraSocketOffset != CameraSpringArmComponent->SocketOffset)
		{
			CameraSpringArmComponent->SocketOffset = FMath::VInterpTo(CameraSpringArmComponent->SocketOffset, DesiredCameraSocketOffset, DeltaTime, CameraSocketOffsetInterpSpeed);
		}
		else if (!DebugMovementSystemComponent)
		{
			SetComponentTickEnabled(false);
		}
	}

	if (DebugMovementSystemComponent)
	{
		DrawActorForwardRotator(FColor::Blue, -.5f);
		DrawCameraForwardRotator(FColor::Green, 0.f);
		DrawCharacterLookRotator(FColor::Yellow, 5.f);
		DrawVelocityRotator(FColor::Red, 10.f);
	}
}


void UNMovementSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicated so we use them to figure out the aim offset for remote players
	DOREPLIFETIME_CONDITION(UNMovementSystemComponent, QuantizedCameraRotationVector2D, COND_SkipOwner);

    // TODO CombatComponent may not need to be replicated here, should test
    DOREPLIFETIME_CONDITION(UNMovementSystemComponent, CombatComponent, COND_SkipOwner);

	// Replicated as this is used in anim blueprint for character stance
	DOREPLIFETIME_CONDITION(UNMovementSystemComponent, Stance, COND_SkipOwner);

    // TODO CameraSpringArmComponent may not need to be replicated here, should test
	DOREPLIFETIME(UNMovementSystemComponent, CameraSpringArmComponent);
}


void UNMovementSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNMovementSystemComponent::Initialize);
}


void UNMovementSystemComponent::BeginDestroy()
{
	if (CombatComponent && Owner->IsLocallyControlled())
	{
		CombatComponent->OnTargetUpdated.RemoveAll(this);
	}

	Super::BeginDestroy();
}


void UNMovementSystemComponent::Initialize()
{
    Owner = Cast<ACharacter>(GetOwner());
    if (!Owner)
    {
        if (DebugMovementSystemComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s UNMovementSystemComponent Can only be attached to ACharacter derived classes."), *FString(__FUNCTION__)), EPrintType::ShutDown);
        }

        return;
    }

    if (Owner->IsLocallyControlled())
    {
        CameraSpringArmComponent = Owner->FindComponentByClass<UNSpringArmComponent>();
        SetCombatComponent(Owner->FindComponentByClass<UNCombatComponent>());
        SetRotationMode(ENRotationMode::VelocityDirection);
        SetCameraModeToDefault();

        if (DebugMovementSystemComponent && !CameraSpringArmComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s Owner does not have UNSpringArmComponent."), *FString(__FUNCTION__)), EPrintType::Error);
        }
    }
}


void UNMovementSystemComponent::SetCombatComponent(UNCombatComponent* InCombatComponent)
{
    if (!InCombatComponent){
        if (DebugMovementSystemComponent)
        {
            Print(GetWorld(), FString::Printf(TEXT("%s <UNCombatComponent* InCombatComponent> was null."), *FString(__FUNCTION__)), EPrintType::Error);
        }

        return;
    }

    if (!OwnerHasAuthority())
    {
        ServerSetCombatComponent(InCombatComponent);
    }

    CombatComponent = InCombatComponent;

    if (Owner->IsLocallyControlled())
    {
        CombatComponent->OnTargetUpdated.AddDynamic(this, &UNMovementSystemComponent::OnTargetUpdated);
        OnTargetUpdated(nullptr);
    }
}


ENMovementGait UNMovementSystemComponent::GetMovementGait() const
{
	return MovementGait;
}


ENRotationMode UNMovementSystemComponent::GetRotationMode() const
{
	return RotationMode;
}


ENStance UNMovementSystemComponent::GetStance() const
{
	return Stance;
}


void UNMovementSystemComponent::SetMovementGait(ENMovementGait InMovementMode, bool bBroadcast)
{
	if (MovementGait != InMovementMode)
	{
		MovementGait = InMovementMode;

		UpdateCamera();
		
		if (bBroadcast)
		{
			BroadcastSystemState();
		}

		if (DebugMovementSystemComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)));
		}
	}
}


void UNMovementSystemComponent::SetRotationMode(ENRotationMode InRotationMode, bool bBroadcast)
{
	if (RotationMode != InRotationMode)
	{
		RotationMode = InRotationMode;

		switch(RotationMode)
		{
		case ENRotationMode::VelocityDirection:
			SetOrientRotationToMovement(true);
			
			if (DebugMovementSystemComponent)
			{
				Print(GetWorld(), FString::Printf(TEXT("%s VelocityDirection"), *FString(__FUNCTION__)));
			}
			
			break;
			
		case ENRotationMode::LookingDirection:
			
			if (DebugMovementSystemComponent)
			{
				Print(GetWorld(), FString::Printf(TEXT("%s LookingDirection"), *FString(__FUNCTION__)));
			}
			
			SetOrientRotationToMovement(false);
			break;
		}
		
		if (bBroadcast)
		{
			BroadcastSystemState();
		}
	}	
}


void UNMovementSystemComponent::SetStance(ENStance InStance, bool bBroadcast)
{
	if (Stance != InStance)
	{
		if (!OwnerHasAuthority())
		{
			ServerSetStance(InStance, bBroadcast);
		}
		
		Stance = InStance;
		
		if (bBroadcast)
		{
			BroadcastSystemState();
		}	
	}
}

void UNMovementSystemComponent::SetCombatType(ENCombatType InCombatType, bool bBroadcast)
{
	if (CombatType != InCombatType)
	{
		CombatType = InCombatType;

		// TODO add combat type to broadcast
//		if (bBroadcast)
//		{
//			BroadcastSystemState();
//		}
	}
	
}


void UNMovementSystemComponent::BroadcastSystemState() const
{
	OnSystemUpdated.Broadcast(MovementGait, RotationMode, Stance);
}


void UNMovementSystemComponent::SetCameraMode(FCameraModeSettings NewCameraState)
{
	CameraModeSettings = NewCameraState;
	UpdateCamera();
}


void UNMovementSystemComponent::SetCameraModeToDefault()
{
	CameraModeSettings = DefaultCameraModeSettings;
	UpdateCamera();
}


void UNMovementSystemComponent::UpdateCamera()
{
	if (Owner->IsLocallyControlled() && CameraSpringArmComponent)
	{
		const FCameraMode NewCameraMode = CameraModeSettings.GetCurrent(MovementGait);
		
		CameraSpringArmComponent->CameraLagSpeed = NewCameraMode.CameraLagSpeed;
		CameraSpringArmComponent->CameraRotationLagSpeed = NewCameraMode.CameraRotationLagSpeed;
		CameraSpringArmComponent->CameraLagMaxDistance = NewCameraMode.CameraLagMaxDistance;

		// Turn on tick to interpolate to new offset
		DesiredCameraSocketOffset = NewCameraMode.SocketOffset;
		SetComponentTickEnabled(true);
	}
}


FVector UNMovementSystemComponent::GetEyesLocation() const
{
	FVector OutLocation;
	FRotator OutRotation;
	Owner->GetActorEyesViewPoint(OutLocation, OutRotation);
	return OutLocation;
}


void UNMovementSystemComponent::OnTargetUpdated(UPrimitiveComponent* InTarget)
{
	// Start NetUpdateTick if we are not targeting (need to replicate camera offset)
	if (!InTarget && Owner->IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer(NetTickTimerHandle, this, &UNMovementSystemComponent::NetUpdateTick, 1 / AimOffsetNetUpdatesPerSecond, true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(NetTickTimerHandle);
	}
}


void UNMovementSystemComponent::SetOrientRotationToMovement(bool Value)
{
	if (!OwnerHasAuthority())
	{
		ServerSetOrientRotationToMovement(Value);
	}
	
	Owner->GetCharacterMovement()->bOrientRotationToMovement = Value;
}


void UNMovementSystemComponent::NetUpdateTick()
{
	const FRotator CameraRotator = GetCameraForwardRotator();
	ServerUpdateCameraRotation(FVector_NetQuantize2D(CameraRotator.Pitch, CameraRotator.Yaw));
}


void UNMovementSystemComponent::DrawActorForwardRotator(FColor Color, float ZOffset) const
{
	FRotator Rotator = GetActorForwardRotator();
	Rotator.Pitch = 0.0f;
	DrawLocomotionDebugArrow(Rotator, Color, ZOffset);
}


void UNMovementSystemComponent::DrawVelocityRotator(FColor Color, float ZOffset) const
{
	if (Owner->GetVelocity().Size() > 0.f)
	{
		FRotator Rotator = GetVelocityRotator();
		Rotator.Pitch = 0.0f;
		DrawLocomotionDebugArrow(Rotator, Color, ZOffset);
	}
}


void UNMovementSystemComponent::DrawCameraForwardRotator(FColor Color, float ZOffset)
{
	FRotator Rotator = GetCameraForwardRotator();
	Rotator.Pitch = 0.0f;
	DrawLocomotionDebugArrow(Rotator, Color, ZOffset);
}


void UNMovementSystemComponent::DrawCharacterLookRotator(FColor Color, float ZOffset)
{
	FRotator Rotator = GetLookRotationOffset() + GetActorForwardRotator();
	Rotator.Pitch = 0.0f;
	DrawLocomotionDebugArrow(Rotator, Color, ZOffset);
}


FRotator UNMovementSystemComponent::GetActorForwardRotator() const
{
	return Owner->GetActorForwardVector().Rotation().GetNormalized();
}


FRotator UNMovementSystemComponent::GetVelocityRotator() const
{
	return Owner->GetVelocity().Rotation().GetNormalized();
}


FRotator UNMovementSystemComponent::GetCameraForwardRotator() const
{	
	if (Owner->IsLocallyControlled() && CameraSpringArmComponent)
	{	
		return (GetActorForwardRotator() + CameraSpringArmComponent->GetRelativeSocketRotation().Rotator()).GetNormalized();
	}

	// Returns the replicated value this actor is not the local player - replicated in NetworkUpdateTick
	return FRotator(QuantizedCameraRotationVector2D.X, QuantizedCameraRotationVector2D.Y, 0.f);
}


FRotator UNMovementSystemComponent::GetLookRotationOffset() const
{
	
	if (CombatComponent && CombatComponent->IsLocked())
	{	
		return ((CombatComponent->GetTargetLocation() - GetEyesLocation()).Rotation() - GetActorForwardRotator()).GetNormalized();
	}	
	
	// Default to offset from camera forward rotation if not target locked
	return (GetCameraForwardRotator() - GetActorForwardRotator()).GetNormalized();
}


FRotator UNMovementSystemComponent::GetVelocityRotationOffset() const
{
	return (GetActorForwardRotator() - GetVelocityRotator()).GetNormalized();
}


float UNMovementSystemComponent::GetSpeed() const
{
	return Owner->GetVelocity().Size();
}


void UNMovementSystemComponent::DrawLocomotionDebugArrow(FRotator Rotator, FColor Color, float ZOffset) const
{
	const FVector Location = GetOwner()->GetActorLocation();
	DrawDebugDirectionalArrow(GetWorld(), Location + FVector(0.f,0.f, ZOffset), Location + Rotator.Vector() * 200.f  + FVector(0.f,0.f, ZOffset), 10, Color, false, -1, 0, 8);
}


void UNMovementSystemComponent::ServerUpdateCameraRotation_Implementation(FVector_NetQuantize2D InVector)
{
	QuantizedCameraRotationVector2D = InVector;
}

bool UNMovementSystemComponent::ServerUpdateCameraRotation_Validate(FVector_NetQuantize2D InVector)
{
	return true;
}


void UNMovementSystemComponent::ServerSetCombatComponent_Implementation(UNCombatComponent* InCombatComponent)
{
	SetCombatComponent(InCombatComponent);
}

bool UNMovementSystemComponent::ServerSetCombatComponent_Validate(UNCombatComponent* InCombatComponent)
{
	return true;
}


void UNMovementSystemComponent::ServerSetOrientRotationToMovement_Implementation(bool Value)
{
	SetOrientRotationToMovement(Value);
}

bool UNMovementSystemComponent::ServerSetOrientRotationToMovement_Validate(bool Value)
{
	return true;
}


void UNMovementSystemComponent::ServerSetStance_Implementation(ENStance InStance, bool bBroadcast)
{
	SetStance(InStance, bBroadcast);
}

bool UNMovementSystemComponent::ServerSetStance_Validate(ENStance InStance, bool bBroadcast)
{
	return true;
}


