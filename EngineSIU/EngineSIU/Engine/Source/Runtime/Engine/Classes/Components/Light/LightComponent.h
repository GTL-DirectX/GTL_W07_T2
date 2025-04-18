#pragma once

#include "LightComponentBase.h"
#include "SceneTypes.h"

class ULightComponent : public ULightComponentBase
{
    DECLARE_CLASS(ULightComponent, ULightComponentBase)
    
public:
    ULightComponent();
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual UObject* Duplicate(UObject* InOuter) override; 

public:
    virtual ELightComponentType GetLightType() const;

public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

        
protected:
    FBoundingBox AABB;

    
};
