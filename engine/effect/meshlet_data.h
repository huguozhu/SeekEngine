#pragma once

#include "kernel/kernel.h"
#include <vector>
#include <cstring>

SEEK_NAMESPACE_BEGIN

// CPU-side meshlet metadata for a single meshlet (max 64 vertices, 84 triangles)
// Mirrored by GPUMeshletData in gpu_culling_types.h for GPU-side access
struct Meshlet
{
    uint32_t vertexOffset   = 0;
    uint32_t triangleOffset = 0;
    uint32_t vertexCount    = 0;
    uint32_t triangleCount  = 0;

    // Bounding sphere (frustum/occlusion culling)
    float    boundingCenter[3];
    float    boundingRadius;

    // Normal cone (backface culling)
    float    coneApex[3];
    float    coneAxis[3];
    float    coneCutoff;
};

// All meshlet data for a single mesh primitive
struct MeshletGroup
{
    std::vector<Meshlet>    meshlets;
    std::vector<uint32_t>   meshletVertices;
    std::vector<uint8_t>    meshletTriangles;

    bool IsEmpty() const { return meshlets.empty(); }
};

SEEK_NAMESPACE_END
