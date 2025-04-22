#pragma once
#include "HAL/PlatformType.h"

struct FShadowLightConstants
{
    uint32 LightIndex;
    float NearPlane;
    float FarPlane;
    uint32 PointLightIndex;
};

struct FShadowInfo
{
    float ShadowResolutionScale; // 그림자 해상도 비율
    float ShadowBias; // 그림자 바이어스
    float ShadowSlopeBias; // 그림자 경사 바이어스
    float ShadowSharpen; // 그림자 선명도
};
