#pragma once

#include "IRenderPass.h"
#include "Define.h"

struct FAmbientLightInfo;
struct FDirectionalLightInfo;
struct FSpotLightInfo;
struct FPointLightInfo;

class FDXDShaderManager;
class UWorld;
class FEditorViewportClient;

class UAmbientLightComponent;
class UDirectionalLightComponent;
class UPointLightComponent;
class USpotLightComponent;

class FUpdateLightBufferPass : public IRenderPass
{
public:
    FUpdateLightBufferPass();
    virtual ~FUpdateLightBufferPass();

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRender() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;
    void UpdateLightBuffer() const;

private:
    FAmbientLightInfo GetAmbientLightInfo(const UAmbientLightComponent* LightComp) const;
    FDirectionalLightInfo GetDirectionalLightInfo(const UDirectionalLightComponent* LightComp) const;
    FPointLightInfo GetPointLightInfo(const UPointLightComponent* LightComp) const;
    FSpotLightInfo GetSpotLightInfo(const USpotLightComponent* LightComp) const;

private:
    TArray<USpotLightComponent*> SpotLights;
    TArray<UPointLightComponent*> PointLights;
    TArray<UDirectionalLightComponent*> DirectionalLights;
    TArray<UAmbientLightComponent*> AmbientLights;

    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
};
