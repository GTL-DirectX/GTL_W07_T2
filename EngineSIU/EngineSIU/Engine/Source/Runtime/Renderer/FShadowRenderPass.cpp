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

void FShadowRenderPass::UpdateShadowMapSize(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    ViewportResource->UpdateShadowMapCapacity(EShadowDepthType::ESDT_Directional, DirectionalLights.Num());
    ViewportResource->UpdateShadowMapCapacity(EShadowDepthType::ESDT_Point,PointLights.Num());
    ViewportResource->UpdateShadowMapCapacity(EShadowDepthType::ESDT_Spot, SpotLights.Num());
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

/**
 * 
 * @param DSVIndex Resolution Level별 Index 
 */
void FShadowRenderPass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport, EShadowDepthType Type, EShadowResolutionLevel::Type ShadowResolutionLevel, bool bIsSelected, int32 DSVIndex, uint32 RenderTargetIndex)
{
    FViewportResource* ViewportResource = Viewport->GetViewportResource();

    auto* TargetShadowRHI = ViewportResource->GetShadowDepthStencil(Type, ShadowResolutionLevel);
    
    /***********************임시 추후 수정 필요/***********************/
    uint32 Resolution = ViewportResource->GetResolution(ShadowResolutionLevel);
    D3D11_VIEWPORT ShadowViewport;

    ShadowViewport.Width = static_cast<FLOAT>(Resolution);
    ShadowViewport.Height = static_cast<FLOAT>(Resolution);
    ShadowViewport.MinDepth = 0.0f;
    ShadowViewport.MaxDepth = 1.0f;
    ShadowViewport.TopLeftX = 0;
    ShadowViewport.TopLeftY = 0;
    Graphics->DeviceContext->RSSetViewports(1, &ShadowViewport);
    /***********************임시 추후 수정 필요/***********************/

    
    ViewportResource->ClearShadowDepthStencil(Graphics->DeviceContext, Type, DSVIndex, ShadowResolutionLevel);
    auto DSV = TargetShadowRHI->DSVs[DSVIndex];

    if (bIsSelected)
    {
        constexpr EResourceType VisualizationResourceType = EResourceType::ERT_ShadowMapVisualize;
        FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(VisualizationResourceType, RenderTargetIndex);
        ViewportResource->ClearRenderTarget(Graphics->DeviceContext, VisualizationResourceType);

        Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, DSV);
        Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    }
    else
    {
        Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, DSV);
        Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    }

    Graphics->DeviceContext->RSSetState(FEngineLoop::GraphicDevice.RasterizerShadow);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);

    BufferManager->BindConstantBuffer(TEXT("FLightCount"), 0, EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FAmbientLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_AmbientLight), EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FDirectionalLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_DirectionalLight), EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FPointLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_PointLight), EShaderStage::Vertex);

    BufferManager->BindStructuredBuffer(TEXT("FSpotLightInfo"), static_cast<UINT>(EShaderSRVSlot::SRV_SpotLight), EShaderStage::Vertex);

    // TODO Slot 임시
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Vertex);
    BufferManager->BindConstantBuffer("FShadowLightConstants", 1, EShaderStage::Pixel);
}

