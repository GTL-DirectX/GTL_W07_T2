#include "ShaderRegisters.hlsl"
#include "Light.hlsl"

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    //float4 ShadowPos : TexCOORD0;
    float2 UV : TEXCOORD1;
};

cbuffer VSConstants : register(b1)
{
    uint LightIndex;
    float NearPlane; 
    float FarPlane;
    uint PointLightIndex;
}

VS_OUTPUT mainVS(VS_INPUT_StaticMesh input)
{
    VS_OUTPUT output;
    output.UV = input.UV;
    output.Pos = mul(float4(input.Position, 1.0f), WorldMatrix);
    
    // PSM
    float4 ClipCam = mul(mul(output.Pos, ViewMatrix), ProjectionMatrix);
    float4 NDC = ClipCam / ClipCam.w;
    
    output.Pos = NDC;
    
    //output.Pos = mul(output.Pos, ViewMatrix);
    //output.Pos = mul(output.Pos, ProjectionMatrix);
    
    if (DirectionalLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex;
        output.Pos = mul(output.Pos, DirectionalLights[TargetIndex].ViewMatrix);
        output.Pos = mul(output.Pos, DirectionalLights[TargetIndex].ProjectionMatrix);
    }
    else if (DirectionalLightsCount + PointLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex - DirectionalLightsCount;
        output.Pos = mul(output.Pos, PointLights[TargetIndex].ViewMatrix[PointLightIndex]);
        output.Pos = mul(output.Pos, PointLights[TargetIndex].ProjectionMatrix);
    }
    else if (DirectionalLightsCount + PointLightsCount + SpotLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex - DirectionalLightsCount - PointLightsCount;
        output.Pos = mul(output.Pos, SpotLights[TargetIndex].ViewMatrix);
        output.Pos = mul(output.Pos, SpotLights[TargetIndex].ProjectionMatrix);
    }
    
    return output;
}

float4 mainPS(VS_OUTPUT Input) : SV_TARGET
{    
    float DepthNDC = Input.Pos.z / Input.Pos.w;
    
    float DepthLinearized = (2.0 * NearPlane * FarPlane) / (FarPlane + NearPlane - DepthNDC * (FarPlane - NearPlane));
    float DepthNormalized = saturate((DepthLinearized - NearPlane) / (FarPlane - NearPlane));
    
    DepthNormalized = DepthNDC;
    
    return float4(DepthNormalized, DepthNormalized, DepthNormalized, 1.0);
}
