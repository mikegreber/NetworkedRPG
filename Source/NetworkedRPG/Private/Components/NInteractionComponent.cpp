// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NInteractionComponent.h"
#include "Interface/NInteractableInterface.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"


FAutoConsoleVariableRef CVarDebugInteractionComponent(
    TEXT("NRPG.Debug.InteractionComponent"),
    DebugInteractionComponent,
    TEXT("Show debug information for InteractionComponent: 0 - Off, 1 - Low, 2 - High"),
    ECVF_Cheat
    );


// Sets default values for this component's properties
UNInteractionComponent::UNInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false); // Will toggle

	// Set defaults
	LineTraceTickRate = 0.3;
	LineTraceDistance = 300.f;
	
	InteractableIndicatorWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("IndicatorWidget"));
	InteractableIndicatorWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractableIndicatorWidgetComponent->SetHiddenInGame(false);
}

// Called every frame while ComponentTick is enabled
void UNInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	IndicatorAnimationTimeline.TickTimeline(DeltaTime);	
}


// Called when the game starts or this component is spawned
void UNInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNInteractionComponent::Initialize);
}

void UNInteractionComponent::Interact()
{
	// Only execute on the server
	if (GetOwnerRole() != ROLE_Authority)
	{
		ServerInteract();
		return;
	}

	if (DebugInteractionComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
	}

	// Call Interact if trace hits an Interactable
	if (TScriptInterface<INInteractableInterface> Interactable = TraceForInteractable())
	{
		if (Interactable && GetOwner())
		{
			Interactable->Interact(GetOwner());
		}
	}
}


void UNInteractionComponent::Initialize()
{
	// UI Setup, so only execute on local player controllers
	APlayerController* PC = GetOwner()->GetInstigatorController<APlayerController>();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (DebugInteractionComponent)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
	}

	// Setup the widget
	if (InteractableIndicatorWidgetClass)
	{
		InteractableIndicatorWidgetComponent->SetWidgetClass(InteractableIndicatorWidgetClass);
		InteractableIndicatorWidgetComponent->SetOwnerPlayer(GetOwner()->GetInstigatorController<APlayerController>()->GetLocalPlayer());
		InteractableIndicatorWidgetComponent->GetUserWidgetObject()->SetRenderOpacity(0.f);
	}
	else
	{
		Print(GetWorld(), FString::Printf(TEXT("%s InteractableIndicatorWidgetClass not set in Blueprint."), *FString(__FUNCTION__)), EPrintType::Failure);
		return;
	}

	// Setup the animation
	if (AnimationCurve && AnimationEndHeight > 0.f && InteractableIndicatorWidgetClass)
	{	
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindDynamic(this, &UNInteractionComponent::HandleTimelineAnimation);

		FOnTimelineEvent OnFinished;
		OnFinished.BindDynamic(this, &UNInteractionComponent::OnTimelineFinish);

		IndicatorAnimationTimeline.SetTimelineLength(AnimationDuration);
		IndicatorAnimationTimeline.AddInterpFloat(AnimationCurve, ProgressFunction);
		IndicatorAnimationTimeline.SetTimelineFinishedFunc(OnFinished);
	}
	else
	{
		Print(GetWorld(), FString::Printf(TEXT("%s Indicator animation not set up - missing variables in Blueprint."), *FString(__FUNCTION__)), EPrintType::Failure);
	}

	// Start the Line Trace 'Tick' 
	GetWorld()->GetTimerManager().SetTimer(InteractableTraceTickTimerHandle, this, &UNInteractionComponent::InteractableTraceTick, LineTraceTickRate, true, 2.f);

	
}


void UNInteractionComponent::InteractableTraceTick()
{
	AActor* Hit = TraceForInteractable();
	if (CurrentInteractable != Hit)
	{
		if (DebugInteractionComponent)
		{
			Print(GetWorld(), FString::Printf(TEXT("%s Trace %s."), *FString(__FUNCTION__), *FString(Hit ? "hit new actor" : "none")), EPrintType::Log);
		}
		
		SetCurrentInteractable(Hit);
	}	
}


AActor* UNInteractionComponent::TraceForInteractable() const
{
	const FVector TraceStart = GetOwner()->GetActorLocation();
	const FVector TraceEnd = TraceStart + GetOwner()->GetActorForwardVector() * LineTraceDistance;

	FHitResult OutHit;
	GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Interactable);

	return OutHit.GetActor();
}


void UNInteractionComponent::SetCurrentInteractable(AActor* InInteractable)
{	
	CurrentInteractable = InInteractable;
	const bool bShowIndicator = CurrentInteractable != nullptr;
	
	if (bShowIndicator)
	{
		INInteractableInterface* Interactable = Cast<INInteractableInterface>(CurrentInteractable);
		if (Interactable)
		{
			InteractableIndicatorWidgetComponent->AttachToComponent(Interactable->GetInteractableRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			IndicatorOffset = Interactable->GetIndicatorOffset();
		}

		IndicatorAnimationTimeline.PlayFromStart();
	}
	
	InteractableIndicatorWidgetComponent->SetHiddenInGame(!bShowIndicator);

	SetComponentTickEnabled(bShowIndicator);
}


void UNInteractionComponent::HandleTimelineAnimation(float Value)
{
	const FVector NewLocation = FMath::Lerp(IndicatorOffset, IndicatorOffset + FVector(0.f,0.f, AnimationEndHeight), Value);
	InteractableIndicatorWidgetComponent->SetRelativeLocation(NewLocation);
	InteractableIndicatorWidgetComponent->GetUserWidgetObject()->SetRenderOpacity(Value);
}


void UNInteractionComponent::OnTimelineFinish()
{
	if (DebugInteractionComponent > 1)
	{
		Print(GetWorld(), FString::Printf(TEXT("%s"), *FString(__FUNCTION__)), EPrintType::Log);
	}
	
	SetComponentTickEnabled(false);
}


void UNInteractionComponent::ServerInteract_Implementation()
{
	Interact();
}


bool UNInteractionComponent::ServerInteract_Validate()
{
	return true;
}