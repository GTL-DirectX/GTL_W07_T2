#include "ShaderRegisters.hlsl"
#include "Light.hlsl"

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

cbuffer VSConstants : register(b1)
{
    uint LightIndex : INDEX;
}

VS_OUTPUT mainVS(VS_INPUT_StaticMesh input)
{
    VS_OUTPUT output;
    output.Pos = mul(input.Position, WorldMatrix);
    if (DirectionalLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex;
        output.Pos = mul(output.Pos, Directional[TargetIndex].ViewMatrix);
        output.Pos = mul(output.Pos, Directional[TargetIndex].ProjectionMatrix);
    }
    else if (DirectionalLightsCount + PointLightsCount > LightIndex)
    {
        uint TargetIndex = LightIndex - DirectionalLightsCount;
        output.Pos = mul(output.Pos, PointLights[TargetIndex].ViewMatrix);
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
