// Fill out your copyright notice in the Description page of Project Settings.


#include "NAssetManager.h"
#include "Items/Data/NItem.h"
#include "AbilitySystemGlobals.h"

const FPrimaryAssetType UNAssetManager::WeaponItemType = TEXT("WeaponItem");
const FPrimaryAssetType UNAssetManager::ArmourItemType = TEXT("ArmourItem");
const FPrimaryAssetType UNAssetManager::ConsumableItemType = TEXT("ConsumableItem");

void UNAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();

    UAbilitySystemGlobals::Get().InitGlobalData();
}

UNAssetManager& UNAssetManager::Get()
{
    UNAssetManager* This = Cast<UNAssetManager>(GEngine->AssetManager);
    if (This)
    {
        return *This;
    }
    else
    {
        return *NewObject<UNAssetManager>();
    }
}

UNItem* UNAssetManager::ForceLoadItem(const FPrimaryAssetId& PrimaryAssetId, bool bLogWarning)
{
    FSoftObjectPath ItemPath = GetPrimaryAssetPath(PrimaryAssetId);

    UNItem* LoadedItem = Cast<UNItem>(ItemPath.TryLoad());

    if (bLogWarning && LoadedItem == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load item for identifier %s!"), *PrimaryAssetId.ToString());
    }

    return LoadedItem;
}
