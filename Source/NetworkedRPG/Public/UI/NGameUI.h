// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Player/NCharacter.h"

#include "NGameUI.generated.h"

class UProgressBar;

// #define PROGRESS_SETTER(ThisClass, ClassName, PropertyName) \
// void ThisClass::Update##PropertyName(float Current, float Max) const \
// { \
// 	if (ClassName* Character = Cast<ClassName>(GetOwningPlayer())) \
// 	{ \
// 		##PropertyName##Bar->Percent = Character->Get##PropertyName() / Character->GetMax##PropertyName(); \
// 	} \
// }

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNGameUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

	UPROPERTY(meta = (BindWidget))
    UProgressBar* StaminaBar;

	void UpdateHealth(float Current, float Max);
	//
	// PROGRESS_SETTER(UNGameUI, ANCharacter, Health);
	// PROGRESS_SETTER(UNGameUI, ANCharacter, Mana);
	// PROGRESS_SETTER(UNGameUI, ANCharacter, Stamina);
	//
};
