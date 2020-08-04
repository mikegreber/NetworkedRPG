// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NProgressWidget.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UNProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
}

void UNProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (CurrentValue != TargetValue)
    {
        if (bAnimate)
        {    
            InterpSpeed = MAX_ATTRIBUTE_UPDATES_PER_SECOND * FMath::Abs(TargetValue - CurrentValue);
            CurrentValue = FMath::FInterpConstantTo(CurrentValue, TargetValue, InDeltaTime, InterpSpeed);
        }
        else
        {
            CurrentValue = TargetValue;
        }

        UpdatePercentage(CurrentValue, MaxValue);
    } 
}

void UNProgressWidget::UpdatePercentage(float Current, float Max)
{
    ProgressBar->SetPercent(Current / Max);
    PercentText->SetText(FText::FromString(FString::SanitizeFloat(Current) + " / " + FString::SanitizeFloat(TargetValue)));
}

void UNProgressWidget::SetPercentage(float Current, float Max, float IntSpeed)
{
    if (IntSpeed > 0)
    {
        InterpSpeed = IntSpeed;
    }
    
    SetValue(Current);
    SetMaxValue(Max);
}

void UNProgressWidget::SetValue(float InValue)
{
    TargetValue = InValue;
}

void UNProgressWidget::SetMaxValue(float InMaxValue)
{    
    if (InMaxValue > 0.0f)
    {
        MaxValue = InMaxValue;
    }    
}
