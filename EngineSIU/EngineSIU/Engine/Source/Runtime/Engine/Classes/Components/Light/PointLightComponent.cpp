#include "PointLightComponent.h"

#include "UObject/Casts.h"

UPointLightComponent::UPointLightComponent()
{
    Intensity = 1000.f;
    Radius = 30.0f;
    FallOffExponent = 2.0f;
    Attenuation = 20.0f;
}

UPointLightComponent::~UPointLightComponent()
{
}

UObject* UPointLightComponent::Duplicate(UObject* InOuter)
{

    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->Radius = Radius;
        NewComponent->FallOffExponent = FallOffExponent;
        NewComponent->Attenuation = Attenuation;
    }
    return NewComponent;
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Radius"), FString::Printf(TEXT("%f"), Radius));
    OutProperties.Add(TEXT("Attenuation"), FString::Printf(TEXT("%f"), Attenuation));
    OutProperties.Add(TEXT("FallOffExponent"), FString::Printf(TEXT("%f"), FallOffExponent));
}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Radius"));
    if (TempStr)
    {
        Radius = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightColor"));
    if (TempStr)
    {
        LightColor = FLinearColor(*TempStr).ToColorSRGB();
    }
    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        Intensity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Attenuation"));
    if (TempStr)
    {
        Attenuation = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FallOffExponent"));
    if (TempStr)
    {
        FallOffExponent = FString::ToFloat(*TempStr);
    }
}

ELightComponentType UPointLightComponent::GetLightType() const
{
    return LightType_Point;
}
