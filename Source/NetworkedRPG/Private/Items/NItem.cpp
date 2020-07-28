// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/NItem.h"
#include "Editor.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "NAssetManager.h"
#include "Framework/Notifications/NotificationManager.h"

void UNItem::PreSave(const ITargetPlatform* TargetPlatform)
{
    Super::PreSave(TargetPlatform);

    if (ItemType == "None")
    {
        ShowWarning("ItemType not set!");
    }
}

bool UNItem::IsConsumable() const
{
    return StackMaxCount < 0;
}

bool UNItem::CanStack() const
{
    return StackMaxCount > 1;
}

FString UNItem::GetIdentifierString() const
{
    return GetPrimaryAssetId().ToString();
}

FPrimaryAssetId UNItem::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(ItemType, GetFName());
}

void UNItem::ShowWarning(FString WarningString) const
{
    FNotificationInfo Info = FNotificationInfo( FText::FromString(WarningString) );
    Info.FadeInDuration = 0.1f;
    Info.FadeOutDuration = 0.5f;
    Info.ExpireDuration = 5.0f;
    Info.bUseThrobber = false;
    Info.bUseSuccessFailIcons = true;
    Info.bUseLargeFont = true;
    Info.bFireAndForget = false;
    Info.bAllowThrottleWhenFrameRateIsLow = false;
    auto NotificationItem = FSlateNotificationManager::Get().AddNotification( Info );
    NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
    NotificationItem->ExpireAndFadeout();
}
