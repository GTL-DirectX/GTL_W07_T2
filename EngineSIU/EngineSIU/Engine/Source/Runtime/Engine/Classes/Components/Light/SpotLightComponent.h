#pragma once

#include "LightComponent.h"

class USpotLightComponent :public ULightComponent
{

    DECLARE_CLASS(USpotLightComponent, ULightComponent)
public:
    USpotLightComponent();

    // Getters and Setters
    FVector GetDirection() const;
    virtual ELightComponentType GetLightType() const override;
    float GetRadius() const { return Radius; }
    void SetRadius(float InRadius) { Radius = InRadius; }
    float GetInnerAngle() const { return InnerAngle; }
    void SetInnerAngle(float InInnerAngle) { InnerAngle = InInnerAngle; }
    float GetOuterAngle() const { return OuterAngle; }
    void SetOuterAngle(float InOuterAngle) { OuterAngle = InOuterAngle; }
    float GetAttenuation() const { return Attenuation; }
    void SetAttenuation(float InAttenuation) { Attenuation = InAttenuation; }
    float GetFallOffExponent() const { return FallOffExponent; }
    void SetFallOffExponent(float InFallOffExponent) { FallOffExponent = InFallOffExponent; }
    // End of Getters and Setters

private:
    float Radius;
    float InnerAngle;
    float OuterAngle;
    float Attenuation;
    float FallOffExponent;

};

