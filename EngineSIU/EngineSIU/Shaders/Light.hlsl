
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4

#define CASCADE_COUNT 4

struct FShadowInfo
{
    float ShadowResolutionScale;
    float ShadowBias;
    float ShadowSlopeBias;
    float ShadowSharpen;
    uint ShadowResolutionLevel;
    uint bUseShadowPCF;
    float2 Padding;
};

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
    
    row_major matrix ViewMatrix[CASCADE_COUNT];
    row_major matrix ProjectionMatrix[CASCADE_COUNT];
    
    FShadowInfo ShadowInfo;
};

struct FPointLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    int Type;
    float Intensity;
    float Attenuation;
    float Padding;
    row_major matrix ViewMatrix[6];
    row_major matrix ProjectionMatrix;
    
    FShadowInfo ShadowInfo;
};

struct FSpotLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    float3 Direction;
    float Intensity;

    int Type;
    float InnerRad;
    float OuterRad;
    float Attenuation;

    row_major matrix ViewMatrix;
    row_major matrix ProjectionMatrix;
    
    FShadowInfo ShadowInfo;
};

StructuredBuffer<FAmbientLightInfo> AmbientLights : register(t60);
StructuredBuffer<FDirectionalLightInfo> DirectionalLights : register(t61);
StructuredBuffer<FPointLightInfo> PointLights : register(t62);
StructuredBuffer<FSpotLightInfo> SpotLights : register(t63);

Texture2DArray<float> DirectionalShadowMap[8] : register(t70); // 70 ~ 79 예약
TextureCubeArray<float> PointShadowMap[8] : register(t80); // 80 ~ 89 예약
Texture2DArray<float> SpotShadowMap[8] : register(t90); // 90 ~ 99 예약

SamplerComparisonState ShadowSampler : register(s2);

// Helper function to check if a value is in a range.
bool InRange(float val, float min, float max)
{
    return (min <= val && val <= max);
}

void GetDirectionalShadowMapResolution(uint Level, out float Widht, out float Height)
{
    float Element;
    [branch]
    switch (Level)
    {
        case 0:
            DirectionalShadowMap[0].GetDimensions(Widht, Height, Element);
            break;
        case 1:
            DirectionalShadowMap[1].GetDimensions(Widht, Height, Element);
            break;
        case 2:
            DirectionalShadowMap[2].GetDimensions(Widht, Height, Element);
            break;
        case 3:
            DirectionalShadowMap[3].GetDimensions(Widht, Height, Element);
            break;
        case 4:
            DirectionalShadowMap[4].GetDimensions(Widht, Height, Element);
            break;
        case 5:
            DirectionalShadowMap[5].GetDimensions(Widht, Height, Element);
            break;
        case 6:
            DirectionalShadowMap[6].GetDimensions(Widht, Height, Element);
            break;
        case 7:
            DirectionalShadowMap[7].GetDimensions(Widht, Height, Element);
            break;
        default:
            Widht = 0;
            Height = 0;
            break;
    }
}

void GetSpotLightShadowMapResolution(uint Level, out float Widht, out float Height)
{
    float Element;
    [branch]
    switch (Level)
    {
        case 0:
            SpotShadowMap[0].GetDimensions(Widht, Height, Element);
            break;
        case 1:
            SpotShadowMap[1].GetDimensions(Widht, Height, Element);
            break;
        case 2:
            SpotShadowMap[2].GetDimensions(Widht, Height, Element);
            break;
        case 3:
            SpotShadowMap[3].GetDimensions(Widht, Height, Element);
            break;
        case 4:
            SpotShadowMap[4].GetDimensions(Widht, Height, Element);
            break;
        case 5:
            SpotShadowMap[5].GetDimensions(Widht, Height, Element);
            break;
        case 6:
            SpotShadowMap[6].GetDimensions(Widht, Height, Element);
            break;
        case 7:
            SpotShadowMap[7].GetDimensions(Widht, Height, Element);
            break;
        default:
            Widht = 0;
            Height = 0;
            break;
    }
}

