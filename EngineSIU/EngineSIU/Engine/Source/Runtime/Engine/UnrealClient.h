#pragma once
#include "Define.h" 
#include <d3d11.h>

#include "Container/Map.h"
#include "Renderer/Types/ShadowTypes.h"


enum class EViewScreenLocation : uint8
{
    EVL_TopLeft,
    EVL_TopRight,
    EVL_BottomLeft,
    EVL_BottomRight,
    EVL_MAX,
};

enum class EResourceType : uint8
{
    ERT_Compositing,
    ERT_Scene,
    ERT_PP_Fog,
    ERT_Editor,
    ERT_Overlay,
    ERT_PostProcessCompositing,
    ERT_ShadowMapVisualize,
    ERT_MAX,
};

enum class EDepthType : uint8
{
    EDT_Depth,
    EDT_GizmosDepth,
    EDT_MAX,
};

enum class EShadowDepthType : uint8
{
    ESDT_Directional,
    ESDT_Point,
    ESDT_Spot,
    ESDT_MAX,
};

struct FRenderTargetRHI
{
    ID3D11Texture2D* Texture2D = nullptr;
    ID3D11RenderTargetView* RTV = nullptr;
    ID3D11ShaderResourceView* SRV = nullptr;

    void Release()
    {
        if (SRV)
        {
            SRV->Release();
            SRV = nullptr;
        }
        if (RTV)
        {
            RTV->Release();
            RTV = nullptr;
        }
        if (Texture2D)
        {
            Texture2D->Release();
            Texture2D = nullptr;
        }
    }
};

struct FDepthStencilRHI
{
    ID3D11Texture2D* Texture2D = nullptr;
    ID3D11DepthStencilView* DSV = nullptr;
    ID3D11ShaderResourceView* SRV = nullptr;

    void Release()
    {
        if (SRV)
        {
            SRV->Release();
            SRV = nullptr;
        }
        if (DSV)
        {
            DSV->Release();
            DSV = nullptr;
        }
        if (Texture2D)
        {
            Texture2D->Release();
            Texture2D = nullptr;
        }
    }
};

struct FShadowDepthStencilRHI
{
    ID3D11Texture2D* Texture2D = nullptr; 
    TArray<ID3D11DepthStencilView*> DSVs;
    ID3D11ShaderResourceView* SRV = nullptr;

    uint32 ArrayCount = 0;

    void Release()
    {
        if (SRV)
        {
            SRV->Release();
            SRV = nullptr;
        }

        for (auto DSV : DSVs)
        {
            DSV->Release();
        }
        DSVs.Empty();
        
        if (Texture2D)
        {
            Texture2D->Release();
            Texture2D = nullptr;
        }
    }
};

class FViewportResource
{
public:
    FViewportResource();
    ~FViewportResource();

    void Initialize(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 NewWidth, uint32 NewHeight);

    void Release(bool bIsReSize = false);

    HRESULT CreateResource(EResourceType Type, uint32 Index = 0);
    HRESULT CreateDepthStencilResource(EDepthType Type);
    HRESULT CreateShadowDepthStencilResource(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel, uint32 ArrayCount);
    
    D3D11_VIEWPORT& GetD3DViewport() { return D3DViewport; }

    // 해당 타입의 리소스를 리턴. 없는 경우에는 생성해서 리턴.
    FRenderTargetRHI* GetRenderTarget(EResourceType Type, uint32 Index = 0);
    FDepthStencilRHI* GetDepthStencil(EDepthType Type);
    FShadowDepthStencilRHI* GetShadowDepthStencil(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel);

    bool HasRenderTarget(EResourceType Type, uint32 Index = 0) const;
    bool HasDepthStencil(EDepthType Type) const;
    bool HasShadowDepthStencil(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel) const;

    // 가지고있는 모든 리소스의 렌더 타겟 뷰를 clear
    void ClearRenderTargets(ID3D11DeviceContext* DeviceContext);
    // 지정한 타입의 렌더 타겟 뷰를 clear. 없는 경우 생성해서 clear.
    void ClearRenderTarget(ID3D11DeviceContext* DeviceContext, EResourceType Type, uint32 Index = 0);

    void ClearDepthStencils(ID3D11DeviceContext* DeviceContext);
    // 지정한 타입의 Depth Stencil 뷰를 clear. 없는 경우 생성해서 clear.
    void ClearDepthStencil(ID3D11DeviceContext* DeviceContext, EDepthType Type);

    void ClearShadowDepthStencils(ID3D11DeviceContext* DeviceContext);
    // 지정한 타입의 Shadow Depth Stencil View를 clear. 없는 경우 생성해서 clear.
    void ClearShadowDepthStencil(ID3D11DeviceContext* DeviceContext, EShadowDepthType Type, uint32 DSVIndex, EShadowResolutionLevel
                                 ShadowResolutionLevel);

    void UpdateShadowMapCapacity(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel, uint32 LightCount);
    
    std::array<float, 4> GetClearColor(EResourceType Type) const;

    uint32 GetResolution(EShadowResolutionLevel Type);

private:
    // DirectX
    D3D11_VIEWPORT D3DViewport = {};

    TMap<EResourceType, TArray<FRenderTargetRHI>> RenderTargets;
    TMap<EDepthType, FDepthStencilRHI> DepthStencils;
    TMap<EShadowDepthType, TMap<EShadowResolutionLevel, FShadowDepthStencilRHI>> ShadowDepthStencils;   // TODO: Viewport마다 동일한 Shadow를 여러번 그린다.
    
    void ReleaseResources();
    void ReleaseResource(EResourceType Type, uint32 Index = 0);
    void ReleaseDepthStencilResources();
    void ReleaseDepthStencilResource(EDepthType Type);
    void ReleaseShadowResources();
    void ReleaseShadowResource(EShadowDepthType Type, EShadowResolutionLevel ShadowResolutionLevel);

    /**
     * ClearColors 맵에는 모든 EResourceType에 대응하는 색상을
     * 이 클래스의 생성자에서 반드시 추가해야 함.
     */
    TMap<EResourceType, std::array<float, 4>> ClearColors;

    TMap<EShadowResolutionLevel, uint32> Resolutions;
};


class FViewport
{
public:
    FViewport();
    FViewport(EViewScreenLocation InViewLocation);
    ~FViewport();

    void Initialize(const FRect& InRect);
    void ResizeViewport(const FRect& InRect);
    void ResizeViewport(const FRect& Top, const FRect& Bottom, const FRect& Left, const FRect& Right);

    D3D11_VIEWPORT& GetD3DViewport() const { return ViewportResource->GetD3DViewport(); }

    EViewScreenLocation GetViewLocation() const { return ViewLocation; }

    FViewportResource* GetViewportResource() const { return ViewportResource; }

    FRect GetRect() const { return Rect; }

    bool bIsHovered(const FVector2D& InPoint) const;

private:
    FViewportResource* ViewportResource;

    EViewScreenLocation ViewLocation;   // 뷰포트 위치

    // 이 값은 화면의 크기 뿐만 아니라 위치 정보도 가지고 있음.
    FRect Rect;
};
