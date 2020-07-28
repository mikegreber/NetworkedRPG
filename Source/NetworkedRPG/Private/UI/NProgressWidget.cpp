// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NProgressWidget.h"


#include "NetworkedRPG/NetworkedRPG.h"
#include "Player/NPlayerState.h"

void UNProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Value = 0.f;
    // TargetValue = 1.f;
    // MaxValue = 1.f;
    // bConstantInterpolation = false;
    // UpInterpSpeed = 10.f;
    // DownInterpSpeed = 10.f;   
}

void UNProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (Value != TargetValue)
    {
        if (bAnimate)
        {    
            InterpSpeed = MAX_ATTRIBUTE_UPDATES_PER_SECOND * FMath::Abs(TargetValue - Value);
            Value = FMath::FInterpConstantTo(Value, TargetValue, InDeltaTime, InterpSpeed);
        }
        else
        {
            Value = TargetValue;
        }

        UpdatePercentage(Value, MaxValue);
    } 
}

void UNProgressWidget::UpdatePercentage(float Current, float Max)
{
    ProgressBar->SetPercent(Current / Max);
    PercentText->SetText(FText::FromString(FString::SanitizeFloat(Current) + " / " + FString::SanitizeFloat(TargetValue)));
}

void UNProgressWidget::SetPercentage(float Current, float Max, float IntSpeed)
{
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
