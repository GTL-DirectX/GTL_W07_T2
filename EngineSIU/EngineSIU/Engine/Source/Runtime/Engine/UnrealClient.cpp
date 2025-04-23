#include "UnrealClient.h"

#include "EngineLoop.h"
#include <array>

#include "Components/Light/LightComponent.h"
#include "Engine/EditorEngine.h"

FViewportResource::FViewportResource()
{
    ClearColors.Add(EResourceType::ERT_Compositing, { 0.f, 0.f, 0.f, 1.f });
    ClearColors.Add(EResourceType::ERT_Scene,  { 0.025f, 0.025f, 0.025f, 1.0f });
    ClearColors.Add(EResourceType::ERT_PP_Fog, { 0.f, 0.f, 0.f, 0.f });
    ClearColors.Add(EResourceType::ERT_Editor, { 0.f, 0.f, 0.f, 0.f });
    ClearColors.Add(EResourceType::ERT_Overlay, { 0.f, 0.f, 0.f, 0.f });
    ClearColors.Add(EResourceType::ERT_PostProcessCompositing, { 0.f, 0.f, 0.f, 0.f });
    ClearColors.Add(EResourceType::ERT_ShadowMapVisualize, { 0.f, 0.f, 1.f, 1.f });

    Resolutions.Add(EShadowResolutionLevel::UltraLow, 64);
    Resolutions.Add(EShadowResolutionLevel::VeryLow, 128);
    Resolutions.Add(EShadowResolutionLevel::Low, 256);
    Resolutions.Add(EShadowResolutionLevel::Medium, 512);
    Resolutions.Add(EShadowResolutionLevel::High, 1024);
    Resolutions.Add(EShadowResolutionLevel::VeryHigh, 2048);
    Resolutions.Add(EShadowResolutionLevel::UltraHigh, 4096);
    Resolutions.Add(EShadowResolutionLevel::Extreme, 8192);
}

FViewportResource::~FViewportResource()
{
    Release();

    ClearColors.Empty();
    Resolutions.Empty();
}

void FViewportResource::Initialize(uint32 InWidth, uint32 InHeight)
{
    D3DViewport.TopLeftX = 0.f;
    D3DViewport.TopLeftY = 0.f;
    D3DViewport.Height = static_cast<float>(InHeight);
    D3DViewport.Width = static_cast<float>(InWidth);
    D3DViewport.MaxDepth = 1.0f;
    D3DViewport.MinDepth = 0.0f;

    HRESULT hr = S_OK;
    hr = CreateDepthStencilResource(EDepthType::EDT_Depth);
    if (FAILED(hr))
    {
        return;
    }
    hr = CreateDepthStencilResource(EDepthType::EDT_GizmosDepth);
    if (FAILED(hr))
    {
        return;
    }

    // Essential resources
    hr = CreateResource(EResourceType::ERT_Compositing);
    if (FAILED(hr))
    {
        return;
    }

    hr = CreateResource(EResourceType::ERT_Scene);
    if (FAILED(hr))
    {
        return;
    }
}

void FViewportResource::Resize(uint32 NewWidth, uint32 NewHeight)
{
    Release(true);

    D3DViewport.Height = static_cast<float>(NewHeight);
    D3DViewport.Width = static_cast<float>(NewWidth);

    for (auto& [Type, Resource] : DepthStencils)
    {
        CreateDepthStencilResource(Type);
    }

    for (auto& [Type, Resources] : RenderTargets)
    {
        for (int i = 0; i < Resources.Num(); i++)
        {
            CreateResource(Type, i);
        }
    }
}

void FViewportResource::Release(bool bIsReSize)
{
    ReleaseResources();
    ReleaseDepthStencilResources();
    if (!bIsReSize)
    {
        ReleaseShadowResources();
    }
}

