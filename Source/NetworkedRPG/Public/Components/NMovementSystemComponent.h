// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "NTypes.h"
#include "Components/ActorComponent.h"
#include "NMovementSystemComponent.generated.h"

// TODO create and move to NetTypes.h
template<int32 MaxValue, int32 NumBits>
bool SerializeFixedVector2D(FVector2D &Vector, FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		bool success = true;
		success &= WriteFixedCompressedFloat<MaxValue, NumBits>(Vector.X, Ar);
		success &= WriteFixedCompressedFloat<MaxValue, NumBits>(Vector.Y, Ar);
		return success;
	}

	ReadFixedCompressedFloat<MaxValue, NumBits>(Vector.X, Ar);
	ReadFixedCompressedFloat<MaxValue, NumBits>(Vector.Y, Ar);
	return true;
}

template<uint32 ScaleFactor, int32 MaxBitsPerComponent>
bool WritePackedVector2D(FVector2D Value, FArchive& Ar)	// Note Value is intended to not be a reference since we are scaling it before serializing!
{
	check(Ar.IsSaving());

	// Scale vector by quant factor first
	Value *= ScaleFactor;

	// Nan Check
	if( Value.ContainsNaN() )
	{
		logOrEnsureNanError(TEXT("WritePackedVector: Value contains NaN, clearing for safety."));
		FVector2D	Dummy(0, 0);
		WritePackedVector2D<ScaleFactor, MaxBitsPerComponent>(Dummy, Ar);
		return false;
	}

	// Some platforms have RoundToInt implementations that essentially reduces the allowed inputs to 2^31.
	const FVector2D ClampedValue(FMath::Clamp(Value.X, -1073741824.0f, 1073741760.0f), FMath::Clamp(Value.Y, -1073741824.0f, 1073741760.0f));
	bool bClamp = ClampedValue != Value;

	// Do basically FVector::SerializeCompressed
	int32 IntX	= FMath::RoundToInt(ClampedValue.X);
	int32 IntY	= FMath::RoundToInt(ClampedValue.Y);
			
	uint32 Bits	= FMath::Clamp<uint32>( FMath::CeilLogTwo( 1 + FMath::Max( FMath::Abs(IntX), FMath::Abs(IntY) ) ), 1, MaxBitsPerComponent ) - 1;

	// Serialize how many bits each component will have
	Ar.SerializeInt( Bits, MaxBitsPerComponent );

	int32  Bias	= 1<<(Bits+1);
	uint32 Max	= 1<<(Bits+2);
	uint32 DX	= IntX + Bias;
	uint32 DY	= IntY + Bias;

	if (DX >= Max) { bClamp=true; DX = static_cast<int32>(DX) > 0 ? Max-1 : 0; }
	if (DY >= Max) { bClamp=true; DY = static_cast<int32>(DY) > 0 ? Max-1 : 0; }
	
	Ar.SerializeInt( DX, Max );
	Ar.SerializeInt( DY, Max );

	return !bClamp;
}

template<uint32 ScaleFactor, int32 MaxBitsPerComponent>
bool ReadPackedVector2D(FVector2D &Value, FArchive& Ar)
{
	uint32 Bits	= 0;

	// Serialize how many bits each component will have
	Ar.SerializeInt( Bits, MaxBitsPerComponent );

	int32  Bias = 1<<(Bits+1);
	uint32 Max	= 1<<(Bits+2);
	uint32 DX	= 0;
	uint32 DY	= 0;
	
	Ar.SerializeInt( DX, Max );
	Ar.SerializeInt( DY, Max );
	
	
	float fact = (float)ScaleFactor;

	Value.X = (float)(static_cast<int32>(DX)-Bias) / fact;
	Value.Y = (float)(static_cast<int32>(DY)-Bias) / fact;

	return true;
}

template<uint32 ScaleFactor, int32 MaxBitsPerComponent>
bool SerializePackedVector2D(FVector2D &Vector, FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		return  WritePackedVector2D<ScaleFactor, MaxBitsPerComponent>(Vector, Ar);
	}

	ReadPackedVector2D<ScaleFactor, MaxBitsPerComponent>(Vector, Ar);
	return true;
}

USTRUCT()
struct FVector_NetQuantize2D : public FVector2D
{
	GENERATED_USTRUCT_BODY()

FORCEINLINE FVector_NetQuantize2D()
	{}

	explicit FORCEINLINE FVector_NetQuantize2D(EForceInit E)
: FVector2D(E)
	{}

	FORCEINLINE FVector_NetQuantize2D(float InX, float InY)
: FVector2D(InX, InY)
	{}

	FORCEINLINE FVector_NetQuantize2D(const FVector2D &InVec)
	{
		FVector2D::operator=(InVec);
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = SerializePackedVector2D<1, 8>(*this, Ar);
		return true;
	}
};