float SampleDirectionalShadowMap(uint Level, float3 UV, float Depth)
{
    [branch]
    if (Level == 0)
        return DirectionalShadowMap[0].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 1)
        return DirectionalShadowMap[1].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 2)
        return DirectionalShadowMap[2].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 3)
        return DirectionalShadowMap[3].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 4)
        return DirectionalShadowMap[4].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 5)
        return DirectionalShadowMap[5].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 6)
        return DirectionalShadowMap[6].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 7)
        return DirectionalShadowMap[7].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else
        return 1;
}

float SamplePointLIghtShadowMap(uint Level, float4 UV, float Depth)
{
    [branch]
    if (Level == 0)
        return PointShadowMap[0].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 1)
        return PointShadowMap[1].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 2)
        return PointShadowMap[2].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 3)
        return PointShadowMap[3].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 4)
        return PointShadowMap[4].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 5)
        return PointShadowMap[5].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 6)
        return PointShadowMap[6].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 7)
        return PointShadowMap[7].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else
        return 1;
}

float SampleSpotLightShadowMap(uint Level, float3 UV, float Depth)
{
    [branch]
    if (Level == 0)
        return SpotShadowMap[0].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 1)
        return SpotShadowMap[1].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 2)
        return SpotShadowMap[2].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 3)
        return SpotShadowMap[3].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 4)
        return SpotShadowMap[4].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 5)
        return SpotShadowMap[5].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 6)
        return SpotShadowMap[6].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else if (Level == 7)
        return SpotShadowMap[7].SampleCmpLevelZero(ShadowSampler, UV, Depth);
    else
        return 1;
}

float SampleDirectionalShadowMapPCF(uint Level, float3 UV, float Depth)
{
    float shadow = 0.0f;
    float2 baseUV = UV.xy;
    uint slice = (uint) UV.z;
    float ShadowMapWidth;
    float ShadowMapHeight;
    float Element;
    GetDirectionalShadowMapResolution(Level, ShadowMapWidth, ShadowMapHeight);
    float2 texelSize = 1.0f / float2(ShadowMapWidth, ShadowMapHeight); // 상수 버퍼에서 제공 필요

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            float2 offsetCoord = baseUV + float2(x, y) * texelSize;
            if (InRange(offsetCoord.x, 0.f, 1.f) && InRange(offsetCoord.y, 0.f, 1.f))
            {
                float3 uv = float3(offsetCoord, slice);
                shadow += SampleDirectionalShadowMap(Level, uv, Depth);
            }
            else
            {
                shadow += 1.0f;
            }
        }
    }
    return shadow / 9.0f;
}

float SampleSpotLightShadowMapPCF(uint Level, float3 UV, float Depth)
{
    float shadow = 0.0f;
    float2 baseUV = UV.xy;
    uint slice = (uint) UV.z;
    float ShadowMapWidth;
    float ShadowMapHeight;
    GetSpotLightShadowMapResolution(Level, ShadowMapWidth, ShadowMapHeight);
    float2 texelSize = 1.0f / float2(ShadowMapWidth, ShadowMapHeight); // 상수 버퍼에서 제공 필요

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            float2 offsetCoord = baseUV + float2(x, y) * texelSize;
            if (InRange(offsetCoord.x, 0.f, 1.f) && InRange(offsetCoord.y, 0.f, 1.f))
            {
                float3 uv = float3(offsetCoord, slice);
                shadow += SampleSpotLightShadowMap(Level, uv, Depth);
            }
            else
            {
                shadow += 1.0f;
            }
        }
    }
    return shadow / 9.0f;
}

