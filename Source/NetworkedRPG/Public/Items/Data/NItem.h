// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NItem.generated.h"

class UNGameplayAbility;

USTRUCT(BlueprintType)
struct NETWORKEDRPG_API FItemAbilityInfo
{
	GENERATED_USTRUCT_BODY()

	FItemAbilityInfo():
		AbilityLevel(0)
	{}

	/** Ability to grant if this item is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UNGameplayAbility> Ability;

	/** Ability level. <= 0 means the character level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AbilityLevel;
};


/**
 * Base class for all items in the game. Should use this or subclass for all Items.
 * Should spawn appropriate actors from the data here if the item has a physical representation.
 */
UCLASS()
class NETWORKEDRPG_API UNItem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UNItem():
		StackMaxCount(1)
	{}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Type of this item, set in native parent class */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FPrimaryAssetType ItemType;

	/** User-visible short name. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FString ItemName;

	/** User-visible long description. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemDescription;

	/** The icon that will be visible in the inventory. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UTexture2D* InventoryIcon;

	/** The skeletal mesh of this object, if it has one. */
	UPROPERTY(EditAnywhere, Category = "Item")
	USkeletalMesh* ItemMesh;
	
	/** Max number of this item that can be in a stack, < 0 means infinite, 0 means this item is instantly activated upon pickup. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Max")
	int32 StackMaxCount;

	/** The max level this item can be, <= 0 means infinite. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Max")
	int32 MaxLevel;

	/** Abilities to grant if this item is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Abilities")
	TArray<FItemAbilityInfo> GrantedAbilities;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Gives warning when ItemType is not set on save */
	virtual void PreSave(const ITargetPlatform* TargetPlatform) override;
	
	/** Overriden to use saved type */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Max count < 0 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsConsumable() const;

	/** Max count > 1 */
	bool CanStack() const;

	/** Returns the logical name, equivalent to the primary asset id*/
	UFUNCTION(BlueprintCallable, Category = "Item")
	FString GetIdentifierString() const;

protected:
	/** Displays a warning in the editor. Use to warn if item is not valid when saved. */
	void ShowWarning(FString WarningString) const;
};

