#pragma once

#include "LightComponent.h"

class UAmbientLightComponent : public ULightComponent
{
    DECLARE_CLASS(UAmbientLightComponent, ULightComponent)

public:
    UAmbientLightComponent();
    virtual ~UAmbientLightComponent() override;
    
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    const FAmbientLightInfo& GetAmbientLightInfo() const;
    void SetAmbientLightInfo(const FAmbientLightInfo& InAmbient);

    FLinearColor GetLightColor() const;
    void SetLightColor(const FLinearColor& InColor);

private:
    FAmbientLightInfo AmbientLightInfo;
};