float SamplePointLightShadowMapPCF(uint Level, float3 Direction, uint ArrayIndex, float Depth)
{
    float shadow = 0.0f;
    
    static const float3 pcfOffsets[12] =
    {
        float3(0.004f, 0.000f, 0.000f),
        float3(-0.004f, 0.000f, 0.000f),
        float3(0.000f, 0.004f, 0.000f),
        float3(0.000f, -0.004f, 0.000f),
        float3(0.000f, 0.000f, 0.004f),
        float3(0.000f, 0.000f, -0.004f),

        float3(0.0028f, 0.0028f, 0.000f),
        float3(-0.0028f, 0.0028f, 0.000f),
        float3(0.0028f, -0.0028f, 0.000f),
        float3(-0.0028f, -0.0028f, 0.000f),

        float3(0.000f, 0.0028f, 0.0028f),
        float3(0.000f, -0.0028f, -0.0028f)
    };

    for (int i = 0; i < 12; ++i)
    {
        float3 offsetDir = normalize(Direction + pcfOffsets[i]);
        shadow += SamplePointLIghtShadowMap(Level, float4(offsetDir, ArrayIndex), Depth);
    }

    return shadow / 12.0f;
}

cbuffer cbLightCount : register(b0)
{    
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    int AmbientLightsCount;
};

float GetDistanceAttenuation(float Distance, float Radius)
{
    float  InvRadius = 1.0 / Radius;
    float  DistSqr = Distance * Distance;
    float  RadiusMask = saturate(1.0 - DistSqr * InvRadius * InvRadius);
    RadiusMask *= RadiusMask;
    
    return RadiusMask / (DistSqr + 1.0);
}

float GetSpotLightAttenuation(float Distance, float Radius, float3 LightDir, float3 SpotDir, float InnerRadius, float OuterRadius)
{
    float DistAtten = GetDistanceAttenuation(Distance, Radius);
    
    float CosTheta = dot(normalize(SpotDir), -normalize(LightDir));
    
    float SpotMask = saturate((CosTheta - cos(OuterRadius)) / (cos(InnerRadius) - cos(OuterRadius)));
    SpotMask *= SpotMask;
    
    return DistAtten * SpotMask;
}

float CalculateDiffuse(float3 WorldNormal, float3 LightDir)
{
    return max(dot(WorldNormal, LightDir), 0.0);
}

float CalculateSpecular(float3 WorldNormal, float3 ToLightDir, float3 ViewDir, float Shininess, float SpecularStrength = 0.5)
{
#ifdef LIGHTING_MODEL_GOURAUD
    float3 ReflectDir = reflect(-ToLightDir, WorldNormal);
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess);
#else
    float3 HalfDir = normalize(ToLightDir + ViewDir); // Blinn-Phong
    float Spec = pow(max(dot(WorldNormal, HalfDir), 0.0), Shininess);
#endif
    return Spec * SpecularStrength;
}

float3 PointLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, float3 SpecularColor, float Shininess)
{
    FPointLightInfo LightInfo = PointLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = GetDistanceAttenuation(Distance, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float3(0.f, 0.f, 0.f);
    }
    
    float3 LightDir = normalize(ToLight);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);

    float3 Lit = (DiffuseFactor * DiffuseColor);
#ifndef LIGHTING_MODEL_LAMBERT
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Shininess);
    Lit += SpecularFactor * SpecularColor;
#endif
    
    return Lit * Attenuation * LightInfo.Intensity * LightInfo.LightColor.rgb;
}

float3 SpotLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, float3 SpecularColor, float Shininess)
{
    FSpotLightInfo LightInfo = SpotLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    float3 LightDir = normalize(ToLight);
    
    float SpotlightFactor = GetSpotLightAttenuation(Distance, LightInfo.Radius, LightDir, normalize(LightInfo.Direction), LightInfo.InnerRad, LightInfo.OuterRad);
    if (SpotlightFactor <= 0.0)
    {
        return float3(0.0, 0.0, 0.0);
    }
    
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
    //DiffuseColor = float3(1, 1, 1);
    
    float3 Lit = DiffuseFactor * DiffuseColor;
#ifndef LIGHTING_MODEL_LAMBERT
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Shininess);
    Lit += SpecularFactor * SpecularColor;
