#pragma once
#include "Math/Vector.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectTypes.h"


class UGizmoBaseComponent;
class USceneComponent;
class FEditorViewportClient;
class UStaticMeshComponent;

class FEditorPlayer : public UObject
{
    DECLARE_CLASS(FEditorPlayer, UObject)

public:
    FEditorPlayer() = default;

    void Initialize();
    bool PickGizmo(FVector& RayOrigin, FEditorViewportClient* InActiveViewport);
    void ProcessGizmoIntersection(UStaticMeshComponent* Component, const FVector& PickPosition, FEditorViewportClient* InActiveViewport, bool& bIsPickedGizmo);
    void PickActor(const FVector& pickPosition);
    void AddControlMode();
    void AddCoordiMode();

private:
    int RayIntersectsObject(const FVector& PickPosition, USceneComponent* Component, float& HitDistance, int& IntersectCount);
    void ScreenToViewSpace(int32 ScreenX, int32 ScreenY, std::shared_ptr<FEditorViewportClient> ActiveViewport, FVector& RayOrigin);
    void PickedObjControl();
    void ControlRotation(USceneComponent* TargetComponent, UGizmoBaseComponent* Gizmo, float DeltaX, float DeltaY);
    void ControlScale(USceneComponent* TargetComponent, UGizmoBaseComponent* Gizmo, float DeltaX, float DeltaY);

    POINT LastMousePos;
    EControlMode ControlMode = CM_TRANSLATION;
    ECoordMode CoordMode = CDM_WORLD;

public:
    void SetMode(EControlMode Mode) { ControlMode = Mode; }
    EControlMode GetControlMode() const { return ControlMode; }
    ECoordMode GetCoordMode() const { return CoordMode; }
};
