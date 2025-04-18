#include "SpotLightComponent.h"

#include "Math/Quat.h"
#include "UObject/Casts.h"

USpotLightComponent::USpotLightComponent()
{
    Radius = 30.0f;
    Intensity = 1000.0f;
    InnerAngle = 0.2618f; // 15 degrees
    OuterAngle = 0.5236f; // 30 degrees
    Attenuation = 20.0f;
    FallOffExponent = 2.0f;
}

UObject* USpotLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->Radius = Radius;
        NewComponent->InnerAngle = InnerAngle;
        NewComponent->OuterAngle = OuterAngle;
        NewComponent->Attenuation = Attenuation;
        NewComponent->FallOffExponent = FallOffExponent;
    }

    return NewComponent;
}

void USpotLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Radius"), FString::Printf(TEXT("%f"), Radius));
    OutProperties.Add(TEXT("InnerAngle"), FString::Printf(TEXT("%f"), InnerAngle));
    OutProperties.Add(TEXT("OuterAngle"), FString::Printf(TEXT("%f"), OuterAngle));
    OutProperties.Add(TEXT("Attenuation"), FString::Printf(TEXT("%f"), Attenuation));
    OutProperties.Add(TEXT("FallOffExponent"), FString::Printf(TEXT("%f"), FallOffExponent));
}

void USpotLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
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
    TempStr = InProperties.Find(TEXT("InnerAngle"));
    if (TempStr)
    {
        InnerAngle = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("OuterAngle"));
    if (TempStr)
    {
        OuterAngle = FString::ToFloat(*TempStr);
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

FVector USpotLightComponent::GetDirection() const
{
    FRotator rotator = GetWorldRotation();
    FVector WorldForward = rotator.ToQuaternion().RotateVector(GetForwardVector());
    return WorldForward;
}

ELightComponentType USpotLightComponent::GetLightType() const
{
    return LightType_Spot;
}
