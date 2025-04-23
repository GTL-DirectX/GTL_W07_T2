#pragma once
#include "HAL/PlatformType.h"


enum class EShadowResolutionLevel
{
    // 다른 괜찮은 이름 추천
    UltraLow = 0,
    VeryLow = 1,
    Low = 2,
    Medium = 3,
    High = 4,
    VeryHigh = 5,
    UltraHigh = 6,
    Extreme = 7,
    Max = 8,
};

struct FShadowLightConstants
{
    uint32 LightIndex;
    uint32 LightInnerIndex;
};

struct FShadowInfo
{
    float ShadowResolutionScale; // 그림자 해상도 비율
    float ShadowBias; // 그림자 바이어스
    float ShadowSlopeBias; // 그림자 경사 바이어스
    float ShadowSharpen; // 그림자 선명도
    uint32 ShadowResolutionLevel; // 그림자 해상도 Level
    int bUseShadowPCF; // 그림자 PCF 사용 여부
    FVector2D Padding;
};
