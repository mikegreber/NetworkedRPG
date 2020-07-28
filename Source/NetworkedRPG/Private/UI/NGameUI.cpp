// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NGameUI.h"
#include "Player/NCharacter.h"
#include "Components/ProgressBar.h"

void UNGameUI::NativeConstruct()
{
    
}

void UNGameUI::UpdateHealth(float Current, float Max)
{
    UE_LOG(LogTemp, Warning, TEXT("Ouch!! %f"), Current / Max)
    // HealthBar->Percent = Current / Max;
    HealthBar->SetPercent(Current / Max);
}