HRESULT FViewportResource::CreateResource(EResourceType Type, uint32 Index)
{
    if (HasRenderTarget(Type, Index))
    {
        ReleaseResource(Type, Index);
    }
    
    FRenderTargetRHI NewResource;
    
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    if (Type == EResourceType::ERT_ShadowMapVisualize)
    {
        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        AActor* SelectedActor = Engine->GetSelectedActor();
        USceneComponent* SelectedComponent = Engine->GetSelectedComponent();
        ULightComponent* TargetComponent = nullptr;
        if (ULightComponent* Comp = Cast<ULightComponent>(SelectedComponent))
        {
            TargetComponent = Comp;
        }
        else if (SelectedActor != nullptr && SelectedActor->GetComponentByClass<ULightComponent>() != nullptr)
        {
            TargetComponent = SelectedActor->GetComponentByClass<ULightComponent>();
        }
        if (TargetComponent == nullptr)
        {
            return E_FAIL;
        }

        TextureDesc.Width = Resolutions[TargetComponent->GetShadowLevel()];
        TextureDesc.Height = Resolutions[TargetComponent->GetShadowLevel()];
    }
    else
    {
        TextureDesc.Width = static_cast<uint32>(D3DViewport.Width);
        TextureDesc.Height = static_cast<uint32>(D3DViewport.Height);
    }
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;
    NewResource.Texture2D = FEngineLoop::GraphicDevice.CreateTexture2D(TextureDesc, nullptr);

    D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
    RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: srgb 옵션 고려해보기
    RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = FEngineLoop::GraphicDevice.Device->CreateRenderTargetView(NewResource.Texture2D, &RTVDesc, &NewResource.RTV);
    if (FAILED(hr))
    {
        return hr;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = TextureDesc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    SRVDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D, &SRVDesc, &NewResource.SRV);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!RenderTargets.Contains(Type))
    {
        RenderTargets.Add(Type, TArray<FRenderTargetRHI>());
    }

    if (Index < RenderTargets[Type].Num())
    {
        RenderTargets[Type][Index] = NewResource;
    }
    else
    {
        RenderTargets[Type].Add(NewResource);
    }

    return hr;
}

HRESULT FViewportResource::CreateDepthStencilResource(EDepthType Type)
{
    // TODO : 나중에 Static Light, Caster, Receiver에 따라 Depth Map 캐싱데이터 재사용 하기
    if (HasDepthStencil(Type))
    {
        ReleaseDepthStencilResource(Type);
    }
    
    FDepthStencilRHI NewResource;
    
    HRESULT hr = S_OK;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = static_cast<uint32>(D3DViewport.Width);
    TextureDesc.Height = static_cast<uint32>(D3DViewport.Height);
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;
    NewResource.Texture2D = FEngineLoop::GraphicDevice.CreateTexture2D(TextureDesc, nullptr);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
    DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DSVDesc.Texture2D.MipSlice = 0;
    hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D,  &DSVDesc,  &NewResource.DSV);
    if (FAILED(hr))
    {
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    SRVDesc.Texture2D.MipLevels = 1;
    hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D,  &SRVDesc,  &NewResource.SRV);
    if (FAILED(hr))
    {
        return hr;
    }
    
    DepthStencils.Add(Type, NewResource);

    return hr;
}


bool FViewportResource::HasRenderTarget(EResourceType Type, uint32 Index) const
{
    return RenderTargets.Contains(Type) && RenderTargets[Type].Num() > Index;
}

