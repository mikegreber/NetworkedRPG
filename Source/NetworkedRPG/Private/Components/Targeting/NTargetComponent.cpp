// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Targeting/NTargetComponent.h"
#include "NetworkedRPG/NetworkedRPG.h"

UNTargetComponent::UNTargetComponent()
{
    BoxExtent = FVector(1.f);
    SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SetCollisionObjectType(ECC_Target);
    SetCollisionResponseToAllChannels(ECR_Ignore);
    SetCollisionResponseToChannel(ECC_Targeting, ECR_Overlap);
}
