#include "FireballActor.h"
#include "Engine/FLoaderOBJ.h"

#include "Components/Light/PointLightComponent.h"

#include "Components/ProjectileMovementComponent.h"

#include "Components/SphereComp.h"

AFireballActor::AFireballActor()
{
    FManagerOBJ::CreateStaticMesh("Contents/Sphere.obj");
    
    SphereComp = AddComponent<USphereComp>("USphereComp_0");
    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));
  
    PointLightComponent = AddComponent<UPointLightComponent>("UPointLightComponent_0");
    
    PointLightComponent->SetLightColor(FColor::Red);
    
    ProjectileMovementComponent = AddComponent<UProjectileMovementComponent>("UProjectileMovementComponent_0");
    PointLightComponent->AttachToComponent(RootComponent);

    ProjectileMovementComponent->SetGravity(0);
    ProjectileMovementComponent->SetVelocity(FVector(100, 0, 0));
    ProjectileMovementComponent->SetInitialSpeed(100);
    ProjectileMovementComponent->SetMaxSpeed(100);
    ProjectileMovementComponent->SetLifetime(10);
}

AFireballActor::~AFireballActor()
{
}

UObject* AFireballActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->SphereComp = NewActor->GetComponentByFName<USphereComp>(SphereComp->GetFName());
    NewActor->PointLightComponent = NewActor->GetComponentByFName<UPointLightComponent>(PointLightComponent->GetFName());
    NewActor->ProjectileMovementComponent = NewActor->GetComponentByFName<UProjectileMovementComponent>(ProjectileMovementComponent->GetFName());

    return NewActor;
}

void AFireballActor::BeginPlay()
{
}
