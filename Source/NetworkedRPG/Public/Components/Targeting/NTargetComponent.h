// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "NTargetComponent.generated.h"

/**
 * Component that can be added to any actor we want to be able to target (Will be detected by targeting component).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKEDRPG_API UNTargetComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UNTargetComponent();
};
