#pragma once

#include "Components/SceneComponent.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();
    virtual ~ULightComponentBase() override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    virtual UObject* Duplicate(UObject* InOuter) override;

public:
    float GetIntensity() const { return Intensity; }
    void SetIntensity(float InIntensity) { Intensity = InIntensity; }
    FColor GetLightColor() const { return LightColor; }
    void SetLightColor(const FColor& InColor) { LightColor = InColor; }
    
protected:
    float Intensity;
    FColor LightColor;
    
};
