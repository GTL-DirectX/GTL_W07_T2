#include "PointLightComponent.h"

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

ELightComponentType UPointLightComponent::GetLightType() const
{
    return LightType_Point;
}
