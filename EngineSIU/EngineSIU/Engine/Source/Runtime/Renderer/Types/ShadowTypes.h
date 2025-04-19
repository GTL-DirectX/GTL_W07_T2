#pragma once
#include "HAL/PlatformType.h"
#include "Math/Vector.h"

struct FShadowLightConstants
{
    uint32 LightIndex;
    float NearPlane;
    float FarPlane;
    float Padding;
};
