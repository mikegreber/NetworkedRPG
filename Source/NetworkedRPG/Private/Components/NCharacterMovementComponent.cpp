// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NCharacterMovementComponent.h"
#include "Characters/NCharacterBase.h"

UNCharacterMovementComponent::UNCharacterMovementComponent()
{
    SprintSpeedMultiplier = 1.4f;
    AimSpeedMultiplier = 0.8f;
}


float UNCharacterMovementComponent::GetMaxSpeed() const
{
    ANCharacterBase* Owner = Cast<ANCharacterBase>(GetOwner());
    if (!Owner)
    {
        Print(GetWorld(), FString::Printf(TEXT("%s Component only works when added to ANCharacterBase Actors."), *FString(__FUNCTION__)), EPrintType::Error);
        return Super::GetMaxSpeed();
    }

    if (!Owner->IsAlive())
    {
        return 0.0f;
    }

    if (RequestToStartSprinting)
    {
        return Owner->GetMoveSpeed() * SprintSpeedMultiplier;
    }

    if (RequestToStartAiming)
    {
        return Owner->GetMoveSpeed() * AimSpeedMultiplier;
    }

    return Owner->GetMoveSpeed();
}


void UNCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    // The Flags parameter contains the compressed input flags that are stored in the save move.
    // UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
    // It basically just resets the movement component to the state when the move was made so it can simulate from there.
    RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

    RequestToStartAiming = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}


FNetworkPredictionData_Client* UNCharacterMovementComponent::GetPredictionData_Client() const
{
    check(PawnOwner);

    if (!ClientPredictionData)
    {
        UNCharacterMovementComponent* MutableThis = const_cast<UNCharacterMovementComponent*>(this);

        MutableThis->ClientPredictionData = new FNNetworkPredictionData_Client(*this);
        MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
        MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
    }

    return ClientPredictionData;
}


void UNCharacterMovementComponent::FNSavedMove::Clear()
{
    Super::Clear();

    SavedRequestToStartSprinting = false;
    SavedRequestToStartAiming = false;
}


uint8 UNCharacterMovementComponent::FNSavedMove::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    if (SavedRequestToStartSprinting)
    {
        Result |= FLAG_Custom_0;
    }

    if (SavedRequestToStartAiming)
    {
        Result |= FLAG_Custom_1;
    }

    return Result;
}


bool UNCharacterMovementComponent::FNSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
    // if (SavedRequestToStartSprinting != (Cast<FNSavedMove>(&NewMove))->SavedRequestToStartSprinting)
    if (SavedRequestToStartSprinting != ((FNSavedMove*)&NewMove)->SavedRequestToStartSprinting)
    {
        return false;
    }

    // if (SavedRequestToStartAiming != (Cast<FNSavedMove>(&NewMove))->SavedRequestToStartAiming)
    if (SavedRequestToStartAiming != ((FNSavedMove*)&NewMove)->SavedRequestToStartAiming)
    {
        return false;
    }

    return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UNCharacterMovementComponent::FNSavedMove::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

    UNCharacterMovementComponent* CharacterMovement = Cast<UNCharacterMovementComponent>(C->GetCharacterMovement());
    if (CharacterMovement)
    {
        SavedRequestToStartSprinting = CharacterMovement->RequestToStartSprinting;
        SavedRequestToStartAiming = CharacterMovement->RequestToStartAiming;
    }
}

// void UNCharacterMovementComponent::FNSavedMove::PrepMoveFor(ACharacter* C)
// {
//     Super::PrepMoveFor(C);
//
//     UNCharacterMovementComponent* CharacterMovement = Cast<UNCharacterMovementComponent>(C->GetMovementComponent());
//     if (CharacterMovement)
//     {
//     }
// }

UNCharacterMovementComponent::FNNetworkPredictionData_Client::FNNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}


FSavedMovePtr UNCharacterMovementComponent::FNNetworkPredictionData_Client::AllocateNewMove()
{
    return MakeShared<FNSavedMove>();
    // return FSavedMovePtr(new FNSavedMove());
}


void UNCharacterMovementComponent::StartSprinting()
{
    RequestToStartSprinting = true;
}


void UNCharacterMovementComponent::StopSprinting()
{
    RequestToStartSprinting = false;
}


void UNCharacterMovementComponent::StartAiming()
{
    RequestToStartAiming = true;
}


void UNCharacterMovementComponent::StopAiming()
{
    RequestToStartAiming = false;
}

