#pragma once
#include "Math/Vector.h"
#include "Math/Color.h"

#include "Renderer/Types/ShadowTypes.h"

#define LIGHT_TYPE_DIRECTIONAL		0 
#define LIGHT_TYPE_POINT			1 
#define LIGHT_TYPE_SPOT				2 
#define LIGHT_TYPE_RECT				3 
#define LIGHT_TYPE_MAX				4 

struct FAmbientLightInfo
{
    FLinearColor AmbientColor;         // RGB + alpha
};

struct FDirectionalLightInfo
{
    FLinearColor LightColor;         // RGB + alpha

    FVector Direction;   // 정규화된 광선 방향 (월드 공간 기준)
    float   Intensity;   // 밝기

    FMatrix View;
    FMatrix Projection;

    FShadowInfo ShadowInfo; // 그림자 정보
};

struct FPointLightInfo
{
    FLinearColor LightColor;         // RGB + alpha

    FVector Position;    // 월드 공간 위치
    float   Radius;      // 감쇠가 0이 되는 거리

    int     Type;        // 라이트 타입 구분용 (예: 1 = Point)
    float   Intensity;   // 밝기
    float   Attenuation;
    float   Padding;  // 16바이트 정렬

    FMatrix View[6];
    FMatrix Projection;

    FShadowInfo ShadowInfo; // 그림자 정보
};

struct FSpotLightInfo
{
    FLinearColor LightColor;         // RGB + alpha

    FVector Position;       // 월드 공간 위치
    float   Radius;         // 감쇠 거리

    FVector Direction;      // 빛이 향하는 방향 (normalize)
    float   Intensity;      // 밝기

    int     Type;           // 라이트 타입 구분용 (예: 2 = Spot)
    float   InnerRad; // cos(inner angle)
    float   OuterRad; // cos(outer angle)
    float   Attenuation;

    FMatrix View;
    FMatrix Projection;

    FShadowInfo ShadowInfo; // 그림자 정보
};

struct FLightCount
{   
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    int AmbientLightsCount;
};
