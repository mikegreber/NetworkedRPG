// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NPlayerController.h"
#include "Components/Inventory/NInventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Characters/NCharacter.h"
#include "UI/NHUDWidget.h"
#include "Player/NPlayerState.h"
#include "UI/Inventory/NInventoryWidget.h"


ANPlayerController::ANPlayerController()
{
    InventoryComponent = CreateDefaultSubobject<UNInventoryComponent>("Inventory");
}

void ANPlayerController::BeginPlay()
{
    Super::BeginPlay();

    CreateHUD();
}


void ANPlayerController::OnPossess(APawn* NewPawn)
{
    Super::OnPossess(NewPawn);

    if (ANPlayerState* PS = GetPlayerState<ANPlayerState>())
    {
        PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, NewPawn);
    }
}


void ANPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
}


void ANPlayerController::CreateHUD()
{
    if (UIHUDWidget || !IsLocalPlayerController())
    {
        return;
    }

    if (!UIHUDWidgetClass)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s UIHUDWidgetClass not set. Please set in Blueprint."), *FString(__FUNCTION__)), EPrintType::Error);
        return;
    }

    if (ANPlayerState* PS = GetPlayerState<ANPlayerState>())
    {
        UIHUDWidget = CreateWidget<UNHUDWidget>(this, UIHUDWidgetClass);
        UIHUDWidget->AddToViewport();

        UIHUDWidget->SetMaxHealth(PS->GetMaxHealth());
        UIHUDWidget->SetHealth(PS->GetHealth());

        UIHUDWidget->SetMaxMana(PS->GetMaxMana());
        UIHUDWidget->SetMana(PS->GetMana());
    
        UIHUDWidget->SetMaxStamina(PS->GetMaxStamina());
        UIHUDWidget->SetStamina(PS->GetStamina());
    
        UIHUDWidget->SetMaxShield(PS->GetMaxShield());
        UIHUDWidget->SetShield(PS->GetShield());
    }
}


UNHUDWidget* ANPlayerController::GetHUDWidget() const
{
    return UIHUDWidget; 
}



