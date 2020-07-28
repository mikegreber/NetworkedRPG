// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetworkedRPGGameMode.generated.h"

UCLASS(minimalapi)
class ANetworkedRPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetworkedRPGGameMode();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PickupItem;
};



