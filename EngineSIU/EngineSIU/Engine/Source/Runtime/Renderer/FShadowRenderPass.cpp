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
#include "Components/Light/DirectionalLightComponent.h"
#include "Types/ShadowTypes.h"

FShadowRenderPass::FShadowRenderPass()
    : InputLayout(nullptr)
    , VertexShader(nullptr)
    //, Sampler(nullptr)
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

    VertexShader = ShaderManager->GetVertexShaderByKey(L"ShadowVertexShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"ShadowVertexShader");
}

void FShadowRenderPass::ReleaseShader()
{
    
}

void FShadowRenderPass::CreateSampler()
{
    // D3D11_SAMPLER_DESC SamplerDesc;
    // ZeroMemory(&SamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    // SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    // SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    // SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    // SamplerDesc.BorderColor[0] = 1.0f;
    // SamplerDesc.BorderColor[1] = 1.0f;
    // SamplerDesc.BorderColor[2] = 1.0f;
    // SamplerDesc.BorderColor[3] = 1.0f;
    // SamplerDesc.MinLOD = 0.f;
    // SamplerDesc.MaxLOD = 0.f;
    // SamplerDesc.MipLODBias = 0.f;
    // SamplerDesc.MaxAnisotropy = 0;
    // SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    // SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    //
    // Graphics->Device->CreateSamplerState(&SamplerDesc, &Sampler);
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
        else if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
        {
            DirectionalLights.Add(DirectionalLight);
        }
    }
}

void FShadowRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport) 
{
    constexpr EDepthType ResourceType = EDepthType::EDT_ShadowDepth;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    //FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    // TODO: Light 개수에 따라 SRV, DSV 따로 해줘야됨.
    ViewportResource->ClearDepthStencil(Graphics->DeviceContext, ResourceType);    
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, ViewportResource->GetShadowDepthStencilView());

    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerShadow);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);

    BufferManager->BindConstantBuffer("FLightInfoBuffer", 0, EShaderStage::Vertex);

    // TODO Slot 임시
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Vertex);
}

void FShadowRenderPass::UpdateLightIndex(uint32 index) const
{
    FShadowLightConstants ObjectData = {};
    ObjectData.LightIndex = index;
    
    BufferManager->UpdateConstantBuffer(TEXT("FShadowLightConstants"), ObjectData);
}

void FShadowRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    
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

    HRESULT hr;
    if (RenderData->MaterialSubsets.Num() == 0)
    {
        hr = Graphics->Device->GetDeviceRemovedReason();
        Graphics->DeviceContext->DrawIndexed(RenderData->Indices.Num(), 0, 0);
        hr = Graphics->Device->GetDeviceRemovedReason();
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        hr = Graphics->Device->GetDeviceRemovedReason();
        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
        hr = Graphics->Device->GetDeviceRemovedReason();
    }
}

void FShadowRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // TODO Temp임
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
    DirectionalLights.Empty();
}
