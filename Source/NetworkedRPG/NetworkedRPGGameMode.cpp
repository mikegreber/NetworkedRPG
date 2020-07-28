// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkedRPGGameMode.h"
#include "NetworkedRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANetworkedRPGGameMode::ANetworkedRPGGameMode()
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	// if (PlayerPawnBPClass.Class != NULL)
	// {
	// 	DefaultPawnClass = PlayerPawnBPClass.Class;
	// }

	
}
