// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Components/NMovementSystemComponent.h"
#include "DrawDebugHelpers.h"
// #include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/NCharacter.h"
#include "Characters/Components/NSpringArmComponent.h"
#include "Characters/Components/NCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

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

	// RotationMode = ENRotationMode::VelocityDirection;

	SetIsReplicatedByDefault(true);
}


// Called every frame while ComponentTick is enabled
void UNMovementSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Interpolate between camera offset positions
	if (bLocallyControlled)
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
	DOREPLIFETIME_CONDITION(UNMovementSystemComponent, CombatComponent, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UNMovementSystemComponent, CombatMode, COND_SkipOwner);
	DOREPLIFETIME(UNMovementSystemComponent, CameraSpringArmComponent);
}


void UNMovementSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNMovementSystemComponent::Initialize);
}


void UNMovementSystemComponent::BeginDestroy()
{
	if (CombatComponent && bLocallyControlled)
	{
		CombatComponent->OnTargetUpdated.RemoveAll(this);
	}

	Super::BeginDestroy();
}


ENMovementGait UNMovementSystemComponent::GetMovementGait() const
{
	return MovementGait;
}


ENRotationMode UNMovementSystemComponent::GetRotationMode() const
{
	return RotationMode;
}


ENCombatMode UNMovementSystemComponent::GetCombatMode() const
{
	return CombatMode;
}


void UNMovementSystemComponent::SetMovementGait(ENMovementGait InMovementMode, bool bBroadcast)
{
	if (MovementGait != InMovementMode)
	{
		MovementGait = InMovementMode;

		UpdateCameraState();
		
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


void UNMovementSystemComponent::SetCombatMode(ENCombatMode InCombatMode, bool bBroadcast)
{
	if (CombatMode != InCombatMode)
	{
		if (GetOwnerRole() != ROLE_Authority)
		{
			ServerSetCombatMode(InCombatMode, bBroadcast);
		}
		
		CombatMode = InCombatMode;
		
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

		if (bBroadcast)
		{
			BroadcastSystemState();
		}
	}
	
}

void UNMovementSystemComponent::ServerSetCombatMode_Implementation(ENCombatMode InCombatMode, bool bBroadcast)
{
	SetCombatMode(InCombatMode, bBroadcast);
}

bool UNMovementSystemComponent::ServerSetCombatMode_Validate(ENCombatMode InCombatMode, bool bBroadcast)
{
	return true;
}

void UNMovementSystemComponent::BroadcastSystemState() const
{
	OnSystemUpdated.Broadcast(MovementGait, RotationMode, CombatMode);
}

void UNMovementSystemComponent::SetCameraMode(FCameraModeSettings NewCameraState)
{
	CameraModeSettings = NewCameraState;
	UpdateCameraState();
}

void UNMovementSystemComponent::SetCameraModeToDefault()
{
	CameraModeSettings = DefaultCameraModeSettings;
	UpdateCameraState();
}

void UNMovementSystemComponent::UpdateCameraState()
{
	if (bLocallyControlled && CameraSpringArmComponent)
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
	if (bLocallyControlled && CameraSpringArmComponent)
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


void UNMovementSystemComponent::Initialize()
{
	// Owner = Cast<ANCharacterBase>(GetOwner());
	Owner = Cast<ACharacter>(GetOwner());
	
	if (Owner && Owner->IsLocallyControlled())
	{
		bLocallyControlled = true;
		SetCombatComponent(Owner->FindComponentByClass<UNCombatComponent>());
		SetSpringArmComponent(Owner->FindComponentByClass<UNSpringArmComponent>());
		SetRotationMode(ENRotationMode::VelocityDirection);
		SetCameraModeToDefault();
	}
	else if (Owner->IsLocallyControlled())
	{
		if (DebugMovementSystemComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s UNMovementSystemComponent Can only be attached to ACharacter derived classes."), *FString(__FUNCTION__)), EPrintType::ShutDown);
		}
	}
}


void UNMovementSystemComponent::SetSpringArmComponent(UNSpringArmComponent* SpringArm)
{
	if (SpringArm)
	{
		CameraSpringArmComponent = SpringArm;
	}
	else
	{
		if (DebugMovementSystemComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s <UNSpringArmComponent* SpringArm> was null."), *FString(__FUNCTION__)), EPrintType::Error);
		}
	}
}


void UNMovementSystemComponent::SetCombatComponent(UNCombatComponent* InCombatComponent)
{

	if (InCombatComponent)
	{
		if (GetOwnerRole() != ROLE_Authority)
		{
			ServerSetCombatComponent(InCombatComponent);
		}
		
		CombatComponent = InCombatComponent;
		
		if (bLocallyControlled)
		{
			CombatComponent->OnTargetUpdated.AddDynamic(this, &UNMovementSystemComponent::OnTargetUpdated);
			OnTargetUpdated(nullptr);
		}
	}
	else
	{
		if (DebugMovementSystemComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s <UNCombatComponent* InCombatComponent> was null."), *FString(__FUNCTION__)), EPrintType::Error);
		}
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
	if (!InTarget && bLocallyControlled)
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
	if (GetOwnerRole() != ROLE_Authority)
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


void UNMovementSystemComponent::DrawLocomotionDebugArrow(FRotator Rotator, FColor Color, float ZOffset) const
{
	FVector Location = GetOwner()->GetActorLocation();
	DrawDebugDirectionalArrow(GetWorld(), Location + FVector(0.f,0.f, ZOffset), Location + Rotator.Vector() * 200.f  + FVector(0.f,0.f, ZOffset), 10, Color, false, -1, 0, 8);
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