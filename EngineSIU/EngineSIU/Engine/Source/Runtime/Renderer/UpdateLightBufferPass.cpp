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
#include "Math/JungleMath.h"
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

        // if (PointLightsCount < MAX_POINT_LIGHT)
        // {
        //     LightBufferData.PointLights[PointLightsCount] = GetPointLightInfo(Light);
        //     LightBufferData.PointLights[PointLightsCount].Position = Light->GetWorldLocation();

        //     FMatrix ViewMatrix = JungleMath::CreateViewMatrix(Light->GetWorldLocation(), Light->GetWorldLocation() + Light->GetWorldForwardVector(), FVector{ 0.0f,0.0f, 1.0f });
        //     // TODO: 임시값
        //     FMatrix ProjectionMatrix = JungleMath::CreateProjectionMatrix(FMath::DegreesToRadians(90), 1, 0.001, D3D11_FLOAT32_MAX);

        //     LightBufferData.PointLights[PointLightsCount].View = ViewMatrix;
        //     LightBufferData.PointLights[PointLightsCount].Projection = ProjectionMatrix;

        //     PointLightsCount++;
        // }
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
    
    //씬 바운딩 스피어 정보
    FVector sceneCenter = FVector(0, 0, 0);
    float sphereRadius = 100.0f;

    // 라이트 방향 벡터 가져와 단위화 및 반전
    FVector lightDir = LightComp->GetDirection().GetSafeNormal();
    FVector invDir = -lightDir;

    // 교차 지점 계산
    FVector eyePos = sceneCenter + invDir * sphereRadius;

    // 타겟 방향
    FVector targetPos = sceneCenter;

    FVector Forward = lightDir;
    FVector TempUp = FVector(0, 0, 1);

    if (abs(Forward.Dot(TempUp)) > 0.99f)
        TempUp = FVector(1, 0, 0);
    
    FVector Right = TempUp.Cross(Forward).GetSafeNormal();
    FVector UpVector = Forward.Cross(Right);

    LightInfo.LightColor = LightComp->GetLightColor();
    LightInfo.Direction = LightComp->GetDirection();
    LightInfo.Intensity = LightComp->GetIntensity();
    LightInfo.View = JungleMath::CreateViewMatrix(eyePos, targetPos, UpVector);
    // TODO : 임의값
    LightInfo.Projection = JungleMath::CreateOrthoProjectionMatrix(100, 100, 0.1f, 150);

    LightInfo.ShadowInfo = GetShadowInfo(LightComp);

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
    LightInfo.ShadowInfo = GetShadowInfo(LightComp);


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
    LightInfo.InnerRad = FMath::DegreesToRadians(LightComp->GetInnerAngle() * 0.5f);
    LightInfo.OuterRad = FMath::DegreesToRadians(LightComp->GetOuterAngle() * 0.5f);
    LightInfo.Attenuation = LightComp->GetAttenuation();
    LightInfo.Direction = LightComp->GetDirection();

    FVector eyePos = LightComp->GetWorldLocation();

    // 타겟 방향
    FVector targetPos = LightComp->GetWorldLocation() + LightComp->GetDirection();

    FVector Forward = targetPos - eyePos;
    FVector TempUp = FVector(0, 0, 1);

    if (abs(Forward.Dot(TempUp)) > 0.99f)
        TempUp = FVector(1, 0, 0);

    FVector Right = TempUp.Cross(Forward).GetSafeNormal();
    FVector UpVector = Forward.Cross(Right); // 진짜 Up

    FMatrix ViewMatrix = JungleMath::CreateViewMatrix(eyePos, targetPos, UpVector);
    // TODO: 임시값 (30 ~ 60값 추천이라 GPT 말함)
    FMatrix ProjectionMatrix = JungleMath::CreateProjectionMatrix(FMath::DegreesToRadians(LightComp->GetOuterAngle()), 1, 0.001, LightComp->GetRadius());

    LightInfo.View = ViewMatrix;
    LightInfo.Projection = ProjectionMatrix;

    LightInfo.ShadowInfo = GetShadowInfo(LightComp);

    return LightInfo;
}

FShadowInfo FUpdateLightBufferPass::GetShadowInfo(const ULightComponent* LightComp) const
{
    FShadowInfo ShadowInfo = {};

    ShadowInfo.ShadowResolutionScale = LightComp->GetShadowResolutionScale();
    ShadowInfo.ShadowBias = LightComp->GetShadowBias();
    ShadowInfo.ShadowSlopeBias = LightComp->GetShadowSlopeBias();
    ShadowInfo.ShadowSharpen = LightComp->GetShadowSharpen();
    
    return ShadowInfo;
}