void FShadowRenderPass::UpdateLightIndex(uint32 index, uint32 PointLightIndex) const
{
    FShadowLightConstants ObjectData = {};
    ObjectData.LightIndex = index;
    ObjectData.NearPlane = 0.001f;
    ObjectData.FarPlane = 30.0f;
    ObjectData.PointLightIndex = PointLightIndex;
    
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
    UpdateShadowMapSize(Viewport);
    int LightIndex = 0;

    TMap<EShadowResolutionLevel::Type, int32> LightIndexPerResolution;
    
    for (; LightIndex < DirectionalLights.Num(); LightIndex++)
    {
        // Directional
        auto TargetIndex = LightIndex;

        auto TargetLight = DirectionalLights[TargetIndex];
        auto ShadowLevel = TargetLight->GetShadowLevel();
        if (!LightIndexPerResolution.Contains(ShadowLevel))
        {
            LightIndexPerResolution.Add(ShadowLevel, 0);
        }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        AActor* SelectedActor = Engine->GetSelectedActor();
        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();

        bool bIsSelected = false;
        if (SelectedComponent != nullptr && SelectedComponent == TargetLight)
        {
            bIsSelected = true;
        }
        else if (SelectedActor != nullptr && SelectedActor->GetComponentByClass<UDirectionalLightComponent>() != nullptr)
        {
            bIsSelected = true;
        }

        PrepareRenderState(Viewport, EShadowDepthType::ESDT_Directional, ShadowLevel, bIsSelected, LightIndexPerResolution[ShadowLevel]);
        UpdateLightIndex(LightIndex);
        RenderMeshComponents();
        TargetLight->SetSliceIndex(LightIndexPerResolution[ShadowLevel]);
        
        LightIndexPerResolution[ShadowLevel]++;
    }
    LightIndexPerResolution.Empty();

    for (; LightIndex < DirectionalLights.Num() + PointLights.Num(); LightIndex++)
    {
        auto TargetLightIndex = (LightIndex - (DirectionalLights.Num()));
        
        
        auto TargetLight = PointLights[TargetLightIndex];
        auto ShadowLevel = TargetLight->GetShadowLevel();
        if (!LightIndexPerResolution.Contains(ShadowLevel))
        {
            LightIndexPerResolution.Add(ShadowLevel, 0);
        }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        AActor* SelectedActor = Engine->GetSelectedActor();
        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();

        bool bIsSelected = false;
        if (SelectedComponent != nullptr && SelectedComponent == TargetLight)
        {
            bIsSelected = true;
        }
        else if (SelectedActor != nullptr && SelectedActor->GetComponentByClass<UPointLightComponent>() != nullptr)
        {
            bIsSelected = true;
        }
        
        TargetLight->SetSliceIndex(LightIndexPerResolution[ShadowLevel]);
        for (int32 i = 0; i < 6; i++)
        {
            PrepareRenderState(Viewport, EShadowDepthType::ESDT_Point, ShadowLevel, bIsSelected, LightIndexPerResolution[ShadowLevel], i);
            UpdateLightIndex(LightIndex, i);
            RenderMeshComponents();
            LightIndexPerResolution[ShadowLevel]++;
        }
    }
    LightIndexPerResolution.Empty();
    
    for (; LightIndex < DirectionalLights.Num() + PointLights.Num() + SpotLights.Num(); LightIndex++)
    {
        // SpotLight
        auto TargetIndex = LightIndex - (DirectionalLights.Num() + PointLights.Num());

        auto TargetLight = SpotLights[TargetIndex];
        auto ShadowLevel = TargetLight->GetShadowLevel();
        if (!LightIndexPerResolution.Contains(ShadowLevel))
        {
            LightIndexPerResolution.Add(ShadowLevel, 0);
        }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        AActor* SelectedActor = Engine->GetSelectedActor();
        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();

        bool bIsSelected = false;
        if (SelectedComponent != nullptr && SelectedComponent == TargetLight)
        {
            bIsSelected = true;
        }
        else if (SelectedActor != nullptr && SelectedActor->GetComponentByClass<USpotLightComponent>() != nullptr)
        {
            bIsSelected = true;
        }

        PrepareRenderState(Viewport, EShadowDepthType::ESDT_Spot, ShadowLevel, bIsSelected, LightIndexPerResolution[ShadowLevel]);
        UpdateLightIndex(LightIndex);
        RenderMeshComponents();
        TargetLight->SetSliceIndex(LightIndexPerResolution[ShadowLevel]);
        
        LightIndexPerResolution[ShadowLevel]++;
    }
    LightIndexPerResolution.Empty();
    
    // 렌더 타겟 해제
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    Graphics->DeviceContext->RSSetViewports(1, &Viewport->GetD3DViewport());
}

void FShadowRenderPass::ClearRenderArr()
{
    StaticMeshComponents.Empty();
    PointLights.Empty();
    SpotLights.Empty();
    DirectionalLights.Empty();
}
