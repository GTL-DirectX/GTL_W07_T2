#include "DirectionalLightComponent.h"

#include "Math/Rotator.h"
#include "Math/Quat.h"

#include "UObject/Casts.h"

UDirectionalLightComponent::UDirectionalLightComponent()
{
    Intensity = 10.0f;
    LightColor = FColor::White;
}

UDirectionalLightComponent::~UDirectionalLightComponent()
{
}

UObject* UDirectionalLightComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    if (!NewComponent)
        return nullptr;

    return NewComponent;
}

void UDirectionalLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void UDirectionalLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}

FVector UDirectionalLightComponent::GetDirection() const 
{
    FRotator rotator = GetWorldRotation();
    FVector WorldDown= rotator.ToQuaternion().RotateVector(-GetUpVector());
    return WorldDown;  
}
