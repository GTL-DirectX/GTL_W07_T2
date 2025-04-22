
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
    
    row_major matrix ViewMatrix;
    row_major matrix ProjectionMatrix;
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
};

StructuredBuffer<FAmbientLightInfo> AmbientLights : register(t90);
StructuredBuffer<FDirectionalLightInfo> DirectionalLights : register(t91);
StructuredBuffer<FPointLightInfo> PointLights : register(t92);
StructuredBuffer<FSpotLightInfo> SpotLights : register(t93);

Texture2DArray<float> DirectionalShadowMap : register(t94);
TextureCubeArray<float> PointShadowMap : register(t95);
Texture2DArray<float> SpotShadowMap : register(t96);
SamplerComparisonState ShadowSampler : register(s2);

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

float GetLightFromShadowMap(float3 WorldPosition, uint LightIndex)
{
    float bias = 0.001;
    
    // TODO - LightIndex와 ShadowMap이 일치해야됨. (매핑되어있어야됨?)
    float4x4 LightViewMatrix;
    float4x4 LightProjectionMatrix;

    float BiasStep = 0.000001f;
    float MinBias = 0.0f;
    float3 LightDirection;
    
    // Light (Dir -> Point -> Spot)순서 바뀌면 위험함.
    bool bIsDirectional = (DirectionalLightsCount > LightIndex);
    bool bIsPoint = !bIsDirectional && (DirectionalLightsCount + PointLightsCount > LightIndex);
    bool bIsSpot = !bIsDirectional && !bIsPoint && (DirectionalLightsCount + PointLightsCount + SpotLightsCount > LightIndex);

    uint TargetIndex;

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

    //float2 ShadowMapTexCoord = {
    //    0.5f + (LightClipSpacePos.x / LightClipSpacePos.w) / 2.f,
    //    0.5f - (LightClipSpacePos.y / LightClipSpacePos.w) / 2.f
    //};
    
    //bool IsInX = ShadowMapTexCoord.x < 0 || ShadowMapTexCoord.x > 1;
    //bool IsInY = ShadowMapTexCoord.y < 0 || ShadowMapTexCoord.y > 1;
  
    //float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
    
    //if (IsInX || IsInY || LightDistance > 1)
    //    return 1.0f;
    
    //float NdotL = dot(normalize(WorldNormal), normalize(LightDirection));
    //float TotalBias = max(BiasStep * (1.0 - NdotL), MinBias);
    
    //LightDistance -= TotalBias;
    
    //return ShadowMap.SampleCmpLevelZero(ShadowSampler, ShadowMapTexCoord, LightDistance).r;
    
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

        // SampleCmpLevelZero 으로 비교
        float shadow = PointShadowMap.SampleCmpLevelZero(
            ShadowSampler,
            float4(LightDirection, TargetIndex),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
            refDepth
        ).r;

        Result = shadow;
    }
    else
    {
        if (bIsDirectional)
        {
            LightViewMatrix = DirectionalLights[TargetIndex].ViewMatrix;
            LightProjectionMatrix = DirectionalLights[TargetIndex].ProjectionMatrix;
        }
        else if (bIsSpot)
        {
            LightViewMatrix = SpotLights[TargetIndex].ViewMatrix;
            LightProjectionMatrix = SpotLights[TargetIndex].ProjectionMatrix;
        }

        float4 LightViewPos = mul(float4(WorldPosition, 1.0f), LightViewMatrix);
        float4 LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);
        float2 ShadowMapTexCoord = {
            0.5f + (LightClipSpacePos.x / LightClipSpacePos.w) / 2.f,
            0.5f - (LightClipSpacePos.y / LightClipSpacePos.w) / 2.f
        };
        float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
        //LightDistance -= bias;


        if (bIsDirectional)
        {
            Result = DirectionalShadowMap.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, TargetIndex), LightDistance).r;
        }
        else if (bIsSpot)
        {
            Result = SpotShadowMap.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, TargetIndex), LightDistance).r;
        }
    }
    
    return Result;
}

float3 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, float3 SpecularColor, float Shininess, out float ShadowMapLight)
{
    ShadowMapLight = 0;
    uint ShadowMapLightCount = 0;

    float3 FinalColor = float3(0.0, 0.0, 0.0);

    uint LightIndex = 0;

    // Light (Dir -> Point -> Spot)순서 바뀌면 위험함.
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        FinalColor += DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight += GetLightFromShadowMap(WorldPosition, LightIndex);
        ShadowMapLightCount++;
        LightIndex++;
    }
    
    for (int i = 0; i < PointLightsCount; i++)
    {
        FinalColor += PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight += GetLightFromShadowMap(WorldPosition, LightIndex);
        ShadowMapLightCount++;
        LightIndex++;
    }    

    for (int j = 0; j < SpotLightsCount; j++)
    {
        FinalColor += SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight += GetLightFromShadowMap(WorldPosition, LightIndex);
        ShadowMapLightCount++;
        LightIndex++;
    }

    for (int l = 0; l < AmbientLightsCount; l++)
    {
        FinalColor += AmbientLights[l].AmbientColor.rgb * DiffuseColor;
    }

    if (ShadowMapLightCount > 0)
    {
        ShadowMapLight /= ShadowMapLightCount;
    }
    else
    {
        ShadowMapLight = 1;
    }
    
    
    return FinalColor;
}
