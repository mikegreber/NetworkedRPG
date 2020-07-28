// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NCharacterBase.h"
#include "NAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API ANAICharacter : public ANCharacterBase
{
	GENERATED_BODY()


public:
	// ANAICharacter();


	explicit ANAICharacter(const FObjectInitializer& ObjectInitializer);


protected:
	virtual void BeginPlay() override;
};
