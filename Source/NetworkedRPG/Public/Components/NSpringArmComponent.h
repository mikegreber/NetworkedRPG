// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "NSpringArmComponent.generated.h"

/**
 *  Subclass of USpringArmComponent with getter for to RelativeSocketRotation.
 */
UCLASS()
class NETWORKEDRPG_API UNSpringArmComponent : public USpringArmComponent
{
	GENERATED_BODY()

public:
	/** Returns the RelativeSocketRotation at the current lagged camera position */
	FQuat GetRelativeSocketRotation() const { return RelativeSocketRotation; }
};
