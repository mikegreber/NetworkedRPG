// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NTypes.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "NTargetingComponent.generated.h"

class INTargetingComponentInterface;
class UNSpringArmComponent;
class UNLocomotionComponent;
class ANCharacter;

class USphereComponent;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetActorUpdated, AActor*, InTargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetUpdated, UPrimitiveComponent*, InTarget);

/** Sections
*	1. Blueprint Settings
*	2. References and State
*	3. Overrides
*	4. Interface and Methods
*	6. Server RPC's
*/

/**
 * ***NOTE: Functionality of this component has been integrated into UNCombatComponent. ***
 *
* UNTargetingComponent allows targeting behavior. For any objects we want to be able to target,
* we must set the objects components Collision Response -> Object Response of 'Targeting' to Overlap.
* Alternatively, Add a NTargetComponent to any actor we want to target for custom placement of a targeting indicator.
* 
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKEDRPG_API UNTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UNMovementSystemComponent;
	
public:
	/** Sets default values for this component's properties */
	UNTargetingComponent();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** The largest angle away from where the controller is facing that will detect TargetActors */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"), Category="TargetingComponent")
	float MaxAngleForTargeting;

	/** The distance a target must be to the owning actor for detection */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "2000.0", UIMin = "0.0", UIMax = "2000.0"), Category="TargetingComponent")
	float MaxTargetDetectionDistance;

	/** The distance a target where a target will be lost */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "2000.0", UIMin = "0.0", UIMax = "2000.0"), Category= "TargetingComponent")
	float MaxTargetLockDistance;

	/** The change in the camera pitch when target locked */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"), Category= "TargetingComponent")
	float TargetingCameraPitchModifier;

	/** The Camera Settings when locked and in walk mode */
	UPROPERTY(EditAnywhere, Category = "TargetingComponent|Camera")
	FCameraMode WalkingCameraSettings;

	/** The Camera Settings when locked and in run mode */
	UPROPERTY(EditAnywhere, Category = "TargetingComponent|Camera")
	FCameraMode RunningCameraSettings;
	
	/** The Camera Settings when locked and in sprint mode */
	UPROPERTY(EditAnywhere, Category = "TargetingComponent|Camera")
	FCameraMode SprintingCameraSettings;
	
	/** The UUserWidget subclass for the visual target indicator widget */ 
	UPROPERTY(EditAnywhere, Category="TargetingComponent")
	TSubclassOf<UUserWidget> TargetIndicatorWidgetClass;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. References and State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** *** Replicated *** Holds the currently locked Target, if there is one. */
	UPROPERTY(Replicated)
	UPrimitiveComponent* Target;

	/** For any object we want to be detected by the targeting component,
	 *we must set the Collision Response -> Object Response to this to Overlap */
	UPROPERTY(VisibleAnywhere, Category = "TargetingComponent")
	TEnumAsByte<ECollisionChannel> CollisionObjectType;

	/** Sphere in which a target must be to be detected. Automatically built in Initialize().*/
	UPROPERTY(VisibleInstanceOnly, Category = "TargetingComponent")
	USphereComponent* TargetingCollisionComponent;
	
	UPROPERTY()
	APlayerController* PlayerController;
	TScriptInterface<INTargetingComponentInterface> Interface;
	UNSpringArmComponent* SpringArm;
	UUserWidget* TargetIndicatorWidget;
	UWidgetComponent* TargetIndicatorWidgetComponent;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Called every frame when TickActorEnabled */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Called when the game starts or an actor owning this component is spawned */
	virtual void BeginPlay() override;
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Returns true if currently target locked */
	UFUNCTION(BlueprintCallable, Category = "TargetingComponent")
	bool IsLocked() const;

	/** Returns the current TargetedActor if there is one */
	UFUNCTION(BlueprintCallable, Category = "TargetingComponent")
	AActor* GetTargetActor() const;
	
	/** Returns the location of the TargetedActor or a zero vector if no target is locked */
	UFUNCTION(BlueprintCallable, Category = "TargetingComponent")
	FVector GetTargetLocation() const;

	//----------------------------------------------------------
	// Actions to bind from actor that owns this component.
	// Could also bind from within this component.
	//----------------------------------------------------------
	
	/** Locks a TargetActor if possible, or turns off Target Locking if a target is already locked  */
	UFUNCTION(BlueprintCallable, Category = "TargetingComponent")
	void ToggleTargetLock();
	/** Moves to the next available target to the right if there is one */
	UFUNCTION(BlueprintCallable, Category = "TargetingComponent")
	void SwitchTargetToRight();
	/** Moves to the next available target to the left if there is one */
	UFUNCTION(BlueprintCallable, Category = "TargetingComponent")
	void SwitchTargetToLeft();

	/** Broadcasts when a new Target is set or Target is set to nullptr */
	FOnTargetUpdated OnTargetUpdated;
	
private:
	/** Creates the objects and references required for this component, MUST call in BeginPlay() */
	void Initialize();
	
	/** [local] Generates the CollisionSphere that will be used to detect Targetable Actors - call in Initialize() */
	void CreateCollisionSphere();

	/** [local] Generates the TargetWidget - call in Initialize() */
	void CreateTargetWidget();

	/** [local] Called whenever there is an update to the MovementSystem state */
	UFUNCTION()
	void OnMovementSystemUpdated(ENMovementGait InMovementMode, ENRotationMode InRotationMode, ENStance InStance);

	/** Sets the new targeted actor and broadcasts */
	void SetTarget(UPrimitiveComponent* InTarget);

	/** [local + sever] Sets lock on input TargetToLock */
	void Lock(UPrimitiveComponent* TargetToLock);

	/** [local + sever] Removes target lock */
	void UnLock();

	/** [local] Finds all targets within the targeting sphere (within MaxTargetDetectionDistance) and within the MaxAngleForTargeting */
	TArray<UPrimitiveComponent*> GetAvailableTargets() const;
	
	/** [local] Finds the actor with the closest angle to the player based on the input predicate
	 * By with no Predicate argument, locks the target that required the smallest turn to target
	 * For use in both Initial Targeting and Switching Targets
	 * Returns true if a new target is found. */
	bool FindSortedTarget(TFunctionRef<bool(float AngleToTurn, float ClosestAngele)> Predicate =
		[](float AngleToTurn, float ClosestAngele){return FMath::Abs(AngleToTurn) < FMath::Abs(ClosestAngele);});
	
	/* Returns the actor location of the actor that owns this component. */
	FVector GetOwnerLocation() const;

	/* Returns control rotation of the PlayerController that owns the actor that owns this component. */
	FRotator GetControlRotation() const;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Server RPC's
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** Adds target lock - Needed to keep TargetActor reference properly replicated */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLock(UPrimitiveComponent* TargetToLock);
	void ServerLock_Implementation(UPrimitiveComponent* TargetToLock);
	bool ServerLock_Validate(UPrimitiveComponent* TargetToLock);

	/** Removes target lock - Needed to keep TargetActor reference properly replicated */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUnLock();
	void ServerUnLock_Implementation();
	bool ServerUnLock_Validate();
};
