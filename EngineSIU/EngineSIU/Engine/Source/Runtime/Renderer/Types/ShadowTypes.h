#pragma once
#include "HAL/PlatformType.h"

struct FShadowLightConstants
{
    uint32 LightIndex;
    float NearPlane;
    float FarPlane;
    uint32 PointLightIndex;
};
