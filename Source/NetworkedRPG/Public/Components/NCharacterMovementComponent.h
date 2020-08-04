// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UNCharacterMovementComponent();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float SprintSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float AimSpeedMultiplier;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2.State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Sprint request Flag */
	uint8 RequestToStartSprinting : 1;

	/** Aim request Flag */
	uint8 RequestToStartAiming : 1;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Returns max speed or modified speed if sprinting or aiming. */
	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	class FNSavedMove : public FSavedMove_Character
	{
	public:
		typedef  FSavedMove_Character Super;

		////////////////
		// State
		////////////////
	public:
		/** Sprint Flag */
		uint8 SavedRequestToStartSprinting : 1;

		/** Aim Flag*/
		uint8 SavedRequestToStartAiming : 1;

		////////////////
		// Overrides
		////////////////
	public:
		/** Resets all saved variables. */
		virtual void Clear() override;

		/** Store input commands in the compressed flags. */
		virtual uint8 GetCompressedFlags() const override;

		/** Checks whether or not two moves can be combined into one.
		  * Basically just check to see if the saved variables are the same. */
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		/** Set up the move before sending it to the server. */
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		// /** Sets variables on character movement component before making a predictive correction. */
		// virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
	{
	public:
		FNNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		////////////////
		// Overrides
		////////////////
		
		/** Allocates a new copy of our custom saved move. */
		virtual FSavedMovePtr AllocateNewMove() override;
	};

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StartSprinting();
	
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	void StopSprinting();

	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StartAiming();
	
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
	void StopAiming();
};
