#include "LightComponent.h"
#include "UObject/Casts.h"

ULightComponent::ULightComponent()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };

    ShadowResolutionScale = 1.0f;
    ShadowBias = 0.5f;
    ShadowSlopeBias = 0.5f;
    ShadowSharpen = 0.03;
}

void ULightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void ULightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}

UObject* ULightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AABB = AABB;
  
    return NewComponent;
}

ELightComponentType ULightComponent::GetLightType() const
{
    return LightType_MAX;
}

int ULightComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res = AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}