template<>
struct TStructOpsTypeTraits< FVector_NetQuantize2D > : public TStructOpsTypeTraitsBase2< FVector_NetQuantize2D >
{
	enum 
	{
		WithNetSerializer = true,
WithNetSharedSerialization = true,
};
};


enum class ENCombatType : unsigned char;

// TODO Remove this delegate and replace with FNotifyDelegate - same functionality
// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotifyListeners);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMovementSystemUpdatedDelegate, ENMovementGait, InMovementMode, ENRotationMode, InRotationMode, ENStance, Stance);


class UNSpringArmComponent;
class UNCombatComponent;
class ACharacter;

/** Sections
*	1. Blueprint Settings
*	2. State
*	3. References
*	4. Overrides
*	5. Interface and Methods
*		5a. Equipments Slots
*		5b. Weapons
*		5c. Targeting
*	6. Server RPC's
*/

/**
* This component handles the movement state and the camera of the character.
* State is referenced from animation blueprint and combat component.
* Can be added to characters implementing the INCombatComponentInterface.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKEDRPG_API UNMovementSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties */
	UNMovementSystemComponent();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** The rate of net updates for aim offset when not targeting */
	UPROPERTY(EditAnywhere, Category="Settings|Network")
	float AimOffsetNetUpdatesPerSecond;

	/** Interp speed for transitioning between camera offsets */
	UPROPERTY(EditAnywhere, Category="Settings|Camera")
	float CameraSocketOffsetInterpSpeed;

	UPROPERTY(EditAnywhere, Category = "Settings|Camera")
	FCameraModeSettings DefaultCameraModeSettings;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** Holds replicated version of the owning players camera rotation to be used for aim offset */
	UPROPERTY(Replicated)
	FVector_NetQuantize2D QuantizedCameraRotationVector2D;

	UPROPERTY(VisibleInstanceOnly, Category="State")
	ENMovementGait MovementGait;

	UPROPERTY(VisibleInstanceOnly, Category="State")
	ENRotationMode RotationMode;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category="State")
	ENStance Stance;

	FTimerHandle NetTickTimerHandle;
	FVector DesiredCameraSocketOffset;
	ENCombatType CombatType;
	FCameraModeSettings CameraModeSettings;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. References
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** ** Replicated ** Reference to owners CameraSpringArmComponent. */
	UPROPERTY(Replicated)
	UNSpringArmComponent* CameraSpringArmComponent;

	/** ** Replicated ** Reference to owners CombatComponent - Used to calculate aim offset. */
	UPROPERTY(Replicated)
	UNCombatComponent* CombatComponent;

	/** The character that owns this component. */
	UPROPERTY()
	ACharacter* Owner;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Handles Camera offset transitions while tick is active, and draws debug arrows if bDrawDebugArrows is true */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Replicate CombatComponent and QuantizedCameraRotationVector2D to other clients */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** Overridden to call Initialize() */
	virtual void BeginPlay() override;

	/** Overridden to remove CombatComponent->OnTargetActorUpdated delegate binding */
	virtual void BeginDestroy() override;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** Get required references and initialize camera state */
	void Initialize();

	/** [local] Called in Initialize(), sets combat component locally and on the server and binds OnTargetUpdated callback. */
	void SetCombatComponent(UNCombatComponent* InCombatComponent);

public:
	/** Broadcasts whenever MovementGait, RotationMode, CameraMode, or Stance are updated,
	* or when BroadcastSystemState() is called. */
	UPROPERTY(BlueprintAssignable)
	FOnMovementSystemUpdatedDelegate OnSystemUpdated;
	
	/** Returns the current MovementGait. */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	ENMovementGait GetMovementGait() const;
	
	/** Returns the current RotationMode. */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	ENRotationMode GetRotationMode() const;

	/** Returns the current Stance (ENStance) */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	ENStance GetStance() const;
	
	/** [local] Sets the MovementGait if the input is different from the current MovementGait. Broadcasts the change by default,
	* but Broadcasting can be disabled via the input parameter if you want to update multiple System values before
	* Broadcasting. */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	void SetMovementGait(ENMovementGait InMovementMode, bool bBroadcast = true);

	/** [local] Sets the RotationMode if the input is different from the current RotationMode. Broadcasts the change by default,
	* but Broadcasting can be disabled via the input parameter if you want to update multiple System values before
	* Broadcasting. */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	void SetRotationMode(ENRotationMode InRotationMode, bool bBroadcast = true);

	/** [local] Sets the Stance if the input is different from the current Stance. Broadcasts the change by default,
	 * but Broadcasting can be disabled via the input parameter if you want to update multiple System values before
	 * Broadcasting. */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	void SetStance(ENStance InStance, bool bBroadcast = true);

	/** [local] Sets the current combat type. */
	void SetCombatType(ENCombatType InCombatType, bool bBroadcast = true);
	
	/** [local] Broadcasts the current MovementGait, RotationMode, CameraMode, and Stance */
	void BroadcastSystemState() const;
	
	/** [local] Updates the camera settings and sets DesiredCameraSocketOffset to interpolate to. */
	void SetCameraMode(FCameraModeSettings NewCameraState);

	/** [local] Updates the camera settings to the default camera settings and sets DesiredCameraSocketOffset to interpolate to. */
	void SetCameraModeToDefault();

	/** [local] Updates the current camera state base on the current MovementGait and CameraModeSettings. */
	void UpdateCamera();

