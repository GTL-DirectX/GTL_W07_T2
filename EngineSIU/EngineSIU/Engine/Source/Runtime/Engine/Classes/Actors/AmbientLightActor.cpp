#include "AmbientLightActor.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Components/BillboardComponent.h"

AAmbientLight::AAmbientLight()
{
    AmbientLightComponent = AddComponent<UAmbientLightComponent>("UAmbientLightComponent_0");
    BillboardComponent = AddComponent<UBillboardComponent>("UBillboardComponent_0");

    RootComponent = BillboardComponent;

    BillboardComponent->SetTexture(L"Assets/Editor/Icon/AmbientLight_64x.png");
    BillboardComponent->bIsEditorBillboard = true;

    AmbientLightComponent->AttachToComponent(RootComponent);

}

AAmbientLight::~AAmbientLight()
{
}

UObject* AAmbientLight::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    
    NewActor->AmbientLightComponent = NewActor->GetComponentByFName<UAmbientLightComponent>(AmbientLightComponent->GetFName());
    NewActor->BillboardComponent = NewActor->GetComponentByFName<UBillboardComponent>(BillboardComponent->GetFName());


    return NewActor;
}
