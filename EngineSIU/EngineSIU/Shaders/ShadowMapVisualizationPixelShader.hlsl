struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

cbuffer VSConstants : register(b1)
{
    uint LightIndex : INDEX;
    float NearPlane : NEAR_PLANE;
    float FarPlane : FAR_PLANE;
    float Padding : PADDING;
}

float4 mainPS(PS_INPUT Input) : SV_TARGET
{
    float DepthRaw = Input.Position.z;

    float DepthNDC = DepthRaw * 2.0 - 1.0;
    
    float DepthLinearized = (2.0 * NearPlane * FarPlane) / (FarPlane + NearPlane - DepthNDC * (FarPlane - NearPlane));

    float DepthNormalized = saturate((DepthLinearized - NearPlane) / (FarPlane - NearPlane));

    return float4(DepthNormalized, DepthNormalized, DepthNormalized, 1.0);
}
