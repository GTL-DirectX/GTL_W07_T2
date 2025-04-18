#include "SpotLightComponent.h"

#include "Math/Quat.h"

USpotLightComponent::USpotLightComponent()
{
    Radius = 30.0f;
    Intensity = 1000.0f;
    InnerAngle = 0.2618f; // 15 degrees
    OuterAngle = 0.5236f; // 30 degrees
    Attenuation = 20.0f;
    FallOffExponent = 2.0f;
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
