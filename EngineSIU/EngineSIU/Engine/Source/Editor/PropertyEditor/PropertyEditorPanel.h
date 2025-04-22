#pragma once

#include "Define.h"
#include "ImGUI/imgui.h"

#include "Math/Color.h"

#include "UnrealEd/EditorPanel.h"


class UActorComponent;
class UMaterial;
class AActor;
class ULightComponentBase;
class ULightComponent;
class USpotLightComponent;
class UDirectionalLightComponent;
class UPointLightComponent;
class UAmbientLightComponent;
class UProjectileMovementComponent;
class UTextComponent;
class UHeightFogComponent;
class AEditorPlayer;
class USceneComponent;
class UStaticMeshComponent;


class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const;
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const;

    void RenderForSceneComponent(USceneComponent* SceneComponent, AEditorPlayer* Player) const;
    void RenderForActor(AActor* SelectedActor, USceneComponent* TargetComponent) const;

    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const;

    void RenderForAmbientLightComponent(UAmbientLightComponent* LightComp) const;
    void RenderForDirectionalLightComponent(UDirectionalLightComponent* LightComponent) const;
    void RenderForPointLightComponent(UPointLightComponent* LightComponent) const;
    void RenderForSpotLightComponent(USpotLightComponent* LightComponent) const;

    void RenderForLightCommon(ULightComponent* LightComponent) const;

    
    void RenderForProjectileMovementComponent(UProjectileMovementComponent* ProjectileComp) const;
    void RenderForTextComponent(UTextComponent* TextComponent) const;
    
    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderCreateMaterialView();

    void RenderForExponentialHeightFogComponent(UHeightFogComponent* ExponentialHeightFogComp) const;

    template<typename T>
        requires std::derived_from<T, UActorComponent>
    T* GetTargetComponent(AActor* SelectedActor, USceneComponent* SelectedComponent);


    // 헬퍼 함수 예시
    template<typename Getter, typename Setter>
    static void DrawColorProperty(const char* label, Getter get, Setter set)
    {
        ImGui::PushItemWidth(200.0f);
        FLinearColor curr = get();
        float col[4] = { curr.R, curr.G, curr.B, curr.A };

        if (ImGui::ColorEdit4(label, col,
            ImGuiColorEditFlags_DisplayRGB |
            ImGuiColorEditFlags_NoSidePreview |
            ImGuiColorEditFlags_NoInputs |
            ImGuiColorEditFlags_Float))
        {
            set(FLinearColor(col[0], col[1], col[2], col[3]));
        }
        ImGui::PopItemWidth();
    }
    
private:
    float Width = 0, Height = 0;
    /* Material Property */
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial;
};
