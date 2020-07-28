// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Equipment/NWeaponActor.h"
#include "NMeleeWeaponActor.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API ANMeleeWeaponActor : public ANWeaponActor
{
	GENERATED_BODY()

public:
    virtual void Initialize(FWeaponActorData InData) override;

};
