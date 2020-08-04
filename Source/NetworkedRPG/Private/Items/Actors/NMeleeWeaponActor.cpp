// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Actors/NMeleeWeaponActor.h"
#include "Components/Combat/NCombatComponent.h"
#include "GameFramework/Character.h"
#include "Items/Actors/NWeaponActor.h"

void ANMeleeWeaponActor::Initialize(FWeaponActorData InData)
{
    Super::Initialize(InData);

    if (InData)
    {
        Mesh->SetGenerateOverlapEvents(true);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Mesh->SetCollisionObjectType(ECC_Weapon);
        Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);

        if (CombatComponent)
        {
            Mesh->OnComponentBeginOverlap.AddDynamic(CombatComponent, &UNCombatComponent::OnWeaponCollision);
        }        
    }  
}

void ANMeleeWeaponActor::BeginDestroy()
{   
    if (CombatComponent)
    {
        Mesh->OnComponentBeginOverlap.RemoveDynamic(CombatComponent, &UNCombatComponent::OnWeaponCollision);
    }

    Super::BeginDestroy();
}
