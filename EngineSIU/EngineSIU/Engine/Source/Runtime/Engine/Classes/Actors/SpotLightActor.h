#pragma once
#include "LightActor.h"
class ASpotLight :
    public ALight
{
    DECLARE_CLASS(ASpotLight, ALight)
public:
    ASpotLight();
    virtual ~ASpotLight();
    UObject* Duplicate(UObject* InOuter) override;
    
protected:
    UPROPERTY
    (USpotLightComponent*, SpotLightComponent, = nullptr);

    UPROPERTY
    (UBillboardComponent*, BillboardComponent, = nullptr);
};

