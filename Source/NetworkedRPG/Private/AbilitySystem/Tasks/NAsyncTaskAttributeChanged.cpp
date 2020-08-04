// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tasks/NAsyncTaskAttributeChanged.h"

UNAsyncTaskAttributeChanged* UNAsyncTaskAttributeChanged::ListenForAttributeChange(
    UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute Attribute)
{
    
    UNAsyncTaskAttributeChanged* WaitForAttributeChangedTask = NewObject<UNAsyncTaskAttributeChanged>();
    WaitForAttributeChangedTask->ASC = AbilitySystemComponent;
    WaitForAttributeChangedTask->AttributeToListenFor = Attribute;

    if (!AbilitySystemComponent || !Attribute.IsValid())
    {
        WaitForAttributeChangedTask->RemoveFromRoot();
        return nullptr;
    }
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(WaitForAttributeChangedTask, &UNAsyncTaskAttributeChanged::AttributeChanged);

    return WaitForAttributeChangedTask;
}

UNAsyncTaskAttributeChanged* UNAsyncTaskAttributeChanged::ListenForAttributesChange(
    UAbilitySystemComponent* AbilitySystemComponent, TArray<FGameplayAttribute> Attributes)
{
    UE_LOG(LogTemp, Warning, TEXT("ListenForAttributes"))
    UNAsyncTaskAttributeChanged* WaitForAttributeChangedTask = NewObject<UNAsyncTaskAttributeChanged>();
    WaitForAttributeChangedTask->ASC = AbilitySystemComponent;
    WaitForAttributeChangedTask->AttributesToListenFor = Attributes;

    if (!AbilitySystemComponent || Attributes.Num() < 1)
    {
        WaitForAttributeChangedTask->RemoveFromRoot();
        return nullptr;
    }

    for (FGameplayAttribute Attribute : Attributes)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(WaitForAttributeChangedTask, &UNAsyncTaskAttributeChanged::AttributeChanged);
    }
    
    return WaitForAttributeChangedTask;
}

void UNAsyncTaskAttributeChanged::EndTask()
{
    if (ASC)
    {
        ASC->GetGameplayAttributeValueChangeDelegate(AttributeToListenFor).RemoveAll(this);

        for (FGameplayAttribute Attribute : AttributesToListenFor)
        {
            ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
        }
    }
}

void UNAsyncTaskAttributeChanged::AttributeChanged(const FOnAttributeChangeData& Data)
{
    // UE_LOG(LogTemp, Warning, TEXT("Broadcast Attribute Changed"))
    OnAttributeChanged.Broadcast(Data.Attribute, Data.NewValue, Data.OldValue);
}
