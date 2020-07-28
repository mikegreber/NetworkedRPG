// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkedRPG/Public/AI/NAICharacter.h"
#include "Perception/AIPerceptionComponent.h"

ANAICharacter::ANAICharacter(const FObjectInitializer& ObjectInitializer) : ANCharacterBase(ObjectInitializer)
{
    // AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerception");
}

void ANAICharacter::BeginPlay()
{
    Super::BeginPlay();

    // AIPerceptionComponent->Activate();
    

}

