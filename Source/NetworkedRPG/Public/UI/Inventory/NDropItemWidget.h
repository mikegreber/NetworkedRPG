// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Inventory/NInventoryTypes.h"
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. References and State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	UPROPERTY()
	UNInventoryComponent* InventoryComponent;
	FNInventorySlot InventorySlot;
	int32 Max;
	int32 Count;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. Widget Components
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
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

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Interface and Callbacks
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	void InitializeData(const FNInventorySlot& InInventorySlot, UNInventoryComponent* InInventoryComponent);

	UFUNCTION()
	void OnUpClicked();

	UFUNCTION()
	void OnDownClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnConfirmClicked();
};
