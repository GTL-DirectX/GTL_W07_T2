#include "DirectionalLightComponent.h"

#include "Math/Rotator.h"
#include "Math/Quat.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    Intensity = 10.0f;
    LightColor = FColor::White;
}

FVector UDirectionalLightComponent::GetDirection() const 
{
    FRotator rotator = GetWorldRotation();
    FVector WorldDown= rotator.ToQuaternion().RotateVector(-GetUpVector());
    return WorldDown;  
}
