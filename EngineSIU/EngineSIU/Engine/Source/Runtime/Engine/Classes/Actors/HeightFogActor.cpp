#include "HeightFogActor.h"

#include "Components/HeightFogComponent.h"

AHeightFogActor::AHeightFogActor()
{
    HeightFogComponent = AddComponent<UHeightFogComponent>("UHeightFogComponent_0");    
}

UObject* AHeightFogActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->RootComponent = NewActor->GetComponentByFName<USceneComponent>(RootComponent->GetFName());

    return NewActor;
}
