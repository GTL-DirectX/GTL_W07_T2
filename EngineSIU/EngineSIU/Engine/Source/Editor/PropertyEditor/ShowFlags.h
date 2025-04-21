#pragma once

namespace EEngineShowFlags
{
    enum Type : uint64
    {
        None = 0,
        SF_AABB = 1ULL << 0,
        SF_Primitives = 1ULL << 1,
        SF_BillboardText = 1ULL << 2,
        SF_UUIDText = 1ULL << 3,
        SF_Fog = 1ULL << 4
    };
}
