#pragma once
#include "HAL/PlatformType.h"


enum class EViewModeIndex : uint8
{
    VMI_Lit_Gouraud = 0,
    VMI_Lit_Lambert = 1,
    VMI_Lit_BlinnPhong = 2,
    VMI_Unlit = 3, // Lit 모드는 이 위에 추가해주세요.
    VMI_Wireframe = 4,
    VMI_SceneDepth = 5,
    VMI_WorldNormal = 6,
    VMI_MAX = 16,
};


enum ELevelViewportType : uint8
{
    LVT_Perspective = 0,

    /** Top */
    LVT_OrthoXY = 1,
    /** Bottom */
    LVT_OrthoNegativeXY,
    /** Left */
    LVT_OrthoYZ,
    /** Right */
    LVT_OrthoNegativeYZ,
    /** Front */
    LVT_OrthoXZ,
    /** Back */
    LVT_OrthoNegativeXZ,

    LVT_MAX,
    LVT_None = 255,
};
