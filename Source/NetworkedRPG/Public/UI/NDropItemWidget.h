// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NInventoryTypes.h"
#include "Blueprint/UserWidget.h"
#include "NDropItemWidget.generated.h"

class UButton;
class UTextBlock;
class UNInventoryComponent;

/**
 * 
 */
UCLASS()
class NETWORKEDRPG_API UNDropItemWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY()
	UNInventoryComponent* InventoryComponent;
	FNInventorySlot InventorySlot;
	int32 Max;
	int32 Count;

public:

	void Initialize(const FNInventorySlot& InInventorySlot, UNInventoryComponent* InInventoryComponent);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountText;

	UPROPERTY(meta = (BindWidget))
	UButton* UpButton;

	UPROPERTY(meta = (BindWidget))
	UButton* DownButton;

	UPROPERTY(meta = (BindWidget))
    UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
    UButton* ConfirmButton;

	UFUNCTION()
	void OnUpClicked();

	UFUNCTION()
	void OnDownClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnConfirmClicked();
};
