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

// ============================================================================
// GPUMeshletData — GPU 端单个 meshlet 的元数据（与 Meshlet 结构体布局一致）
// StructuredBuffer<GPUMeshletData> 供 Compute Shader / Mesh Shader 访问
// ============================================================================
struct GPUMeshletData
{
    uint32_t vertexOffset   = 0;  // meshlet 顶点索引缓冲的偏移（uint32_t 索引单位）
    uint32_t triangleOffset = 0;  // meshlet 图元索引缓冲的偏移（字节单位）
    uint32_t vertexCount    = 0;  // 顶点数（≤ 64）
    uint32_t triangleCount  = 0;  // 图元数（≤ 84）
    float    boundingCenterX = 0.0f;
    float    boundingCenterY = 0.0f;
    float    boundingCenterZ = 0.0f;
    float    boundingRadius   = 0.0f;
    float    coneApexX = 0.0f;
    float    coneApexY = 0.0f;
    float    coneApexZ = 0.0f;
    float    coneAxisX = 0.0f;
    float    coneAxisY = 0.0f;
    float    coneAxisZ = 0.0f;
    float    coneCutoff = 0.0f;
    // 填充到 64 字节对齐（实际大小 = 64 bytes）
    uint32_t padding[2] = {0, 0};
};

SEEK_NAMESPACE_END
