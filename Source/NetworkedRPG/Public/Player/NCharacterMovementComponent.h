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

	class FNSavedMove : public FSavedMove_Character
	{
	public:

		typedef  FSavedMove_Character Super;

		// Resets all saved variables
		virtual void Clear() override;

		// Store input commands in the compressed flags
		virtual uint8 GetCompressedFlags() const override;

		// Checks whether or not two moves can be combined into one.
		// Basically just check to see if the saved variables are the same.
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		// Set up the move before sending it to the server.
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		// Sets variables on character movement component before making a predictive correction.
		virtual void PrepMoveFor(ACharacter* C) override;

		// Sprint
		uint8 SavedRequestToStartSprinting : 1;

		// Aim Down sights
		uint8 SavedRequestToStartADS : 1;
	};

	class FNNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
	{
	public:
		FNNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		// Allocates a new copy of our custom saved move
		virtual FSavedMovePtr AllocateNewMove() override;
	};
	
public:
	UNCharacterMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float SprintSpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Down Sights")
	float ADSSpeedMultiplier;

	uint8 RequestToStartSprinting : 1;
	uint8 RequestToStartADS : 1;

	float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	// Sprint
	UFUNCTION(BlueprintCallable, Category = "Sprint")
    void StartSprinting();
	UFUNCTION(BlueprintCallable, Category = "Sprint")
    void StopSprinting();

	// Aim Down Sights
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
    void StartAimDownSights();
	UFUNCTION(BlueprintCallable, Category = "Aim Down Sights")
    void StopAimDownSights();
};
