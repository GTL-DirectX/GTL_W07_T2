#pragma once
#include "LightActor.h"
class APointLight : public ALight
{
    DECLARE_CLASS(APointLight, ALight)
public:
    APointLight();
    virtual ~APointLight();
    UObject* Duplicate(UObject* InOuter) override;
protected:
    UPROPERTY
    (UPointLightComponent*, PointLightComponent, = nullptr);

    UPROPERTY
    (UBillboardComponent*, BillboardComponent, = nullptr);
};