private:
	/**  Returns the eyes location of the owning character */
	FVector GetEyesLocation() const;

	/** [local] Starts NetUpdateTick() ticking if there is no TargetedActor, Stops ticking if there is */
	UFUNCTION()
	void OnTargetUpdated(UPrimitiveComponent* InTargetActor);

	/** [local] Sets the Owner CharacterMovementComponent bOrientRotationToMovement property */
	void SetOrientRotationToMovement(bool Value);

	/** [local] Called while there is no targeted actor at rate set with AimOffsetNetUpdatesPerSecond. 
	* updates the replicated version of the camera rotation for aim offset with ServerUpdateCameraRotation() */
	void NetUpdateTick();

	/** Draws an arrow in the direction the actor is facing (ForwardActorRotation) */
	void DrawActorForwardRotator(FColor Color, float ZOffset) const;

	/** Draws an arrow in the direction the actor is moving (VelocityRotationOffset) */
	void DrawVelocityRotator(FColor Color, float ZOffset) const;

	/** Draws an arrow in the direction the camera is facing (At its currently lagged position) */
	void DrawCameraForwardRotator(FColor Color, float ZOffset);

	/** Draws an arrow in the direction the player is looking (ForwardActorRotation + LookRotationOffset) */
	void DrawCharacterLookRotator(FColor Color, float ZOffset);

	
	///////////////////////////////////
	/// Functions for AnimBlueprint ///
	///////////////////////////////////
public:
	/** MUST Call this before using getters in AnimBlueprints */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	bool OwnerIsValid() const { return Owner != nullptr; }

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Returns the forward rotation of the owning actor */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	FRotator GetActorForwardRotator() const;

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Returns the velocity rotation of the owning actor */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	FRotator GetVelocityRotator() const;

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Returns the camera rotation of the owning actor (as it exists with lag) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MovementSystem")
	FRotator GetCameraForwardRotator() const;

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Returns the look rotation offset of the owning actor (offset from forward rotation) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="MovementSystem")
	FRotator GetLookRotationOffset() const;

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Returns the velocity rotation offset the owning actor  (offset from forward rotation) */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	FRotator GetVelocityRotationOffset() const;

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Returns the current speed of the owning actor */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	float GetSpeed() const;

	/* MUST Call OwnerIsValid before using in AnimBP
	*  Draws an arrow in the direction of the input Rotator */
	UFUNCTION(BlueprintCallable, Category="MovementSystem")
	void DrawLocomotionDebugArrow(FRotator Rotator, FColor Color, float ZOffset) const;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 6. Server RPC's
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** Updates the replicated camera rotation to replicate the aim offset when not targeting.
	* 	Called in NetUpdateTick(), which is only active while the player is not targeting, at
	* 	at a rate set by AimOffsetNetUpdatesPerSecond. */
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerUpdateCameraRotation(FVector_NetQuantize2D InVector);
	void ServerUpdateCameraRotation_Implementation(FVector_NetQuantize2D InVector);
	bool ServerUpdateCameraRotation_Validate(FVector_NetQuantize2D InVector);

	/** Replicates the CombatComponent to remote players, used for aim offset */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetCombatComponent(UNCombatComponent* InCombatComponent);
	void ServerSetCombatComponent_Implementation(UNCombatComponent* InCombatComponent);
	bool ServerSetCombatComponent_Validate(UNCombatComponent* InCombatComponent);

	/** Sets the players bOrientRotationToMovement to server, necessary for proper root motion gameplay abilities */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetOrientRotationToMovement(bool Value);
	void ServerSetOrientRotationToMovement_Implementation(bool Value);
	bool ServerSetOrientRotationToMovement_Validate(bool Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetStance(ENStance InStance, bool bBroadcast = true);
	void ServerSetStance_Implementation(ENStance InStance, bool bBroadcast = true);
	bool ServerSetStance_Validate(ENStance InStance, bool bBroadcast = true);
};