HRESULT FViewportResource::CreateShadowDepthStencilResource(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel, uint32 ArrayCount)
{
    HRESULT hr = S_OK;

    // TODO : 나중에 Static Light, Caster, Receiver에 따라 Depth Map 캐싱데이터 재사용 하기
    if (HasShadowDepthStencil(Type, ShadowResolutionLevel) && ArrayCount <= GetShadowDepthStencil(Type, ShadowResolutionLevel)->ArrayCount)
    {
        return hr;
    }
    
    if (HasShadowDepthStencil(Type, ShadowResolutionLevel))
    {
        ReleaseShadowResource(Type, ShadowResolutionLevel);
    }

    uint32 ShadowMapWidth = Resolutions[ShadowResolutionLevel];
    uint32 ShadowMapHeight = Resolutions[ShadowResolutionLevel];
    
    FShadowDepthStencilRHI NewResource;
    
    NewResource.ArrayCount = ArrayCount;

    if (Type == EShadowDepthType::ESDT_Point)
    {
        // Texture2D 생성
        D3D11_TEXTURE2D_DESC TextureDesc;
        ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
        // TODO : Widht, Height Viewprot아닌 다른 사이즈로 하면 RTV도 수정 요함.
        TextureDesc.Width = ShadowMapWidth;
        TextureDesc.Height = ShadowMapHeight;
        TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        TextureDesc.MipLevels = 1;
        TextureDesc.ArraySize = ArrayCount * 6;
        TextureDesc.Usage = D3D11_USAGE_DEFAULT;
        TextureDesc.CPUAccessFlags = 0;
        TextureDesc.SampleDesc.Count = 1;
        TextureDesc.SampleDesc.Quality = 0;
        TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        NewResource.Texture2D = FEngineLoop::GraphicDevice.CreateTexture2D(TextureDesc, nullptr);
    
        // DSV 6개 생성
        D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
        ZeroMemory(&DSVDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
        DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY; //D3D11_DSV_DIMENSION_TEXTURE2DARRAY
        DSVDesc.Texture2DArray.MipSlice = 0;
        DSVDesc.Texture2DArray.ArraySize = 1;

        for (uint32 i = 0; i < ArrayCount * 6; i++)
        {
            DSVDesc.Texture2DArray.FirstArraySlice = i; // Slice Index
            ID3D11DepthStencilView* TempDSV = nullptr;
            hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D,  &DSVDesc, &TempDSV);
            NewResource.DSVs.Add(TempDSV);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    
        // 샘플용 SRV 생성
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
        SRVDesc.TextureCubeArray.MipLevels = 1;
        SRVDesc.TextureCubeArray.MostDetailedMip = 0;
        SRVDesc.TextureCubeArray.First2DArrayFace = 0;
        SRVDesc.TextureCubeArray.NumCubes = ArrayCount;
        
        hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D,  &SRVDesc,  &NewResource.SRV);
        if (FAILED(hr))
        {
            return hr;
        } 
    }
    else if (Type == EShadowDepthType::ESDT_Directional)
    {
        D3D11_TEXTURE2D_DESC TextureDesc;
        ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
        TextureDesc.Width = ShadowMapWidth;
        TextureDesc.Height = ShadowMapHeight;
        TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        TextureDesc.MipLevels = 0;
        TextureDesc.ArraySize = ArrayCount * CASCADE_COUNT;
        TextureDesc.Usage = D3D11_USAGE_DEFAULT;
        TextureDesc.CPUAccessFlags = 0;
        TextureDesc.SampleDesc.Count = 1;
        TextureDesc.SampleDesc.Quality = 0;
        TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        NewResource.Texture2D = FEngineLoop::GraphicDevice.CreateTexture2D(TextureDesc, nullptr);
    
        D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
        ZeroMemory(&DSVDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
        DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        DSVDesc.Texture2DArray.MipSlice = 0;
        DSVDesc.Texture2DArray.ArraySize = 1;
        for (uint32 i = 0; i < ArrayCount * CASCADE_COUNT; ++i)
        {
            DSVDesc.Texture2DArray.FirstArraySlice = i;
            ID3D11DepthStencilView* TempDSV = nullptr;
            hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D,  &DSVDesc,  &TempDSV);
            NewResource.DSVs.Add(TempDSV);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        SRVDesc.Texture2DArray.MipLevels = 1;
        SRVDesc.Texture2DArray.ArraySize = ArrayCount * CASCADE_COUNT;
        SRVDesc.Texture2DArray.FirstArraySlice = 0;
        SRVDesc.Texture2DArray.MostDetailedMip = 0;
        
        hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D,  &SRVDesc,  &NewResource.SRV);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else if (Type == EShadowDepthType::ESDT_Spot)
    {
        D3D11_TEXTURE2D_DESC TextureDesc;
        ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
        // TODO : Widht, Height Viewprot아닌 다른 사이즈로 하면 RTV도 수정 요함.
        TextureDesc.Width = ShadowMapWidth;
        TextureDesc.Height = ShadowMapHeight;
        TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        TextureDesc.MipLevels = 0;
        TextureDesc.ArraySize = ArrayCount;
        TextureDesc.Usage = D3D11_USAGE_DEFAULT;
        TextureDesc.CPUAccessFlags = 0;
        TextureDesc.SampleDesc.Count = 1;
        TextureDesc.SampleDesc.Quality = 0;
        TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        NewResource.Texture2D = FEngineLoop::GraphicDevice.CreateTexture2D(TextureDesc, nullptr);
    
        D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
        ZeroMemory(&DSVDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
        DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
        DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        DSVDesc.Texture2DArray.MipSlice = 0;
        DSVDesc.Texture2DArray.ArraySize = 1;
        for (uint32 i = 0; i < ArrayCount; ++i)
        {
            DSVDesc.Texture2DArray.FirstArraySlice = i;
            ID3D11DepthStencilView* TempDSV = nullptr;
            hr = FEngineLoop::GraphicDevice.Device->CreateDepthStencilView(NewResource.Texture2D,  &DSVDesc,  &TempDSV);
            NewResource.DSVs.Add(TempDSV);
            if (FAILED(hr))
            {
                return hr;
            }
        }
    
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        SRVDesc.Texture2DArray.MipLevels = 1;
        SRVDesc.Texture2DArray.ArraySize = ArrayCount;
        SRVDesc.Texture2DArray.FirstArraySlice = 0;
        SRVDesc.Texture2DArray.MostDetailedMip = 0;
        
        hr = FEngineLoop::GraphicDevice.Device->CreateShaderResourceView(NewResource.Texture2D,  &SRVDesc,  &NewResource.SRV);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (!ShadowDepthStencils.Contains(Type))
    {
        ShadowDepthStencils.Add(Type, TMap<EShadowResolutionLevel, FShadowDepthStencilRHI>());
    }
    ShadowDepthStencils[Type].Add(ShadowResolutionLevel, NewResource);

    return hr;
}

bool FViewportResource::HasDepthStencil(EDepthType Type) const
{
    return DepthStencils.Contains(Type);
}

bool FViewportResource::HasShadowDepthStencil(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel) const
{
    return ShadowDepthStencils.Contains(Type) && ShadowDepthStencils[Type].Contains(ShadowResolutionLevel);
}

FRenderTargetRHI* FViewportResource::GetRenderTarget(EResourceType Type, uint32 Index)
{
    uint32 StartIndex;
    if (HasRenderTarget(Type, 0))
    {
        StartIndex = RenderTargets[Type].Num();
    }
    else
    {
        StartIndex = 0;
    }
    
    for (uint32 i = StartIndex; !HasRenderTarget(Type, Index); i++)
    {
        if (FAILED(CreateResource(Type, i)))
        {
            return nullptr;
        }
    }
    return &RenderTargets[Type][Index];
}

FDepthStencilRHI* FViewportResource::GetDepthStencil(EDepthType Type)
{
    if (!DepthStencils.Contains(Type))
    {
        if (FAILED(CreateDepthStencilResource(Type)))
        {
            return nullptr;
        }
    }
    return DepthStencils.Find(Type);
}

FShadowDepthStencilRHI* FViewportResource::GetShadowDepthStencil(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel)
{
    if (!HasShadowDepthStencil(Type, ShadowResolutionLevel))
    {
        if (FAILED(CreateShadowDepthStencilResource(Type, ShadowResolutionLevel, 16)))
        {
            return nullptr;
        }
    }
    return ShadowDepthStencils[Type].Find(ShadowResolutionLevel);
}

// 기존 EDepthType::EDT_Depth를 같이 Clear하고 있었음. 고려 필요
void FViewportResource::ClearRenderTargets(ID3D11DeviceContext* DeviceContext)
{
    for (auto& [Type, Resources] : RenderTargets)
    {
        for (auto& Resource : Resources)
        {
            DeviceContext->ClearRenderTargetView(Resource.RTV, ClearColors[Type].data());
        }
    }
}

void FViewportResource::ClearRenderTarget(ID3D11DeviceContext* DeviceContext, EResourceType Type, uint32 Index)
{
    if (FRenderTargetRHI* Resource = GetRenderTarget(Type, Index))
    {
        DeviceContext->ClearRenderTargetView(Resource->RTV, ClearColors[Type].data());
    }
}

void FViewportResource::ClearDepthStencils(ID3D11DeviceContext* DeviceContext)
{
    for (auto& [Type, Resource] : DepthStencils)
    {
        DeviceContext->ClearDepthStencilView(Resource.DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void FViewportResource::ClearDepthStencil(ID3D11DeviceContext* DeviceContext, EDepthType Type)
{
    if (FDepthStencilRHI* Resource = GetDepthStencil(Type))
    {
        DeviceContext->ClearDepthStencilView(Resource->DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void FViewportResource::ClearShadowDepthStencils(ID3D11DeviceContext* DeviceContext)
{
    for (auto& [ShadowType, ShadowMapLevelMap] : ShadowDepthStencils)
    {
        for (auto& [Level, Resource] : ShadowMapLevelMap)
        {
            for (auto* DSV : Resource.DSVs)
            {
                DeviceContext->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            }
        }
    }
}

void FViewportResource::ClearShadowDepthStencil(ID3D11DeviceContext* DeviceContext, EShadowDepthType Type, uint32 DSVIndex, EShadowResolutionLevel ShadowResolutionLevel)
{
    if (FShadowDepthStencilRHI* Resource = GetShadowDepthStencil(Type, ShadowResolutionLevel))
    {
        DeviceContext->ClearDepthStencilView(Resource->DSVs[DSVIndex], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void FViewportResource::UpdateShadowMapCapacity(EShadowDepthType Type, uint32 LightCount)
{
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::UltraLow, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::VeryLow, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::Low, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::Medium, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::High, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::VeryHigh, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::UltraHigh, LightCount);
    CreateShadowDepthStencilResource(Type, EShadowResolutionLevel::Extreme, LightCount);
}

std::array<float, 4> FViewportResource::GetClearColor(EResourceType Type) const
{
    if (const std::array<float, 4>* Found = ClearColors.Find(Type))
    {
        return *Found;
    }
    return { 0.0f, 0.0f, 0.0f, 1.0f };
}

uint32 FViewportResource::GetResolution(EShadowResolutionLevel Type)
{
    return Resolutions[Type];
}

void FViewportResource::ReleaseDepthStencilResources()
{
    for (auto& [Type, Resource] : DepthStencils)
    {
        Resource.Release();
    }
}

void FViewportResource::ReleaseDepthStencilResource(EDepthType Type)
{
    if (HasDepthStencil(Type))
    {
        DepthStencils[Type].Release();
    }
}

void FViewportResource::ReleaseResources()
{
    for (auto& [Type, Resources] : RenderTargets)
    {
        for (auto& Resource : Resources)
        {
            Resource.Release();
        }
    }
}

void FViewportResource::ReleaseResource(EResourceType Type, uint32 Index)
{
    if (HasRenderTarget(Type, Index))
    {
        RenderTargets[Type][Index].Release();
    }
}

void FViewportResource::ReleaseShadowResources()
{
    for (auto& [ShadowType, ShadowMapLevelMap] : ShadowDepthStencils)
    {
        for (auto& [Level, Resource] : ShadowMapLevelMap)
        {
            Resource.Release();
        }
    }
}

void FViewportResource::ReleaseShadowResource(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel)
{
    if (HasShadowDepthStencil(Type, ShadowResolutionLevel))
    {
        ShadowDepthStencils[Type][ShadowResolutionLevel].Release();
    }
}

FViewport::FViewport()
    : FViewport(EViewScreenLocation::EVL_MAX)
{
}

FViewport::FViewport(EViewScreenLocation InViewLocation)
    : ViewportResource(new FViewportResource())
    , ViewLocation(InViewLocation) 
{
}

FViewport::~FViewport()
{
    delete ViewportResource;
}

void FViewport::Initialize(const FRect& InRect)
{
    Rect = InRect;
    const uint32 Width = static_cast<uint32>(Rect.Width);
    const uint32 Height = static_cast<uint32>(Rect.Height);

    ViewportResource->Initialize(Width, Height);
}

void FViewport::ResizeViewport(const FRect& InRect)
{
    Rect = InRect;
    const uint32 Width = static_cast<uint32>(Rect.Width);
    const uint32 Height = static_cast<uint32>(Rect.Height);

    ViewportResource->Resize(Width, Height);
}

void FViewport::ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right)
{
    switch (ViewLocation)
    {
    case EViewScreenLocation::EVL_TopLeft:
        Rect.TopLeftX = Left.TopLeftX;
        Rect.TopLeftY = Top.TopLeftY;
        Rect.Width = Left.Width;
        Rect.Height = Top.Height;
        break;
    case EViewScreenLocation::EVL_TopRight:
        Rect.TopLeftX = Right.TopLeftX;
        Rect.TopLeftY = Top.TopLeftY;
        Rect.Width = Right.Width;
        Rect.Height = Top.Height;
        break;
    case EViewScreenLocation::EVL_BottomLeft:
        Rect.TopLeftX = Left.TopLeftX;
        Rect.TopLeftY = Bottom.TopLeftY;
        Rect.Width = Left.Width;
        Rect.Height = Bottom.Height;
        break;
    case EViewScreenLocation::EVL_BottomRight:
        Rect.TopLeftX = Right.TopLeftX;
        Rect.TopLeftY = Bottom.TopLeftY;
        Rect.Width = Right.Width;
        Rect.Height = Bottom.Height;
        break;
    default:
        return;
    }
    
    const uint32 Width = static_cast<uint32>(Rect.Width);
    const uint32 Height = static_cast<uint32>(Rect.Height);
    ViewportResource->Resize(Width, Height);
}

bool FViewport::bIsHovered(const FVector2D& InPoint) const
{
    return (Rect.TopLeftX <= static_cast<float>(InPoint.X) && static_cast<float>(InPoint.X) <= Rect.TopLeftX + Rect.Width) &&
           (Rect.TopLeftY <= static_cast<float>(InPoint.Y) && static_cast<float>(InPoint.Y) <= Rect.TopLeftY + Rect.Height);
}
