// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "Interface/NInteractableInterface.h"
#include "NInteractionComponent.generated.h"

int32 DebugInteractionComponent = 0;

class UWidgetComponent;

/** Sections
*	1. Blueprint Settings
*	2. References and State
*	3. Overrides
*	4. Interface and Methods
*	6. Server RPC's
*/

/**
 * Interaction component displays a visual cue (InteractableIndicatorWidgetClass) when an object can be interacted with.
 * Any object we wish to interact with must must have Collision Response -> TraceResponse -> Interactable (ECC_Interactable) set to Block,
 * and implement INInteractableInterface. Actual interaction only happens on the server.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKEDRPG_API UNInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Sets default values for this component's properties */
	UNInteractionComponent();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 1. Blueprint Settings
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	/** The rate of line traces */
	UPROPERTY(EditAnywhere, Category = "InteractionComponent")
	float LineTraceTickRate;

	/** How far the line trace goes in detecting interactable objects */
	UPROPERTY(EditAnywhere, Category = "InteractionComponent")
	float LineTraceDistance;
	
	/** Sets the duration over which the indicator widget is animated in */
	UPROPERTY(EditAnywhere, Category = "InteractionComponent|IndicatorWidget")
	float AnimationDuration;
	
	/** The height from the interactable root that the widget will animate up to as it appears */
	UPROPERTY(EditAnywhere, Category = "InteractionComponent|IndicatorWidget")
	float AnimationEndHeight;

	/** Curve with the animation behavior for the appearance of the indicator widget, should start with a value of 0 and end at 1
	* Reverse animation is used for fade out */
	UPROPERTY(EditAnywhere, Category = "InteractionComponent|IndicatorWidget")
	UCurveFloat* AnimationCurve;

	/** The UUserWidget that will appear when an object can be interacted with */
	UPROPERTY(EditAnywhere, Category = "InteractionComponent|IndicatorWidget")
	TSubclassOf<UUserWidget> InteractableIndicatorWidgetClass;

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 2. References and State
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	UPROPERTY(VisibleAnywhere, Category = "InteractionComponent")
	AActor* CurrentInteractable;
	UUserWidget* PickupIndicatorWidget;
	UWidgetComponent* InteractableIndicatorWidgetComponent;
	FTimeline IndicatorAnimationTimeline;
	FVector IndicatorOffset;
	FTimerHandle InteractableTraceTickTimerHandle;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 3. Overrides
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:	
	/** Called every frame while ComponentTick is enabled, ticks the animation timeline */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	/** Called when the game starts or this component is spawned*/
	virtual void BeginPlay() override;


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 4. Interface and Methods
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:	

	/** [local] Initiates a line trace on the server, if it hits a valid interactable, calls Interact() on it */
	UFUNCTION()
	void Interact();

private:
	/** [local] Initializes the InteractionIndicatorWidget, sets up it's animation, and starts Interactable Trace tick */
	void Initialize();

	/** [local] Runs a line trace for an Interactable object and shows indicator if we hit an Interactable */
	void InteractableTraceTick();

	/** [local + server] Runs a line trace for an Interactable object */
	AActor* TraceForInteractable() const;

	/** [local] Initiates animation to show or hide the InteractionIndicatorWidget */
	void SetCurrentInteractable(AActor* Interactable);
	
	/** [local] Updates the IndicatorWidget animation */
	UFUNCTION()
	void HandleTimelineAnimation(float Value);

	/** [local] Turns off ticking when not animating */
	UFUNCTION()
	void OnTimelineFinish();

	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 5. Server RPC's
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/** [server] Marked Unreliable so it will only work if good networking conditions,
	*  since this shouldn't be crucial, a failed attempt can just try again. */
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerInteract();
	void ServerInteract_Implementation();
	bool ServerInteract_Validate();
};
