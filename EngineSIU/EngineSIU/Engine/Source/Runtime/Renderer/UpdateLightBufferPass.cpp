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
#include "Editor/UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------
FUpdateLightBufferPass::FUpdateLightBufferPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
    CascadeSplits.SetNum(NUM_CASCADES);
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
    UpdateLightBuffer(Viewport);
}

void FUpdateLightBufferPass::ClearRenderArr()
{
    PointLights.Empty();
    SpotLights.Empty();
    DirectionalLights.Empty();
    AmbientLights.Empty();
}


void FUpdateLightBufferPass::UpdateLightBuffer(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    TArray<FDirectionalLightInfo> DirectionalLightInfo = {};
    TArray<FAmbientLightInfo> AmbientLightInfo = {};
    TArray<FPointLightInfo> PointLightInfo = {};
    TArray<FSpotLightInfo> SpotLightInfo = {};


    SLevelEditor* LevelEd = GEngineLoop.GetLevelEditor();

    // CascadeSplits 계산
    float Near = LevelEd->GetActiveViewportClient()->GetCameraNearClip();
    float Far = LevelEd->GetActiveViewportClient()->GetCameraFarClip();

    CalculateCascadeSplits(Near, Far);
    for (UDirectionalLightComponent* Light : DirectionalLights)
    {
        for (int CascadeIdx = 0; CascadeIdx < NUM_CASCADES; ++Cascadeidx)
        {
            float CascadeNear;
            if(CascadeIdx==0){
                CascadeNear = Near;
            }
            else {
                CascadeNear = CascadeSplits[CascadeSplits[Cascadeidx - 1]];
            }
            float CascadeFar = CascadeSplits[CascadeIdx];
            DirectionalLightInfo.Add(GetDirectionalLightInfo(Light, Viewport));
        }
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

FDirectionalLightInfo FUpdateLightBufferPass::GetDirectionalLightInfo(const UDirectionalLightComponent* LightComp, const std::shared_ptr<FEditorViewportClient>& Viewport) const
{
    FDirectionalLightInfo LightInfo = {};
    
    //씬 바운딩 스피어 정보
    //FVector sceneCenter = FVector(0, 0, 0);
    FVector sceneCenter = FVector(Viewport->GetCameraLocation());
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

    FVector lightPos = LightComp->GetWorldLocation();

    // 6면 방향 벡터
    FVector targets[6] = {
        {+1, 0, 0}, {-1, 0, 0},
        {0, +1, 0}, {0, -1, 0},
        {0, 0, +1}, {0, 0, -1}
    };
    // 각 면에 맞는 up 벡터 (Y+, Y+, Z-, Z+, Y+, Y+)
    FVector ups[6] = {
        {0, 1, 0}, {0, 1, 0},
        {0, 0, -1},{0, 0, +1},
        {0, 1, 0}, {0, 1, 0}
    };

    // face 별 뷰 행렬 구하기
    FMatrix viewMats[6];
    for (int face = 0; face < 6; ++face)
    {
        FVector eye = lightPos;
        FVector target = lightPos + targets[face];
        FVector up = ups[face];

        viewMats[face] = JungleMath::CreateViewMatrix(eye, target, up);
        LightInfo.View[face] = viewMats[face];
    }

    float fov = FMath::DegreesToRadians(90);
    float aspect = 1;
    float nearPlane = 0.01;
    float farPlane = LightComp->GetRadius();

    LightInfo.Projection = JungleMath::CreateProjectionMatrix(fov, aspect, nearPlane, farPlane);
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
    ShadowInfo.ShadowResolutionLevel = LightComp->GetShadowLevel();
    
    return ShadowInfo;
}

void FUpdateLightBufferPass::CalculateCascadeSplits(float NearPlane, float FarPlane)
{
    float SplitLambda = 0.95f;        // 0.0 = 완전 선형, 1.0 = 완전 로그
    for (int i = 0; i < NUM_CASCADES; ++i) {
        // 1-based cascade 비율
        float CascadeId = (i + 1) / static_cast<float>(NUM_CASCADES);

        // 로그 스플릿
        float LogSplit = NearPlane * FMath::Pow(FarPlane / NearPlane, CascadeId);
        // 선형 스플릿
        float LinSplit = NearPlane + (FarPlane - NearPlane) * CascadeId;

        // 혼합
        CascadeSplits[i] = FMath::Lerp(LinSplit, LogSplit, SplitLambda);

    }
}

