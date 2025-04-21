#include "DirectionalLightActor.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/BillboardComponent.h"
ADirectionalLight::ADirectionalLight()
{
    DirectionalLightComponent = AddComponent<UDirectionalLightComponent>("UDirectionalLightComponent_0");
    BillboardComponent = AddComponent<UBillboardComponent>("UBillboardComponent_0");

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/DirectionalLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    DirectionalLightComponent->AttachToComponent(RootComponent);
}

ADirectionalLight::~ADirectionalLight()
{
}

UObject* ADirectionalLight::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    
    NewActor->DirectionalLightComponent = NewActor->GetComponentByFName<UDirectionalLightComponent>(DirectionalLightComponent->GetFName());
    NewActor->BillboardComponent = NewActor->GetComponentByFName<UBillboardComponent>(BillboardComponent->GetFName());

    return NewActor;
}

void ADirectionalLight::SetIntensity(float Intensity)
{
    if (DirectionalLightComponent)
    {
        DirectionalLightComponent->SetIntensity(Intensity);
    }
}
