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
            // Begin Test
            else if (UAmbientLightComponent* AmbientLight = Cast<UAmbientLightComponent>(iter))
            {
                AmbientLights.Add(AmbientLight);
            }
            // End Test
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
    FLightInfoBuffer LightBufferData = {};

    int DirectionalLightsCount=0;
    int PointLightsCount=0;
    int SpotLightsCount=0;
    int AmbientLightsCount=0;
    
    for (UDirectionalLightComponent* Light : DirectionalLights)
    {
        if (DirectionalLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            LightBufferData.Directional[DirectionalLightsCount] = GetDirectionalLightInfo(Light);
            LightBufferData.Directional[DirectionalLightsCount].Direction = Light->GetDirection();
            DirectionalLightsCount++;
        }
    }

    for (UAmbientLightComponent* Light : AmbientLights)
    {
        if (AmbientLightsCount < MAX_DIRECTIONAL_LIGHT)
        {
            LightBufferData.Ambient[AmbientLightsCount] = GetAmbientLightInfo(Light);
            LightBufferData.Ambient[AmbientLightsCount].AmbientColor = Light->GetLightColor();
            AmbientLightsCount++;
        }
    }
    
    for (USpotLightComponent* Light : SpotLights)
    {
        if (SpotLightsCount < MAX_SPOT_LIGHT)
        {
            LightBufferData.SpotLights[SpotLightsCount] = GetSpotLightInfo(Light);
            LightBufferData.SpotLights[SpotLightsCount].Position = Light->GetWorldLocation();
            LightBufferData.SpotLights[SpotLightsCount].Direction = Light->GetDirection();
            SpotLightsCount++;
        }
    }

    for (UPointLightComponent* Light : PointLights)
    {
        if (PointLightsCount < MAX_POINT_LIGHT)
        {
            LightBufferData.PointLights[PointLightsCount] = GetPointLightInfo(Light);
            LightBufferData.PointLights[PointLightsCount].Position = Light->GetWorldLocation();
            PointLightsCount++;
        }
    }
    
    LightBufferData.DirectionalLightsCount = DirectionalLightsCount;
    LightBufferData.PointLightsCount = PointLightsCount;
    LightBufferData.SpotLightsCount = SpotLightsCount;
    LightBufferData.AmbientLightsCount = AmbientLightsCount;

    BufferManager->UpdateConstantBuffer(TEXT("FLightInfoBuffer"), LightBufferData);
    
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

