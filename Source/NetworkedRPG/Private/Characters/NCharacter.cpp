// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NCharacter.h"
#include "Components/NCharacterMovementComponent.h"
#include "Player/NPlayerController.h"
#include "Player/NPlayerState.h"
#include "NetworkedRPG/NetworkedRPG.h"
#include "Components/NSpringArmComponent.h"
// #include "Components/Targeting/NTargetingComponent.h"
#include "Components/NInteractionComponent.h"
#include "Components/NMovementSystemComponent.h"
#include "Components/Combat/NCombatComponent.h"
#include "AbilitySystem/NAbilitySystemComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ANCharacter::ANCharacter(const FObjectInitializer& ObjectInitializer) : ANCharacterBase(ObjectInitializer)
{
	// Toggle tick on when necessary
	PrimaryActorTick.bCanEverTick = false;
	// PrimaryActorTick.SetTickFunctionEnable(false);
	
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bASCInputBound = false;

	// Create a camera spring arm
	CameraSpringArm = CreateDefaultSubobject<UNSpringArmComponent>(TEXT("CameraBoom"));
	CameraSpringArm->SetupAttachment(RootComponent);
	CameraSpringArm->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraSpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraSpringArm->bEnableCameraLag = true;
	CameraSpringArm->bEnableCameraRotationLag = true;
	CameraSpringArm->SocketOffset = AimingCameraOffset;

	// Set camera default settings
	DefaultFOV = 80.f;

	// Create a camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	Camera->FieldOfView = DefaultFOV;
	
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->SetIsReplicated(true);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Add components
	InteractionComponent = CreateDefaultSubobject<UNInteractionComponent>(TEXT("InteractionComponent"));

	AIPerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AIPerceptionStimuliSource");	
}


void ANCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANCharacter, Camera);
	DOREPLIFETIME(ANCharacter, CameraSpringArm);
}


void ANCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement
	PlayerInputComponent->BindAxis("MoveForward", this, &ANCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ANCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ANCharacter::LookUp);

	// Mechanics
	PlayerInputComponent->BindAction("ToggleStance", IE_Pressed, this, &ANCharacter::ToggleStance);
	PlayerInputComponent->BindAction("ToggleTargetLock", IE_Pressed, this, &ANCharacter::ToggleTargetLock);
	PlayerInputComponent->BindAction("SwitchTargetLeft", IE_Pressed, this, &ANCharacter::SwitchTargetLeft);
	PlayerInputComponent->BindAction("SwitchTargetRight", IE_Pressed, this, &ANCharacter::SwitchTargetRight);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANCharacter::Interact);

	BindASCInput();
}


// Server only
void ANCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ANPlayerState* PS = GetPlayerState<ANPlayerState>();
	if (!PS)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Failed to get PlayerState."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		return;
	}

	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}

	// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
	AbilitySystemComponent = Cast<UNAbilitySystemComponent>(PS->GetAbilitySystemComponent());

	// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for characters that have PlayerController.
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);

	// Set the AttributeSetBase for convenience attribute functions
	AttributeSetBase = PS->GetAttributeSetBase();

	// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession from rejoining doesn't reset attributes
	// For now assume possession = spawn / respawn.
	InitializeAttributes();
	AddStartupEffects();
	AddCharacterAbilities();

	if (ANPlayerController* PC = Cast<ANPlayerController>(GetController()))
	{
		PC->CreateHUD();
	}

	if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Character was dead. Reseting attributes to max."),*FString(__FUNCTION__)), EPrintType::Log);
		}
		
		// Set Health/Mana/Stamina/Shield to their max. This is only necessary for RESPAWN.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());
		SetShield(GetMoveSpeed());

		// Forcibly set the DeadTag count to 0. This is only necessary for RESPAWN.
		AbilitySystemComponent->SetTagMapCount(DeadTag,0);
	}
}


// Client only
void ANCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	ANPlayerState* PS = GetPlayerState<ANPlayerState>();
	if (!PS)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Failed to get PlayerState."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	

	// Set the ASC for clients. Server does this in PossessedBy.
	AbilitySystemComponent = Cast<UNAbilitySystemComponent>(PS->GetAbilitySystemComponent());

	// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);

	// Bind player input to the ASC. Also called in SetupPlayerInputComponent because of potential race condition.
	BindASCInput();

	AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(this, &ANCharacter::OnAbilityActivationFailed);

	// Set the AttributeSetBase for the convenience attribute functions
	AttributeSetBase = PS->GetAttributeSetBase();

	// If we handle players disconnecting and rejoining in the future, we';; have to change this so that possession from rejoining
	// doesn't reset attributes. For now assume possession = spawn or respawn
	InitializeAttributes();

	if (ANPlayerController* PC = Cast<ANPlayerController>(GetController()))
	{
		PC->CreateHUD();
	}

	if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Character was dead. Reseting attributes to max."),*FString(__FUNCTION__)), EPrintType::Log);
		}
		
		// Set Health/Mana/Stamina/Shield to their max. This is only necessary for RESPAWN.
		SetHealth(GetMaxHealth());
		SetMana(GetMaxMana());
		SetStamina(GetMaxStamina());
		SetShield(GetMaxShield());
	}

	// Forcibly set the DeadTag count to 0. This is only necessary for RESPAWN.
	AbilitySystemComponent->SetTagMapCount(DeadTag,  0);
	
}


void ANCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
}


UAbilitySystemComponent* ANCharacter::GetAbilitySystemComponent() const
{
	return Super::GetAbilitySystemComponent();
}


// INCombatComponentInterface version
UNAbilitySystemComponent* ANCharacter::GetNAbilitySystemComponent() const
{
	return Cast<UNAbilitySystemComponent>(Super::GetAbilitySystemComponent());
}


UNMovementSystemComponent* ANCharacter::GetMovementSystemComponent() const
{
	return MovementSystemComponent;
}


USceneComponent* ANCharacter::GetTraceStartComponent() const
{
	return Camera;
}


void ANCharacter::BeginPlay()
{
	Super::BeginPlay();
}


FRotator ANCharacter::GetCameraRotation() const
{
	// TODO Should find more efficient way than replicating camera
	return Camera->GetComponentRotation();
}


void ANCharacter::Die()
{
	Super::Die();
}


void ANCharacter::FinishDying()
{
	Super::FinishDying();
}


void ANCharacter::Interact()
{
	// TODO Should activate this from a Gameplay Ability
	if (IsValid(InteractionComponent))
	{
		InteractionComponent->Interact();
	}
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Error: InteractionComponent not found.", true, true, FColor::Red, 4);
	}
}


void ANCharacter::ToggleTargetLock()
{
	// TODO Should activate this from a Gameplay Ability
	if (IsValid(CombatComponent))
	{
		CombatComponent->ToggleTargetLock();
	}
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Error: TargetingComponent not found.", true, true, FColor::Red, 4);
	}	
}


void ANCharacter::SwitchTargetLeft()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->SwitchTargetToLeft();
    }
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Error: TargetingComponent not found.", true, true, FColor::Red, 4);
	}
}


void ANCharacter::SwitchTargetRight()
{
	if (IsValid(CombatComponent))
	{
		CombatComponent->SwitchTargetToRight();
    }
	else
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Error: TargetingComponent not found.", true, true, FColor::Red, 4);
	}
}


void ANCharacter::ToggleStance()
{
	if (MovementSystemComponent->GetStance() == ENStance::None)
	{
		MovementSystemComponent->SetStance(ENStance::Melee);
	}
	else
	{
		MovementSystemComponent->SetStance(ENStance::None);
	}
}


void ANCharacter::MoveRight(float Value)
{
	if (IsValid(Controller) && Value != 0.0f)
	{
		// get controller rotation
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ANCharacter::MoveForward(float Value)
{
	if (IsValid(Controller) && Value != 0.0f)
	{
		// get controller rotation
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ANCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void ANCharacter::LookUp(float Value)
{

	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void ANCharacter::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
            FString("ConfirmTarget"),
            FString("CancelTarget"),
            FString("ENAbilityInputID"),
            static_cast<int32>(ENAbilityInputID::Confirm),
            static_cast<int32>(ENAbilityInputID::Cancel)));
		
		bASCInputBound = true;

		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s "),*FString(__FUNCTION__)), EPrintType::Success);
		}
	} 
}


void ANCharacter::AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire)
{
	if (!HasAuthority())
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Called on Client. Only call on server."),*FString(__FUNCTION__)), EPrintType::Warning);
		}
		
		return;
	}

	if (!AbilityToAcquire)
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s <TSubclassOf<UGameplayAbility> AbilityToAcquire> was null."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (!IsValid(AbilitySystemComponent))
	{
		if (DebugCharacter)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Invalid AbilitySystemComponent."),*FString(__FUNCTION__)), EPrintType::Error);
		}
		
		return;
	}

	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityToAcquire, 1, 0));
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
}


void ANCharacter::AddGameplayTag(FGameplayTag& TagToAdd, bool bStack) const
{
	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	GetAbilitySystemComponent()->AddLooseGameplayTag(TagToAdd);
	if (!bStack)
	{
		GetAbilitySystemComponent()->SetTagMapCount(TagToAdd, 1);
	}
}


void ANCharacter::RemoveGameplayTag(FGameplayTag& TagToRemove) const
{
	if (DebugCharacter > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"),*FString(__FUNCTION__)), EPrintType::Log);
	}
	
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(TagToRemove);
}


void ANCharacter::OnAbilityActivationFailed(const UGameplayAbility* FailedAbility,
	const FGameplayTagContainer& FailTags)
{
	// TODO
	if (DebugCharacter)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Not Implemented."),*FString(__FUNCTION__)), EPrintType::Error);
	}
}


