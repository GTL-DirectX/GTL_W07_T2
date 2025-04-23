#pragma once
#include "HAL/PlatformType.h"

namespace EShadowResolutionLevel
{
    enum Type
    {
        // 다른 괜찮은 이름 추천
        UltraLow = 1,
        VeryLow = 2,
        Low = 3,
        Medium = 4,
        High = 5,
        VeryHigh = 6,
        UltraHigh = 7,
        Extreme = 8,
    };
}

struct FShadowLightConstants
{
    uint32 LightIndex;
    float NearPlane;
    float FarPlane;
    uint32 PointLightIndex;
    uint32 CascadedIndex;
    FVector Padding2;
};

struct FShadowInfo
{
    float ShadowResolutionScale; // 그림자 해상도 비율
    float ShadowBias; // 그림자 바이어스
    float ShadowSlopeBias; // 그림자 경사 바이어스
    float ShadowSharpen; // 그림자 선명도
    uint32 ShadowResolutionLevel; // 그림자 해상도 Level
    FVector Padding;

};
