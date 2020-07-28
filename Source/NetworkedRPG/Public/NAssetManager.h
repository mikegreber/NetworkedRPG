// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "NAssetManager.generated.h"

class UNItem;

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	UNAssetManager() {}
	virtual void StartInitialLoading() override;

	/** Static types for items */
	static const FPrimaryAssetType WeaponItemType;
	static const FPrimaryAssetType ArmourItemType;
	static const FPrimaryAssetType ConsumableItemType;


	/** Returns the current AssetManager object */
	static UNAssetManager& Get();

	UNItem* ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning = true);
	
};
