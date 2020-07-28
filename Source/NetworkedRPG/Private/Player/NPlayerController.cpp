// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/NCharacter.h"
#include "UI/NHUDWidget.h"
#include "Player/NPlayerState.h"
#include "UI/NInventoryWidget.h"


ANPlayerController::ANPlayerController()
{
    InventoryComponent = CreateDefaultSubobject<UNInventoryComponent>("Inventory");
}

void ANPlayerController::BeginPlay()
{
    Super::BeginPlay();

    CreateHUD();
}

void ANPlayerController::CreateHUD()
{
    if (UIHUDWidget || !IsLocalPlayerController())
    {
        return;
    }

    if (!UIHUDWidgetClass)
    {
        UKismetSystemLibrary::PrintString(GetWorld(), "ANPlayerController::CreateHUD() missing UIHUDWidgetClass. Please fill in on the blueprint for the player controller", true, true, FColor::Red);
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

void ANPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
}

UNHUDWidget* ANPlayerController::GetNHUD() const
{
    return UIHUDWidget; 
}

void ANPlayerController::OnPossess(APawn* aPawn)
{
    Super::OnPossess(aPawn);

    if (ANPlayerState* PS = GetPlayerState<ANPlayerState>())
    {
        PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, aPawn);
    }
}
