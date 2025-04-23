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
    OutProperties.Add(TEXT("ShadowResolutionScale"), FString::SanitizeFloat(ShadowResolutionScale));
    OutProperties.Add(TEXT("ShadowBias"), FString::SanitizeFloat(ShadowBias));
    OutProperties.Add(TEXT("ShadowSlopeBias"), FString::SanitizeFloat(ShadowSlopeBias));
    OutProperties.Add(TEXT("ShadowSharpen"), FString::SanitizeFloat(ShadowSharpen));
    OutProperties.Add(TEXT("ShadowResolutionLevel"), FString::Printf(TEXT("%d"), static_cast<int>(ShadowResolutionLevel)));
}

void ULightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("ShadowResolutionScale"));
    if (TempStr)
    {
        ShadowResolutionScale = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("ShadowBias"));
    if (TempStr)
    {
        ShadowBias = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("ShadowSlopeBias"));
    if (TempStr)
    {
        ShadowSlopeBias = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("ShadowSharpen"));
    if (TempStr)
    {
        ShadowSharpen = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("ShadowResolutionLevel"));
    if (TempStr)
    {
        ShadowResolutionLevel = static_cast<EShadowResolutionLevel>(FCString::Atoi(**TempStr));
    }
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
