
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4



struct FShadowInfo
{
    float ShadowResolutionScale;
    float ShadowBias;
    float ShadowSlopeBias;
    float ShadowSharpen;
    uint ShadowResolutionLevel;
    float3 Padding;
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
    
    row_major matrix ViewMatrix;
    row_major matrix ProjectionMatrix;
    
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

Texture2DArray<float> DirectionalShadowMapLevel1 : register(t70);
Texture2DArray<float> DirectionalShadowMapLevel2 : register(t71);
Texture2DArray<float> DirectionalShadowMapLevel3 : register(t72);
Texture2DArray<float> DirectionalShadowMapLevel4 : register(t73);
Texture2DArray<float> DirectionalShadowMapLevel5 : register(t74);
Texture2DArray<float> DirectionalShadowMapLevel6 : register(t75);
Texture2DArray<float> DirectionalShadowMapLevel7 : register(t76);
Texture2DArray<float> DirectionalShadowMapLevel8 : register(t77);

TextureCubeArray<float> PointShadowMapLevel1 : register(t80);
TextureCubeArray<float> PointShadowMapLevel2 : register(t81);
TextureCubeArray<float> PointShadowMapLevel3 : register(t82);
TextureCubeArray<float> PointShadowMapLevel4 : register(t83);
TextureCubeArray<float> PointShadowMapLevel5 : register(t84);
TextureCubeArray<float> PointShadowMapLevel6 : register(t85);
TextureCubeArray<float> PointShadowMapLevel7 : register(t86);
TextureCubeArray<float> PointShadowMapLevel8 : register(t87);

Texture2DArray<float> SpotShadowMapLevel1 : register(t90);
Texture2DArray<float> SpotShadowMapLevel2 : register(t91);
Texture2DArray<float> SpotShadowMapLevel3 : register(t92);
Texture2DArray<float> SpotShadowMapLevel4 : register(t93);
Texture2DArray<float> SpotShadowMapLevel5 : register(t94);
Texture2DArray<float> SpotShadowMapLevel6 : register(t95);
Texture2DArray<float> SpotShadowMapLevel7 : register(t96);
Texture2DArray<float> SpotShadowMapLevel8 : register(t97);

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

float GetLightFromShadowMap(float3 WorldPosition, uint LightIndex, inout uint ShadowMapIndices[8], float4 ShadowPos)
{
    float bias = 0.001;

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
    
    //float NdotL = dot(normalize(WorldNormal), normalize(LightDirection));
    //float TotalBias = max(BiasStep * (1.0 - NdotL), MinBias);
    
    //LightDistance -= TotalBias;
    
    //return ShadowMap.SampleCmpLevelZero(ShadowSampler, ShadowMapTexCoord, LightDistance).r;

    //PSM
    //WorldPosition = mul(float4(WorldPosition, 1.0f), InvViewMatrix);
    
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

        uint ShadowResolutionLevel = p.ShadowInfo.ShadowResolutionLevel;
        if (ShadowResolutionLevel == 1)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel1.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 2)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel2.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 3)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel3.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 4)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel4.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 5)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel5.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 6)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel6.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 7)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel7.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else if (ShadowResolutionLevel == 8)
        {
            // SampleCmpLevelZero 으로 비교
            Result = PointShadowMapLevel8.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightDirection, ShadowMapIndices[ShadowResolutionLevel - 1]),  // dir.xyzw: (방향벡터, 큐브맵 array 인덱스)
                refDepth
            ).r;
            ShadowMapIndices[ShadowResolutionLevel - 1]++;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        float4 InputPosition = float4(WorldPosition, 1.0f);

        InputPosition = ShadowPos;
        
        float4 LightViewPos;
        float4 LightClipSpacePos;
        if (bIsDirectional)
        {
            float4x4 LightViewMatrix = DirectionalLights[TargetIndex].ViewMatrix;
            float4x4 LightProjectionMatrix = DirectionalLights[TargetIndex].ProjectionMatrix;
            LightViewPos = mul(InputPosition, LightViewMatrix);
            LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);
        }
        else if (bIsSpot)
        {
            float4x4 LightViewMatrix = SpotLights[TargetIndex].ViewMatrix;
            float4x4 LightProjectionMatrix = SpotLights[TargetIndex].ProjectionMatrix;
            LightViewPos = mul(InputPosition, LightViewMatrix);
            LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);
        }

        float2 ShadowMapTexCoord = {
            0.5f + (LightClipSpacePos.x / LightClipSpacePos.w) / 2.f,
            0.5f - (LightClipSpacePos.y / LightClipSpacePos.w) / 2.f
        };
        float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
        
        bool IsInX = ShadowMapTexCoord.x < 0 || ShadowMapTexCoord.x > 1;
        bool IsInY = ShadowMapTexCoord.y < 0 || ShadowMapTexCoord.y > 1;
    
        if (IsInX || IsInY || LightDistance > 1)
            return 1.0f;

        if (bIsSpot)
        {
            FSpotLightInfo LightInfo = SpotLights[TargetIndex];

            uint ShadowResolutionLevel = LightInfo.ShadowInfo.ShadowResolutionLevel;
            if (ShadowResolutionLevel == 1)
            {
                Result = SpotShadowMapLevel1.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 2)
            {
                Result = SpotShadowMapLevel2.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 3)
            {
                Result = SpotShadowMapLevel3.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 4)
            {
                Result = SpotShadowMapLevel4.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 5)
            {
                Result = SpotShadowMapLevel5.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 6)
            {
                Result = SpotShadowMapLevel6.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 7)
            {
                Result = SpotShadowMapLevel7.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 8)
            {
                Result = SpotShadowMapLevel8.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else
            {
                return 1;
            }
        }
        else if (bIsDirectional)
        {
            FDirectionalLightInfo LightInfo = DirectionalLights[TargetIndex];
            uint ShadowResolutionLevel = LightInfo.ShadowInfo.ShadowResolutionLevel;

            if (ShadowResolutionLevel == 1)
            {
                Result = DirectionalShadowMapLevel1.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 2)
            {
                Result = DirectionalShadowMapLevel2.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 3)
            {
                Result = DirectionalShadowMapLevel3.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 4)
            {
                Result = DirectionalShadowMapLevel4.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 5)
            {
                Result = DirectionalShadowMapLevel5.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 6)
            {
                Result = DirectionalShadowMapLevel6.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 7)
            {
                Result = DirectionalShadowMapLevel7.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else if (ShadowResolutionLevel == 8)
            {
                Result = DirectionalShadowMapLevel8.SampleCmpLevelZero(ShadowSampler, float3(ShadowMapTexCoord, ShadowMapIndices[ShadowResolutionLevel - 1]), LightDistance).r;
                ShadowMapIndices[ShadowResolutionLevel - 1]++;
            }
            else
            {
                return 1;
            }
        }
    }
    
    return Result;
}

float3 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor, float3 SpecularColor, float Shininess, float4 ShadowPos)
{
    float ShadowMapLight = 0;
    uint ShadowMapLightCount = 0;

    float3 LightColor = float3(0.0, 0.0, 0.0);
    float3 FinalColor = float3(0.0, 0.0, 0.0);

    uint LightIndex = 0;

    uint ShadowMapIndices[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    // Light (Dir -> Point -> Spot)순서 바뀌면 위험함.
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        LightColor = DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight = GetLightFromShadowMap(WorldPosition, LightIndex, ShadowMapIndices, ShadowPos);
        
        LightColor *= ShadowMapLight;
        FinalColor += LightColor;
        
        LightIndex++;
    }

    for (int i = 0; i < 8; i++)
    {
        ShadowMapIndices[i] = 0;
    }
    for (int i = 0; i < PointLightsCount; i++)
    {
        LightColor = PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor, SpecularColor, Shininess);
        ShadowMapLight = GetLightFromShadowMap(WorldPosition, LightIndex, ShadowMapIndices, ShadowPos);
        
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
        ShadowMapLight = GetLightFromShadowMap(WorldPosition, LightIndex, ShadowMapIndices, ShadowPos);
        
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
