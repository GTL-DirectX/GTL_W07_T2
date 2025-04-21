#include "Define.h"
#include "UObject/Casts.h"
#include "UpdateLightBufferPass.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Components/Light/LightComponentBase.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FUpdateLightBufferPass::~FUpdateLightBufferPass()
{
}

void FUpdateLightBufferPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
}

void FUpdateLightBufferPass::PrepareRender()
{
    // World에서 모든 LightComponent를 가져와 배열 채우기.
    // 여기서 Light를 Info로 안 바꾸는 이유는 추후 Culling을 위해서.
    for (const auto iter : TObjectRange<ULightComponentBase>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            if (UPointLightComponent* PointLight = Cast<UPointLightComponent>(iter))
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
            else if (UAmbientLightComponent* AmbientLight = Cast<UAmbientLightComponent>(iter))
            {
                AmbientLights.Add(AmbientLight);
            }
        }
    }
}

void FUpdateLightBufferPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    UpdateLightBuffer();
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    PointLights.Empty();
    SpotLights.Empty();
    DirectionalLights.Empty();
    AmbientLights.Empty();
}


void FUpdateLightBufferPass::UpdateLightBuffer() const
{
    TArray<FDirectionalLightInfo> DirectionalLightInfo = {};
    TArray<FAmbientLightInfo> AmbientLightInfo = {};
    TArray<FPointLightInfo> PointLightInfo = {};
    TArray<FSpotLightInfo> SpotLightInfo = {};
    
    for (UDirectionalLightComponent* Light : DirectionalLights)
    {
        DirectionalLightInfo.Add(GetDirectionalLightInfo(Light));
    }

    for (UAmbientLightComponent* Light : AmbientLights)
    {
        AmbientLightInfo.Add(GetAmbientLightInfo(Light));
    }
    
    for (USpotLightComponent* Light : SpotLights)
    {
        SpotLightInfo.Add(GetSpotLightInfo(Light));
    }

    for (UPointLightComponent* Light : PointLights)
    {
        PointLightInfo.Add(GetPointLightInfo(Light));
    }

    // TODO: Light 갯수 변화에 따라 동적으로 내용 변경 or 버퍼 재생성.
    BufferManager->CreateStructuredBuffer("FDirectionalLightInfo", DirectionalLightInfo, true);
    BufferManager->CreateStructuredBuffer("FAmbientLightInfo", AmbientLightInfo, true);
    BufferManager->CreateStructuredBuffer("FPointLightInfo", PointLightInfo, true);
    BufferManager->CreateStructuredBuffer("FSpotLightInfo", SpotLightInfo, true);

    FLightCount LightCount = {};
    LightCount.DirectionalLightsCount = DirectionalLightInfo.Num();
    LightCount.AmbientLightsCount = AmbientLightInfo.Num();
    LightCount.PointLightsCount = PointLightInfo.Num();
    LightCount.SpotLightsCount = SpotLightInfo.Num();
    
    BufferManager->UpdateConstantBuffer("FLightCount", LightCount);
}

FAmbientLightInfo FUpdateLightBufferPass::GetAmbientLightInfo(const UAmbientLightComponent* LightComp) const
{
    FAmbientLightInfo LightInfo = {};
    LightInfo.AmbientColor = LightComp->GetLightColor();
    return LightInfo;
}

FDirectionalLightInfo FUpdateLightBufferPass::GetDirectionalLightInfo(const UDirectionalLightComponent* LightComp) const
{
    FDirectionalLightInfo LightInfo = {};
    
    LightInfo.LightColor = LightComp->GetLightColor();
    LightInfo.Direction = LightComp->GetDirection();
    LightInfo.Intensity = LightComp->GetIntensity();
    
    return LightInfo;
}

FPointLightInfo FUpdateLightBufferPass::GetPointLightInfo(const UPointLightComponent* LightComp) const
{
    FPointLightInfo LightInfo = {};
    
    LightInfo.LightColor = LightComp->GetLightColor();
    LightInfo.Position = LightComp->GetWorldLocation();
    LightInfo.Radius = LightComp->GetRadius();
    LightInfo.Intensity = LightComp->GetIntensity();
    LightInfo.Type = LightComp->GetLightType();
    LightInfo.Attenuation = LightComp->GetAttenuation();
    
    return LightInfo;
}

FSpotLightInfo FUpdateLightBufferPass::GetSpotLightInfo(const USpotLightComponent* LightComp) const
{
    FSpotLightInfo LightInfo = {};

    LightInfo.LightColor = LightComp->GetLightColor();
    LightInfo.Position = LightComp->GetWorldLocation();
    LightInfo.Radius = LightComp->GetRadius();
    LightInfo.Intensity = LightComp->GetIntensity();
    LightInfo.Type = LightComp->GetLightType();
    LightInfo.InnerRad = LightComp->GetInnerAngle();
    LightInfo.OuterRad = LightComp->GetOuterAngle();
    LightInfo.Attenuation = LightComp->GetAttenuation();
    LightInfo.Direction = LightComp->GetDirection();
    
    return LightInfo;
}

