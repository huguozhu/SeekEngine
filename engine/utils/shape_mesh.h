#pragma once

#include "kernel/kernel.h"
#include "math/aabbox.h"
#include "rhi/base/rhi_definition.h"


SEEK_NAMESPACE_BEGIN

struct MeshData
{
    std::vector<float3> positions;
    std::vector<float2> texcoords;
    std::vector<float3> normals;
    std::vector<float4> tangent;        // optional
    std::vector<uint16_t> indices;      // optional

    MeshTopologyType eTopoType;
    AABBox aabb;
};

void CreateCube(MeshData& mesh_data);
void CreateSphere(MeshData& mesh_data, float radius = 1.0, uint32_t levels = 20, uint32_t slices = 20);
void CreateCone(MeshData& mesh_data, float radius = 1.0f, float height = 2.0, uint32_t slices = 20);
void CreateTerrain(MeshData& mesh_data, float width, float height, uint32_t slices_x = 10, uint32_t slices_z = 10, float max_texcoord_u = 1.0f, float max_texcoord_v = 1.0f,
    const std::function<float(float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
	const std::function<float3(float, float)>& normalFunc = [](float x, float z) { return float3(0.0f, 1.0f, 0.0f); });

RHIMeshPtr CreateMeshFromMeshData(Context* context, MeshData& mesh_data);

SEEK_NAMESPACE_END
