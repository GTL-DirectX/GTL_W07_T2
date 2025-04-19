#include "SpotLightActor.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/BillboardComponent.h"
ASpotLight::ASpotLight()
{
    SpotLightComponent = AddComponent<USpotLightComponent>("USpotLightComponent_0");
    BillboardComponent = AddComponent<UBillboardComponent>("UBillboardComponent_0");

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/SpotLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    SpotLightComponent->AttachToComponent(RootComponent);
}

ASpotLight::~ASpotLight()
{
}

UObject* ASpotLight::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    
    NewActor->SpotLightComponent = NewActor->GetComponentByFName<USpotLightComponent>(SpotLightComponent->GetFName());
    NewActor->BillboardComponent = NewActor->GetComponentByFName<UBillboardComponent>(BillboardComponent->GetFName());


    return NewActor;
}
