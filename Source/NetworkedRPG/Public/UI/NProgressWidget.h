// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


#include "NProgressWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNProgressWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* ProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* PercentText;
	
	// Values for current state
	UPROPERTY()
	float Value;
	
	UPROPERTY()
	float TargetValue;

	UPROPERTY()
	float MaxValue;

	UPROPERTY(EditAnywhere, Category="Default|Animation")
    bool bAnimate;
	
	UPROPERTY(EditAnywhere, Category="Default|Animation")
	bool bConstantInterpolation;
	
	UPROPERTY(EditAnywhere, Category="Default|Animation")
	float UpInterpSpeed;

	UPROPERTY(EditAnywhere, Category="Default|Animation")
	float DownInterpSpeed;

	UPROPERTY()
	float InterpSpeed;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void UpdatePercentage(float Current, float Max);
	
public:

	void SetPercentage(float Current, float Max, float IntSpeed = -1.f);
	
	void SetValue(float InValue);
	
	void SetMaxValue(float InMaxValue);
};
