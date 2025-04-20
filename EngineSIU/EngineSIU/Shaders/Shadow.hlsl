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
    float NearPlane; 
    float FarPlane;
    float2 Padding;
}

VS_OUTPUT mainVS(VS_INPUT_StaticMesh input)
{
    VS_OUTPUT output;
    output.UV = input.UV;
    output.Pos = mul(float4(input.Position, 1.0f), WorldMatrix);
    //output.Pos = mul(input.Position, WorldMatrix);
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
        //output.Pos.r = 0;
    }
        
    return output;
}

float4 mainPS(VS_OUTPUT Input) : SV_TARGET
{
    //return float4(1, 1, 1, 1);
    return float4(Input.Pos.z.xxx, 1);
    float NearPlane = 0.01;
    
    float FarPlane = 1000;
    
    //float DepthRaw = Input.Pos.z / Input.Pos.w;

    //float DepthNDC = DepthRaw * 2.0 - 1.0;
    
    float DepthNDC = Input.Pos.z;
    
    //float DepthLinearized = (2.0 * NearPlane * FarPlane) / (FarPlane + NearPlane - DepthNDC * (FarPlane - NearPlane));
    
    float DepthLinearized = (NearPlane * FarPlane) / (FarPlane - DepthNDC * (FarPlane - NearPlane));

    float DepthNormalized = saturate((DepthLinearized - NearPlane) / (FarPlane - NearPlane));
    
    return float4(DepthNormalized, DepthNormalized, DepthNormalized, 1.0);
}
