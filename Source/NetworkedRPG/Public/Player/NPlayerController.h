// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NPlayerController.generated.h"

class UNHUDWidget;
class UNInventoryComponent;
class UNInventoryWidget;
/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API ANPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANPlayerController();

	void BeginPlay() override;

	void CreateHUD();


protected:
	virtual void SetupInputComponent() override;
public:
	UNHUDWidget* GetNHUD() const;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|CombatComponent")
	UNInventoryComponent* InventoryComponent;
	
protected:
	UNHUDWidget* UIHUDWidget;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UNHUDWidget> UIHUDWidgetClass;
	
	virtual void OnPossess(APawn* aPawn) override;
	
};
