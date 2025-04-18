
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);
SamplerComparisonState ShadowSampler : register(s2);

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D ShadowMap : register(t104);

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
    float bias = 0.001f;

    // TODO - LightIndex와 ShadowMap이 일치해야됨. (매핑되어있어야됨?)
    uint LightIndex = 0;
    float4x4 LightViewMatrix;
    float4x4 LightProjectionMatrix;
    if (DirectionalLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex;

        LightViewMatrix = Directional[TargetIndex].ViewMatrix;
        LightProjectionMatrix = Directional[TargetIndex].ProjectionMatrix;
    }
    else if (DirectionalLightsCount + PointLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex - DirectionalLightsCount;
        LightViewMatrix = PointLights[TargetIndex].ViewMatrix;
        LightProjectionMatrix = PointLights[TargetIndex].ProjectionMatrix;
    }
    else if (DirectionalLightsCount + PointLightsCount + SpotLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex - DirectionalLightsCount - PointLightsCount;
        LightViewMatrix = SpotLights[TargetIndex].ViewMatrix;
        LightProjectionMatrix = SpotLights[TargetIndex].ProjectionMatrix;
    }
    
    float4 LightViewPos = mul(Input.WorldPosition, LightViewMatrix);
    float4 LightClipSpacePos = mul(LightViewPos, LightProjectionMatrix);
    
    float2 ShadowMapTexCoord = {
        0.5f + LightClipSpacePos.x / LightClipSpacePos.w / 2.f,
        0.5f - LightClipSpacePos.y / LightClipSpacePos.w / 2.f
    };
    float LightDistance = LightClipSpacePos.z / LightClipSpacePos.w;
    LightDistance -= bias;
    
    return ShadowMap.SampleCmpLevelZero(ShadowSampler, ShadowMapTexCoord, LightDistance).r;
}

float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

    float ShadowMapLight = GetLightFromShadowMap(Input);


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
    
    // Lighting
    if (IsLit)
    {
#ifdef LIGHTING_MODEL_GOURAUD
        FinalColor = float4(Input.Color.rgb * DiffuseColor, 1.0);
#else
        float3 LitColor = Lighting(Input.WorldPosition, WorldNormal, Input.WorldViewPosition, DiffuseColor, Material.SpecularColor, Material.SpecularScalar).rgb;
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
