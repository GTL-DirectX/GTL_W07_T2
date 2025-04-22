#pragma once
#include "Launch/EngineLoop.h"

enum class EShaderSRVSlot : int8
{
    SRV_AmbientLight = 60,
    SRV_DirectionalLight = 61,
    SRV_PointLight = 62,
    SRV_SpotLight = 63,

    SRV_DirectionalShadowMapLevel1 = 70,
    SRV_DirectionalShadowMapLevel2 = 71,
    SRV_DirectionalShadowMapLevel3 = 72,
    SRV_DirectionalShadowMapLevel4 = 73,
    SRV_DirectionalShadowMapLevel5 = 74,
    SRV_DirectionalShadowMapLevel6 = 75,
    SRV_DirectionalShadowMapLevel7 = 76,
    SRV_DirectionalShadowMapLevel8 = 77,


    SRV_PointShadowMapLevel1 = 80,
    SRV_PointShadowMapLevel2 = 81,
    SRV_PointShadowMapLevel3 = 82,
    SRV_PointShadowMapLevel4 = 83,
    SRV_PointShadowMapLevel5 = 84,
    SRV_PointShadowMapLevel6 = 85,
    SRV_PointShadowMapLevel7 = 86,
    SRV_PointShadowMapLevel8 = 87,
    
    SRV_SpotShadowMapLevel1 = 90,
    SRV_SpotShadowMapLevel2 = 91,
    SRV_SpotShadowMapLevel3 = 92,
    SRV_SpotShadowMapLevel4 = 93,
    SRV_SpotShadowMapLevel5 = 94,
    SRV_SpotShadowMapLevel6 = 95,
    SRV_SpotShadowMapLevel7 = 96,
    SRV_SpotShadowMapLevel8 = 97,
    
    SRV_SceneDepth = 99,
    SRV_Scene = 100,
    SRV_PostProcess = 101,
    SRV_EditorOverlay = 102,
    SRV_Fog = 103,

    SRV_ShadowMapVisualization = 105,
    
    SRV_Viewport = 120,

    SRV_MAX = 127,
};

namespace MaterialUtils
{
    inline void UpdateMaterial(FDXDBufferManager* BufferManager, FGraphicsDevice* Graphics, const FObjMaterialInfo& MaterialInfo)
    {
        FMaterialConstants Data;
        Data.DiffuseColor = MaterialInfo.Diffuse;
        Data.TransparencyScalar = MaterialInfo.TransparencyScalar;
        
        Data.SpecularColor = MaterialInfo.Specular;
        Data.SpecularScalar = MaterialInfo.SpecularScalar;
        
        Data.EmissiveColor = MaterialInfo.Emissive;
        Data.DensityScalar = MaterialInfo.DensityScalar;
        
        Data.AmbientColor = MaterialInfo.Ambient;
        Data.TextureFlag = MaterialInfo.TextureFlag;

        BufferManager->UpdateConstantBuffer(TEXT("FMaterialConstants"), Data);
        
        // Update Textures
        if (MaterialInfo.TextureFlag & (1 << 1)) {
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.DiffuseTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        else
        {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);

        }
        
        if (MaterialInfo.TextureFlag & (1 << 2))
        {
            std::shared_ptr<FTexture> texture = FEngineLoop::ResourceManager.GetTexture(MaterialInfo.BumpTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(1, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(1, 1, &texture->SamplerState);
        }
        else {
            ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
            ID3D11SamplerState* nullSampler[1] = { nullptr };
            Graphics->DeviceContext->PSSetShaderResources(1, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(1, 1, nullSampler);
        }
    }
}
