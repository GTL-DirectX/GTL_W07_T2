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

public:
    float GetShadowResolutionScale() const { return ShadowResolutionScale; }
    void SetShadowResolutionScale(float InShadowResolutionScale) { ShadowResolutionScale = InShadowResolutionScale; }
    float GetShadowBias() const { return ShadowBias; }
    void SetShadowBias(float InShadowBias) { ShadowBias = InShadowBias; }
    float GetShadowSlopeBias() const { return ShadowSlopeBias; }
    void SetShadowSlopeBias(float InShadowSlopeBias) { ShadowSlopeBias = InShadowSlopeBias; }
    float GetShadowSharpen() const { return ShadowSharpen; }
    void SetShadowSharpen(float InShadowSharpen) { ShadowSharpen = InShadowSharpen; }
    
protected:
    // Shadow Map 해상도 비율 조절 값. 1.0 기본, > 1.0 고해상도, < 1.0 저해상도.
    float ShadowResolutionScale;
    // 그림자 Bias 값. 0.0f ~ 1.0f. Shadow Acne 방지를 위한 Depth Offset.
    float ShadowBias;
    // 그림자 Slope Bias 값. 0.0f ~ 1.0f. 기울어진 면에서 Shadow Acne 방지.
    float ShadowSlopeBias;
    // 그림자 Sharpening 값. 0.0f ~ 1.0f. 그림자 경계 선명도 조절 값.
    // 0.0f = 흐릿한 그림자, 1.0f = 선명한 그림자. 1.0f 이상은 그림자 경계가 날카로워짐.
    float ShadowSharpen;
    
};
