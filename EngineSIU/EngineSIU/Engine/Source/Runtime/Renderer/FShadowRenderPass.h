#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>

#include "IRenderPass.h"
#include "Container/Array.h"
#include "Define.h"


class UDirectionalLightComponent;
class UStaticMeshComponent;

class FShadowRenderPass : public IRenderPass
{
public:
    FShadowRenderPass();
    
    virtual ~FShadowRenderPass();
    
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    
    virtual void PrepareRender() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    
    void RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData) const;

    virtual void ClearRenderArr() override;

    void PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport);

    void UpdateLightIndex(uint32 index) const;
    void UpdateObjectConstant(const FMatrix& WorldMatrix) const;

    // Shader 관련 함수 (생성/해제 등)
    void CreateShader();
    void ReleaseShader();

    void CreateSampler();
    
private:
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    TArray<UDirectionalLightComponent*> DirectionalLights;
    
    ID3D11InputLayout* InputLayout = nullptr;
    
    ID3D11VertexShader* VertexShader = nullptr;

    //ID3D11SamplerState* Sampler = nullptr;
    
    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;
};
