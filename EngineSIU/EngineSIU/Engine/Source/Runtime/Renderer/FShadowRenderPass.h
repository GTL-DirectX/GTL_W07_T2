#pragma once
#define _TCHAR_DEFINED
#include <d3d11.h>

#include "IRenderPass.h"
#include "Container/Array.h"
#include "Define.h"


enum class EShadowDepthType : uint8;
class USpotLightComponent;
class UPointLightComponent;
class UAmbientLightComponent;
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

    void PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport, EShadowDepthType Type, int32 DSVIndex = 0);
    
    void UpdateLightIndex(uint32 index) const;
    void UpdateObjectConstant(const FMatrix& WorldMatrix) const;


private:
    void RenderMeshComponents();
    // Shader 관련 함수 (생성/해제 등)
    void CreateShader();
    void ReleaseShader();
    
private:
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    
    TArray<UDirectionalLightComponent*> DirectionalLights;
    TArray<UPointLightComponent*> PointLights;
    TArray<USpotLightComponent*> SpotLights;

private:
    // TODO : 기존의 StaticMeshComponents를 ShadowCaster, ShadowReceiver로 분리
    TArray<UStaticMeshComponent*> ShadowCasters; // 그림자 캐스팅하는 StaticMesh
    TArray<UStaticMeshComponent*> ShadowReceivers; // 그림자 받는 StaticMesh
    
    ID3D11InputLayout* InputLayout = nullptr;
    
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;

    FDXDBufferManager* BufferManager = nullptr;
    FGraphicsDevice* Graphics = nullptr;
    FDXDShaderManager* ShaderManager = nullptr;
};
