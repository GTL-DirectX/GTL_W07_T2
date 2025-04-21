#include "Cube.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/FLoaderOBJ.h"

ACube::ACube()
{
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
}
