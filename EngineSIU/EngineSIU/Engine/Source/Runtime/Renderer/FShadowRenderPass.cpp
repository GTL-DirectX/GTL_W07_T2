#include "FShadowRenderPass.h"

#include "UnrealEd/EditorViewportClient.h"

#include "EngineLoop.h"
#include "Engine/EditorEngine.h"

#include "UnrealClient.h"
#include "BaseGizmos/GizmoBaseComponent.h"

#include "UObject/Casts.h"
#include "UObject/UObjectIterator.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "Components/StaticMeshComponent.h"
#include "Types/ShadowTypes.h"

FShadowRenderPass::FShadowRenderPass()
    : InputLayout(nullptr)
    , VertexShader(nullptr)
    , BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FShadowRenderPass::~FShadowRenderPass()
{
    ReleaseShader();
}

void FShadowRenderPass::CreateShader()
{
    // TODO: Layout 필요없는거 (Position 빼고 필요없음) 삭제
    D3D11_INPUT_ELEMENT_DESC ShadowLayoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    HRESULT hr = ShaderManager->AddVertexShaderAndInputLayout(L"ShadowVertexShader", L"Shaders/Shadow.hlsl", "mainVS", ShadowLayoutDesc, ARRAYSIZE(ShadowLayoutDesc));
    if (FAILED(hr))
    {
        // MessageBox(hwnd, L"failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }    

    hr = ShaderManager->AddPixelShader(L"ShadowShader", L"Shaders/ShadowMapVisualizationPixelShader.hlsl", "mainPS");
    if (FAILED(hr))
    {
        // MessageBox(hwnd, L"failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    VertexShader = ShaderManager->GetVertexShaderByKey(L"ShadowVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"ShadowShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"ShadowVertexShader");
}

void FShadowRenderPass::ReleaseShader()
{
    
}

void FShadowRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateShader();
}

void FShadowRenderPass::PrepareRender()
{
    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (!Cast<UGizmoBaseComponent>(iter) && iter->GetWorld() == GEngine->ActiveWorld)
        {
            StaticMeshComponents.Add(iter);
        }
    }
}

void FShadowRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport) 
{
    constexpr EResourceType ResourceType = EResourceType::ERT_ShadowMapVisualization;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    // TODO: Light 개수에 따라 SRV, DSV 따로 해줘야됨.
    ViewportResource->ClearDepthStencil(Graphics->DeviceContext, EDepthType::EDT_ShadowDepth);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, ViewportResource->GetDepthStencil(EDepthType::EDT_ShadowDepth)->DSV);

    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerShadow);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    BufferManager->BindConstantBuffer("FLightInfoBuffer", 0, EShaderStage::Vertex);

    // TODO Slot 임시
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Pixel);
}

void FShadowRenderPass::UpdateLightIndex(uint32 index) const
{
    FShadowLightConstants ObjectData = {};
    ObjectData.LightIndex = index;
    ObjectData.NearPlane = 0.001f;
    ObjectData.FarPlane = 30.0f;
    
    BufferManager->UpdateConstantBuffer(TEXT("FShadowLightConstants"), ObjectData);
}

void FShadowRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    
    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FShadowRenderPass::RenderPrimitive(OBJ::FStaticMeshRenderData* RenderData) const
{
    UINT Stride = sizeof(FStaticMeshVertex);
    UINT Offset = 0;
    
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &RenderData->VertexBuffer, &Stride, &Offset);

    if (RenderData->IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(RenderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    if (RenderData->MaterialSubsets.Num() == 0)
    {
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // TODO: Temp - Light만큼 DSV 그리기
    for (int i = 0; i < 1; i++)
    {
        PrepareRenderState(Viewport);
    
        UpdateLightIndex(i);
    
        // Draw Component
        for (UStaticMeshComponent* Comp : StaticMeshComponents)
        {
            if (!Comp || !Comp->GetStaticMesh())
            {
                continue;
            }
    
            OBJ::FStaticMeshRenderData* RenderData = Comp->GetStaticMesh()->GetRenderData();
            if (RenderData == nullptr)
            {
                continue;
            }
                
            FMatrix WorldMatrix = Comp->GetWorldMatrix();
        
            UpdateObjectConstant(WorldMatrix);
        
            RenderPrimitive(RenderData);
        }
    }
    // 렌더 타겟 해제
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FShadowRenderPass::ClearRenderArr()
{
    StaticMeshComponents.Empty();
}
