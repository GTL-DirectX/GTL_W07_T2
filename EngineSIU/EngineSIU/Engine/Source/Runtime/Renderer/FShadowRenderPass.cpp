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
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Types/ShadowTypes.h"
#include "Runtime/Renderer/RendererHelpers.h"

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

    hr = ShaderManager->AddPixelShader(L"ShadowPixelShader", L"Shaders/Shadow.hlsl", "mainPS");
    if (FAILED(hr))
    {
        // MessageBox(hwnd, L"failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    VertexShader = ShaderManager->GetVertexShaderByKey(L"ShadowVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"ShadowPixelShader");
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
    for (const auto iter : TObjectRange<USceneComponent>())
    {
        if (iter->GetWorld() != GEngine->ActiveWorld)
        {
            continue;
        }
        if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(iter))
        {
            if (!Cast<UGizmoBaseComponent>(iter))
            {
                StaticMeshComponents.Add(StaticMeshComp);
            }
        }
        else if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
        {
            PointLights.Add(PointLight);
        }
        else if (USpotLightComponent* SpotLight = Cast<USpotLightComponent>(iter))
        {
            SpotLights.Add(SpotLight);
        }
        else if (UDirectionalLightComponent* DirectionalLight = Cast<UDirectionalLightComponent>(iter))
        {
            DirectionalLights.Add(DirectionalLight);
        }
    }
}

void FShadowRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport, EShadowDepthType Type, int32 DSVIndex)
{
    constexpr EResourceType VisualizationResourceType = EResourceType::ERT_ShadowMapVisualization;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(VisualizationResourceType);

    // TODO - ㅁㄴㅇ TextureArray
    ViewportResource->ClearShadowDepthStencil(Graphics->DeviceContext, Type);
    ViewportResource->ClearRenderTarget(Graphics->DeviceContext, VisualizationResourceType);
    // TODO - ㅁㄴㅇ TextureArray
    auto DSV = ViewportResource->GetShadowDepthStencil(Type)->DSVs[DSVIndex];
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, DSV);

    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerShadow);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    BufferManager->BindConstantBuffer(TEXT("FLightCount"), 0, EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FAmbientLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_AmbientLight), EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FDirectionalLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_DirectionalLight), EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FPointLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_PointLight), EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FSpotLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_SpotLight), EShaderStage::Vertex);

    // TODO Slot 임시
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Pixel);
}

void FShadowRenderPass::UpdateLightIndex(uint32 index) const
{
    FShadowLightConstants ObjectData = {};
    ObjectData.LightIndex = index;
    ObjectData.NearPlane = 0.001f;
    ObjectData.FarPlane = 200.0f;
    
    BufferManager->UpdateConstantBuffer(TEXT("FShadowLightConstants"), ObjectData);
}

void FShadowRenderPass::UpdateObjectConstant(const FMatrix& WorldMatrix) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    
    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FShadowRenderPass::RenderMeshComponents()
{
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
    int LightIndex = 0;

    for (; LightIndex < DirectionalLights.Num(); LightIndex++)
    {
        // auto TargetIndex = (DirectionalLights.Num()) - LightIndex;

        PrepareRenderState(Viewport, EShadowDepthType::ESDT_Directional);    
        UpdateLightIndex(LightIndex);
        RenderMeshComponents();
    }

    for (; LightIndex < DirectionalLights.Num() + PointLights.Num(); LightIndex++)
    {
        // auto TargetIndex = (DirectionalLights.Num() + PointLights.Num()) - LightIndex;
        for (int32 i = 0; i < 6; i++)
        {
            PrepareRenderState(Viewport, EShadowDepthType::ESDT_Point, i);    
            UpdateLightIndex(LightIndex);
            RenderMeshComponents();
        }
    }

    for (; LightIndex < DirectionalLights.Num() + PointLights.Num() + SpotLights.Num(); LightIndex++)
    {
        // auto TargetIndex = (DirectionalLights.Num() + PointLights.Num() + SpotLights.Num()) - LightIndex;

        PrepareRenderState(Viewport, EShadowDepthType::ESDT_Spot);    
        UpdateLightIndex(LightIndex);
        RenderMeshComponents();
    }
    // 렌더 타겟 해제
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FShadowRenderPass::ClearRenderArr()
{
    StaticMeshComponents.Empty();
    PointLights.Empty();
    SpotLights.Empty();
    DirectionalLights.Empty();
}
