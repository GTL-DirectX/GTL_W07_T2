#pragma once

#include "LightDefine.h"

enum ELightComponentType
{
    LightType_Directional = LIGHT_TYPE_DIRECTIONAL,
    LightType_Point		  = LIGHT_TYPE_POINT,
    LightType_Spot		  = LIGHT_TYPE_SPOT,
    LightType_Rect 		  = LIGHT_TYPE_RECT,
    LightType_MAX         = LIGHT_TYPE_MAX,
    LightType_NumBits = 2
};
