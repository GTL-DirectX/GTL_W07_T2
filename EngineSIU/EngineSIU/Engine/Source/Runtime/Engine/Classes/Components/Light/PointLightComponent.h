#pragma once

#include "LightComponent.h"

class UPointLightComponent :public ULightComponent
{

    DECLARE_CLASS(UPointLightComponent, ULightComponent)
public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    // Getters and Setters
    virtual ELightComponentType GetLightType() const override;
    
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

