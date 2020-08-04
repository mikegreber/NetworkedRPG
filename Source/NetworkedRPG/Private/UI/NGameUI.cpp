// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NGameUI.h"
#include "Characters/NCharacter.h"
#include "Components/ProgressBar.h"


void UNGameUI::UpdateHealth(float Current, float Max)
{
    UE_LOG(LogTemp, Warning, TEXT("Ouch!! %f"), Current / Max)
    HealthBar->SetPercent(Current / Max);
}