#endif
    
    return Lit * SpotlightFactor * LightInfo.Intensity * LightInfo.LightColor.rgb;
}

float3 DirectionalLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, float3 SpecularColor, float Shininess)
{
    FDirectionalLightInfo LightInfo = DirectionalLights[nIndex];
    
    float3 LightDir = normalize(-LightInfo.Direction);
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
    
    float3 Lit = DiffuseFactor * DiffuseColor;
#ifndef LIGHTING_MODEL_LAMBERT
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Shininess);
    Lit += SpecularFactor * SpecularColor;
#endif
    return Lit * LightInfo.Intensity * LightInfo.LightColor.rgb;
}

float GetLightFromShadowMap(float3 WorldPosition, float3 WorldNormal, uint LightIndex, inout uint ShadowMapIndices[8])
{
    //float bias = 0.001;

    float BiasStep = 0.000001f;
    float MinBias = 0.0f;
    float3 LightDirection;
    
    // Light (Dir -> Point -> Spot)순서 바뀌면 위험함.
    bool bIsDirectional = (DirectionalLightsCount > LightIndex);
    bool bIsPoint = !bIsDirectional && (DirectionalLightsCount + PointLightsCount > LightIndex);
    bool bIsSpot = !bIsDirectional && !bIsPoint && (DirectionalLightsCount + PointLightsCount + SpotLightsCount > LightIndex);

    uint TargetIndex = 0;

    if (bIsDirectional)
    {
        TargetIndex = LightIndex;
    }
    else if (bIsPoint)
    {
        TargetIndex = LightIndex - DirectionalLightsCount;
    }
    else if (bIsSpot)
    {
        TargetIndex = LightIndex - DirectionalLightsCount - PointLightsCount;
    }
    else
    {
        return 1;
    }
    
    float Result = 1;
    
    uint ShadowResolutionLevel = (bIsDirectional ? DirectionalLights[TargetIndex].ShadowInfo.ShadowResolutionLevel :
                                           (bIsPoint ? PointLights[TargetIndex].ShadowInfo.ShadowResolutionLevel : SpotLights[TargetIndex].ShadowInfo.ShadowResolutionLevel));
    if (ShadowResolutionLevel < 0 || ShadowResolutionLevel >= 8)
    {
        return 1.0f;
    }
 
    if (bIsPoint)
    {
         // PointLight 정보
        FPointLightInfo p = PointLights[TargetIndex];
        float3 LightPosition = p.Position;
        //float  bias = 0.001f;          // 샘플 바이어스

        // 월드 위치 동차 좌표로 변경
        float4 WorldPosition4 = float4(WorldPosition, 1.0f);

        // 방향 벡터 (큐브맵 샘플할 좌표)
        float3 LightDirection = normalize(WorldPosition - LightPosition);

        // 어떤 face를 쓸지 (GPU가 큐브맵 샘플링할 때와 동일한 규칙)
        float3 LightDirectionAbs = abs(LightDirection);                // 절대값 각 축 크기 구함.
        int   face;
        if (LightDirectionAbs.x >= LightDirectionAbs.y && LightDirectionAbs.x >= LightDirectionAbs.z) 
            face = LightDirection.x > 0 ? 0 : 1;   // +X, -X
        else if (LightDirectionAbs.y >= LightDirectionAbs.z)
            face = LightDirection.y > 0 ? 2 : 3;   // +Y, -Y
        else
            face = LightDirection.z > 0 ? 4 : 5;   // +Z, -Z

        // ClipSpace 깊이 계산
        float4 LightViewPos = mul(WorldPosition4, p.ViewMatrix[face]);        // 월드 → 라이트(큐브 face) 공간
        float4 clipPos = mul(LightViewPos, p.ProjectionMatrix);   // 라이트 공간 -> Clip space

        //FIXME : bias 적용
        // NDC 깊이 (0~1) 추출
        //float refDepth = clipPos.z / clipPos.w - bias;
        float refDepth = clipPos.z / clipPos.w;
        
        // Bias 적용
        float NdotL = saturate(dot(normalize(WorldNormal), normalize(PointLights[TargetIndex].Position - WorldPosition)));
        
        float ViewSpaceDepth = length(LightViewPos.xyz);
        
        float Bias = p.ShadowInfo.ShadowBias * 0.0005f; // 기본 Bias는 작게
        float SlopeBias = p.ShadowInfo.ShadowSlopeBias * (1.0 - NdotL) * 0.005f;
        Bias += SlopeBias;
        Bias += BiasStep * (1.0 - saturate(ViewSpaceDepth / p.Radius));
        
        refDepth = saturate(refDepth - Bias);
        
        if (p.ShadowInfo.bUseShadowPCF == 0)
        {
            Result = SamplePointLIghtShadowMap(ShadowResolutionLevel, float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel]), refDepth).r;
        }
        else
        {
            Result = SamplePointLightShadowMapPCF
            (
                ShadowResolutionLevel,
                LightDirection,
                ShadowMapIndices[ShadowResolutionLevel],
                refDepth
            ).r;            
        }
        ShadowMapIndices[ShadowResolutionLevel]++;
    }
    else if (bIsDirectional)
    {
        uint CascadeIndex = 0;
        
        float4x4 LightViewMatrix = DirectionalLights[TargetIndex].ViewMatrix[CascadeIndex];
        float4x4 LightProjectionMatrix = DirectionalLights[TargetIndex].ProjectionMatrix[CascadeIndex];
        float4 LightViewPos = mul(float4(WorldPosition, 1.0f), LightViewMatrix);
        float4 LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);

        float2 ShadowMapTexCoord = {
            0.5f + (LightClipSpacePos.x / LightClipSpacePos.w) / 2.f,
            0.5f - (LightClipSpacePos.y / LightClipSpacePos.w) / 2.f
        };
        
        FShadowInfo ShadowInfo = DirectionalLights[TargetIndex].ShadowInfo;
        
        float NdotL = saturate(dot(normalize(WorldNormal), normalize(DirectionalLights[TargetIndex].Direction)));

        float bias = DirectionalLights[TargetIndex].ShadowInfo.ShadowBias * 0.005f; // 기본 Bias는 작게
        float slopeBias = DirectionalLights[TargetIndex].ShadowInfo.ShadowSlopeBias * (1.0 - NdotL) * 0.01f;
        
        bias += slopeBias;
        
        float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
        LightDistance -= bias;
        
        
        bool IsInX = ShadowMapTexCoord.x < 0 || ShadowMapTexCoord.x > 1;
        bool IsInY = ShadowMapTexCoord.y < 0 || ShadowMapTexCoord.y > 1;
    
        if (IsInX || IsInY || LightDistance > 1)
            return 1.0f;

        FDirectionalLightInfo LightInfo = DirectionalLights[TargetIndex];
        if (LightInfo.ShadowInfo.bUseShadowPCF == 0)
        {
            Result = SampleDirectionalShadowMap(ShadowResolutionLevel, float3(ShadowMapTexCoord.x, ShadowMapTexCoord.y, ShadowMapIndices[ShadowResolutionLevel]), LightDistance).r;
        }
        else
        {
            Result = SampleDirectionalShadowMapPCF(ShadowResolutionLevel, float3(ShadowMapTexCoord.x, ShadowMapTexCoord.y, ShadowMapIndices[ShadowResolutionLevel]), LightDistance).r;
        }
            
        ShadowMapIndices[ShadowResolutionLevel]++;
    }
    else if (bIsSpot)
    {
        float4x4 LightViewMatrix = SpotLights[TargetIndex].ViewMatrix;
        float4x4 LightProjectionMatrix = SpotLights[TargetIndex].ProjectionMatrix;
        float4 LightViewPos = mul(float4(WorldPosition, 1.0f), LightViewMatrix);
        float4 LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);

        float2 ShadowMapTexCoord = {
            0.5f + (LightClipSpacePos.x / LightClipSpacePos.w) / 2.f,
            0.5f - (LightClipSpacePos.y / LightClipSpacePos.w) / 2.f
        };
        float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
        
        bool IsInX = ShadowMapTexCoord.x < 0 || ShadowMapTexCoord.x > 1;
        bool IsInY = ShadowMapTexCoord.y < 0 || ShadowMapTexCoord.y > 1;
    
        if (IsInX || IsInY || LightDistance > 1)
            return 1.0f;

        FSpotLightInfo LightInfo = SpotLights[TargetIndex];
        
        float NdotL = saturate(dot(normalize(WorldNormal), normalize(SpotLights[TargetIndex].Direction)));
        
        float Bias = SpotLights[TargetIndex].ShadowInfo.ShadowBias * 0.0005f; // 기본 Bias는 작게
        float SlopeBias = SpotLights[TargetIndex].ShadowInfo.ShadowSlopeBias * (1.0 - NdotL) * 0.005f;
        Bias += SlopeBias;
        
        LightDistance = saturate(LightDistance - Bias);
            
        if (LightInfo.ShadowInfo.bUseShadowPCF == 0)
        {
            Result = SampleSpotLightShadowMap(ShadowResolutionLevel, float3(ShadowMapTexCoord.x, ShadowMapTexCoord.y, ShadowMapIndices[ShadowResolutionLevel]), LightDistance).r;
        }
        else
        {
            Result = SampleSpotLightShadowMapPCF(ShadowResolutionLevel, float3(ShadowMapTexCoord.x, ShadowMapTexCoord.y, ShadowMapIndices[ShadowResolutionLevel]), LightDistance).r;
        }
        ShadowMapIndices[ShadowResolutionLevel]++;
    }
    
    return Result;
}

