#pragma once

#include "LightComponent.h"

class UPointLightComponent :public ULightComponent
{

    DECLARE_CLASS(UPointLightComponent, ULightComponent)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    virtual ELightComponentType GetLightType() const override;

    // Getters and Setters
    float GetRadius() const { return Radius; }
    void SetRadius(float InRadius) { Radius = InRadius; }
    float GetFallOffExponent() const { return FallOffExponent; }
    void SetFallOffExponent(float InFallOffExponent) { FallOffExponent = InFallOffExponent; }
    float GetAttenuation() const { return Attenuation; }
    void SetAttenuation(float InAttenuation) { Attenuation = InAttenuation; }
    // End of Getters and Setters

private:
    float Radius;
    float FallOffExponent;
    float Attenuation;

};

