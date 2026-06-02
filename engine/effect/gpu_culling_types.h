#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

// ============================================================================
// GPU Driven Rendering — C++ 数据结构
// 与 shader/GPUCulling.slangh 中的 HLSL 结构体布局保持一致
// ============================================================================

struct GPUMeshData
{
    uint32_t indexOffset      = 0;
    uint32_t vertexOffset     = 0;
    uint32_t indexCount       = 0;
    uint32_t vertexCount      = 0;
    uint32_t materialIndex    = 0;
    uint32_t topologyType     = 0;
    uint32_t indexBufferType  = 0;
    uint32_t vertexStride     = 0;
    float4   aabbMinLocal     = float4(0, 0, 0, 0);
    float4   aabbMaxLocal     = float4(0, 0, 0, 0);
};

struct GPUObjectData
{
    float4x4 worldMatrix    = float4x4::Identity();
    float4x4 normalMatrix   = float4x4::Identity();
    float4   aabbMinWorld   = float4(0, 0, 0, 0);
    float4   aabbMaxWorld   = float4(0, 0, 0, 0);
    uint32_t meshDataIndex  = 0;
    uint32_t visible        = 0;
    uint32_t padding[2]     = {0, 0};
};

struct DrawIndexedIndirectArgs
{
    uint32_t indexCountPerInstance = 0;
    uint32_t instanceCount         = 0;
    uint32_t startIndexLocation    = 0;
    int32_t  baseVertexLocation    = 0;
    uint32_t startInstanceLocation = 0;
};

struct FrustumCullingParams
{
    uint32_t objectCount       = 0;
    uint32_t totalVisibleCount = 0;
    uint32_t padding[2]        = {0, 0};
    float4   frustumPlanes[6];
};

SEEK_NAMESPACE_END
