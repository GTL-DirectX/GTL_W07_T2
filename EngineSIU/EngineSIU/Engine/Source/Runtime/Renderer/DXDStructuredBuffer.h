#pragma once

#include "HAL/PlatformType.h"
#include "Container/Array.h"
#include "UserInterface/Console.h"

class ID3D11Device;
class ID3D11Buffer;
class ID3D11ShaderResourceView;

struct FDXDStructuredBuffer
{
    
public:
    FDXDStructuredBuffer() : Buffer(nullptr), SRV(nullptr), UAV(nullptr), ElementCount(0), ElementStride(0) {}

    // StructuredBuffer 데이터.
    ID3D11Buffer* Buffer = nullptr;
    // StructuredBuffer에 대한 SRV.
    ID3D11ShaderResourceView* SRV;
    ID3D11UnorderedAccessView* UAV;
    UINT ElementCount; // StructuredBuffer의 Element 개수.
    UINT ElementStride; // 각 Element 별 크기.
};