float3 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, float3 SpecularColor, float Shininess, out float ShadowMapLight)
{
    ShadowMapLight = 0;
    uint ShadowMapLightCount = 0;

    float3 LightColor = float3(0.0, 0.0, 0.0);
    float3 FinalColor = float3(0.0, 0.0, 0.0);

    uint LightIndex = 0;
    
    uint LightCounts;
    uint pStride;

    uint ShadowMapIndices[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    // Light (Dir -> Point -> Spot)순서 바뀌면 위험함.
    DirectionalLights.GetDimensions(LightCounts, pStride);
    for (int k = 0; k < LightCounts; k++)
    {
        LightColor = DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight = GetLightFromShadowMap(WorldPosition, WorldNormal, LightIndex, ShadowMapIndices);
        
        LightColor *= ShadowMapLight;
        FinalColor += LightColor;
        
        LightIndex++;
    }
    
    for (int i = 0; i < 8; i++)
    {
        ShadowMapIndices[i] = 0;
    }
    
    
    PointLights.GetDimensions(LightCounts, pStride);
    for (int i = 0; i < LightCounts; i++)
    {
        LightColor = PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight = GetLightFromShadowMap(WorldPosition, WorldNormal, LightIndex, ShadowMapIndices);
        
        LightColor *= ShadowMapLight;
        FinalColor += LightColor;
        
        LightIndex++;
    }    

    for (int i = 0; i < 8; i++)
    {
        ShadowMapIndices[i] = 0;
    }
    
    for (int j = 0; j < SpotLightsCount; j++)
    {
        LightColor = SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight = GetLightFromShadowMap(WorldPosition, WorldNormal, LightIndex, ShadowMapIndices);
        
        LightColor *= ShadowMapLight;
        FinalColor += LightColor;
        
        LightIndex++;
    }

    for (int l = 0; l < AmbientLightsCount; l++)
    {
        FinalColor += AmbientLights[l].AmbientColor.rgb * DiffuseColor;
    }
    
    return FinalColor;
}
