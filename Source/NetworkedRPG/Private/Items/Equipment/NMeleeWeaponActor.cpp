// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Equipment/NMeleeWeaponActor.h"
#include "GameFramework/Character.h"
#include "Characters/Components/NCombatComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void ANMeleeWeaponActor::Initialize(FWeaponActorData InData)
{
    Super::Initialize(InData);

    if (InData)
    {
        WeaponMesh->SetGenerateOverlapEvents(true);
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        WeaponMesh->SetCollisionObjectType(ECC_Weapon);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Overlap);

        if (CombatComponent)
        {
            WeaponMesh->OnComponentBeginOverlap.AddDynamic(CombatComponent, &UNCombatComponent::OnWeaponCollision);
        }        
    }
    
     
}
