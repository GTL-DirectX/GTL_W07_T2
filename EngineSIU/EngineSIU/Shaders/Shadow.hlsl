#include "ShaderRegisters.hlsl"
#include "Light.hlsl"

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

cbuffer VSConstants : register(b1)
{
    uint LightIndex;
    uint LightInnerIndex;
}

VS_OUTPUT mainVS(VS_INPUT_StaticMesh input)
{
    VS_OUTPUT output;
    output.UV = input.UV;
    output.Pos = mul(float4(input.Position, 1.0f), WorldMatrix);
    if (DirectionalLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex;
        output.Pos = mul(output.Pos, DirectionalLights[TargetIndex].ViewMatrix[LightInnerIndex]);
        output.Pos = mul(output.Pos, DirectionalLights[TargetIndex].ProjectionMatrix[LightInnerIndex]);
    }
    else if (DirectionalLightsCount + PointLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex - DirectionalLightsCount;
        output.Pos = mul(output.Pos, PointLights[TargetIndex].ViewMatrix[LightInnerIndex]);
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
    float NearPlane = 0.001f;
    float FarPlane = 30.0f;
    
    float DepthNDC = Input.Pos.z / Input.Pos.w;
    
    float DepthLinearized = (2.0 * NearPlane * FarPlane) / (FarPlane + NearPlane - DepthNDC * (FarPlane - NearPlane));
    float DepthNormalized = saturate((DepthLinearized - NearPlane) / (FarPlane - NearPlane));

    DepthNormalized = DepthNDC;
    
    return float4(DepthNormalized, DepthNormalized, DepthNormalized, 1.0);
}
