
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s2);

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b3)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b4)
{
    float2 UVOffset;
    float2 TexturePad0;
}

#include "Light.hlsl"

float GetLightFromShadowMap(PS_INPUT_StaticMesh Input)
{
    float BiasStep = 0.000001f;
    float MinBias = 0.0f;
    float3 LightDirection;
    // TODO - LightIndex와 ShadowMap이 일치해야됨. (매핑되어있어야됨?)
    uint LightIndex = 0;
    float4x4 LightViewMatrix;
    float4x4 LightProjectionMatrix;
    
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

    if (bIsPoint)
    {
        float ShadowSum = 0;
        for (int i = 0; i < 6; i++)
        {
            float3 LightPosition = PointLights[TargetIndex].Position;

            float3 LightToPixelDirVector = normalize(Input.WorldPosition - LightPosition);
            
            LightViewMatrix = PointLights[TargetIndex].ViewMatrix[i];
            LightProjectionMatrix = PointLights[TargetIndex].ProjectionMatrix;

            float4 LightViewPos = mul(float4(Input.WorldPosition, 1.0f), LightViewMatrix);
            float4 LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);

            float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
            //LightDistance -= bias;

            // 큐브맵 샘플링
            ShadowSum += PointShadowMap.SampleCmpLevelZero(
                ShadowSampler,
                float4(LightToPixelDirVector, TargetIndex), // 텍스처 좌표 + TargetIndex
                LightDistance
            ).r;
        }
        Result = ShadowSum / 6.0f;
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

        float4 LightViewPos = mul(float4(Input.WorldPosition, 1.0f), LightViewMatrix);
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

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

    // Diffuse
    float3 DiffuseColor = Material.DiffuseColor;
    if (Material.TextureFlag & (1 << 1))
    {
        DiffuseColor = DiffuseTexture.Sample(DiffuseSampler, Input.UV).rgb;
        DiffuseColor = SRGBToLinear(DiffuseColor);
    }

    // Normal
    float3 WorldNormal = Input.WorldNormal;
    if (Material.TextureFlag & (1 << 2))
    {
        float3 Normal = NormalTexture.Sample(NormalSampler, Input.UV).rgb;
        Normal = normalize(2.f * Normal - 1.f);
        WorldNormal = normalize(mul(mul(Normal, Input.TBN), (float3x3) InverseTransposedWorld));
    }
    
    float ShadowMapLight = GetLightFromShadowMap(Input);
    //float ShadowMapLight = GetLightFromShadowMap(Input.WorldPosition, Input.WorldNormal);
    
    // Lighting
    if (IsLit)
    {
#ifdef LIGHTING_MODEL_GOURAUD
        FinalColor = float4(Input.Color.rgb * DiffuseColor, 1.0);
#else
        float3 LitColor = Lighting(Input.WorldPosition, WorldNormal, Input.WorldViewPosition, DiffuseColor, Material.SpecularColor, Material.SpecularScalar);
        FinalColor = float4(LitColor, 1);
#endif
    }
    else
    {
        FinalColor = float4(DiffuseColor, 1);
    }
    
    if (bIsSelected)
    {
        FinalColor += float4(0.01, 0.01, 0.0, 1);
    }
    
    FinalColor *= ShadowMapLight;
    
    return FinalColor;
}
    
