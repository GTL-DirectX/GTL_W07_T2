#pragma once
#include "LightComponent.h"

class UDirectionalLightComponent : public ULightComponent
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)

public:
    UDirectionalLightComponent();
    virtual ~UDirectionalLightComponent() override;
    
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    FVector GetDirection();

    const FDirectionalLightInfo& GetDirectionalLightInfo() const;
    void SetDirectionalLightInfo(const FDirectionalLightInfo& InDirectionalLightInfo);

    float GetIntensity() const;
    void SetIntensity(float InIntensity);

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);

private:
    FDirectionalLightInfo DirectionalLightInfo;
};

