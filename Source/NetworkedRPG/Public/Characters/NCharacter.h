// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NCharacterBase.h"
#include "Components/Combat/NCombatComponentInterface.h"
#include "NCharacter.generated.h"

class UCameraComponent;
class UWidgetComponent;
class UNSpringArmComponent;
class UNTargetingComponent;
class UNInteractionComponent;
class UAIPerceptionStimuliSourceComponent;
class UGameplayAbility;

/** Sections
*	1. Blueprint Settings
*	2. Components
*	3. State
*	4. Overrides
*	5. Interface and Methods
*/

/**
 * Class to use for player controlled characters. 
 */
UCLASS()
class NETWORKEDRPG_API ANCharacter : public ANCharacterBase, public INCombatComponentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANCharacter(const FObjectInitializer& ObjectInitializer);

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Camera")
	float BaseTurnRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Camera")
	float BaseLookUpRate;
	
	UPROPERTY(EditAnywhere, Category = "Settings|Camera")
	FVector AdditionalSocketOffset;
	
	UPROPERTY(BlueprintReadOnly, Category="Settings|Camera")
	float DefaultFOV;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Components
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	UNInteractionComponent* InteractionComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components")
	UAIPerceptionStimuliSourceComponent* AIPerceptionStimuliSourceComponent;

	/** ** Replicated** */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UNSpringArmComponent* CameraSpringArm;

	/** ** Replicated** */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	bool bASCInputBound;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Replicates camera and spring arm. */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;

	// Client only
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;

	// For AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/* INCombatComponentInterface */
	virtual UNAbilitySystemComponent* GetNAbilitySystemComponent() const override;
	virtual UNMovementSystemComponent* GetMovementSystemComponent() const override;
	
	/** INGameplayAbilityActorInterface */
	virtual USceneComponent* GetTraceStartComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Returns the current camera rotation. */
	FRotator GetCameraRotation() const;

	/** Call from player state when health = 0 */
	virtual void Die() override;

protected:
	/** Called after dying animation */
	virtual void FinishDying() override;
	
	/** Input bindings */
	void Interact();
	void ToggleTargetLock();
	void SwitchTargetLeft();
	void SwitchTargetRight();
	void ToggleStance();
	void MoveRight(float Value);
	void MoveForward(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	/** Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition where the PlayerController might call
	  * ClientRestart which calls SetupPlayerInputComponent before the PlayerState is repped to the client so the PlayerState would be null in
	  * SetupPlayerInputComponent. Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart so the Actor's InputComponent
	  * would be null in OnRep_PlayerState. */
	void BindASCInput();

	/** [server] Grants the input ability to the character. */
	UFUNCTION(BlueprintCallable, Category = "CharacterAbilities")
	void AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire);

	/** Adds a loose gameplay tag. Sets tag count to 1 if bStack is false. ** Does Not Replicate Tag ** */
	UFUNCTION(BlueprintCallable, Category = "CharacterAbilities")
	void AddGameplayTag(FGameplayTag& TagToAdd, bool bStack = false) const;

	/** Removed a loose gameplay tag. ** Does Not Replicate ** */
	UFUNCTION(BlueprintCallable, Category = "CharacterAbilities")
	void RemoveGameplayTag(FGameplayTag& TagToRemove) const;

	/** Not implemented */
	void OnAbilityActivationFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& FailTags);
};
