#include "components/shape_mesh_component.h"
#include "components/camera_component.h"
#include "kernel/context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_gpu_buffer.h"

//#include "effect/effect.h"
//#include "effect/technique.h"

#define SEEK_MACRO_FILE_UID 99     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * Shape Create
 ******************************************************************************/
void CreateCube(MeshData& mesh_data)
{
    // Points
    float v = 0.5f;
    
    // Positions
    {
        // +X Face
        mesh_data.positions.push_back(float3(+v, -v, -v));
        mesh_data.positions.push_back(float3(+v, +v, -v));
        mesh_data.positions.push_back(float3(+v, +v, +v));
        mesh_data.positions.push_back(float3(+v, -v, +v));
        // -X Face
        mesh_data.positions.push_back(float3(-v, -v, +v));
        mesh_data.positions.push_back(float3(-v, +v, +v));
        mesh_data.positions.push_back(float3(-v, +v, -v));
        mesh_data.positions.push_back(float3(-v, -v, -v));

        // +Y Face
        mesh_data.positions.push_back(float3(-v, +v, -v));
        mesh_data.positions.push_back(float3(-v, +v, +v));
        mesh_data.positions.push_back(float3(+v, +v, +v));
        mesh_data.positions.push_back(float3(+v, +v, -v));

        // -Y Face
        mesh_data.positions.push_back(float3(+v, -v, -v));
        mesh_data.positions.push_back(float3(+v, -v, +v));
        mesh_data.positions.push_back(float3(-v, -v, +v));
        mesh_data.positions.push_back(float3(-v, -v, -v));

        // +Z Face
        mesh_data.positions.push_back(float3(+v, -v, +v));
        mesh_data.positions.push_back(float3(+v, +v, +v));
        mesh_data.positions.push_back(float3(-v, +v, +v));
        mesh_data.positions.push_back(float3(-v, -v, +v));

        // -Z Face
        mesh_data.positions.push_back(float3(-v, -v, -v));
        mesh_data.positions.push_back(float3(-v, +v, -v));
        mesh_data.positions.push_back(float3(+v, +v, -v));
        mesh_data.positions.push_back(float3(+v, -v, -v));
    }

    // Texcoord
    {
        mesh_data.texcoords.resize(24);
        for (uint32_t i = 0; i < 6; ++i)
        {
            mesh_data.texcoords[i * 4 + 0] = float2(0, 1);
            mesh_data.texcoords[i * 4 + 1] = float2(0, 0);
            mesh_data.texcoords[i * 4 + 2] = float2(1, 0);
            mesh_data.texcoords[i * 4 + 3] = float2(1, 1);
        }
    }

    // Normal & Tangent
    {
        mesh_data.normals.resize(24);
        mesh_data.tangent.resize(24);
        for (uint32_t i = 0; i < 4; ++i)
        {
            // +X Face
            mesh_data.normals[i] = (float3(1, 0, 0));
            mesh_data.tangent[i] = (float4(0, 0, 1, 1));
            // -X Face
            mesh_data.normals[i + 4] = (float3(-1, 0, 0));
            mesh_data.tangent[i + 4] = (float4(0, 0, -1, 1));
            // +Y Face
            mesh_data.normals[i + 8] = (float3(0, 1, 0));
            mesh_data.tangent[i + 8] = (float4(1, 0, 0, 1));
            // -Y Face
            mesh_data.normals[i + 12] = (float3(0, -1, 0));
            mesh_data.tangent[i + 12] = (float4(-1, 0, 0, 1));
            // +Z Face
            mesh_data.normals[i + 16] = (float3(0, 0, 1));
            mesh_data.tangent[i + 16] = (float4(-1, 0, 0, 1));
            // -Z Face
            mesh_data.normals[i + 20] = (float3(0, 0, -1));
            mesh_data.tangent[i + 20] = (float4(1, 0, 0, 1));
        }
    }

    // indices
    mesh_data.indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    mesh_data.eTopoType = MeshTopologyType::Triangles;

    AABBox box;
    box.Min(float3(-v, -v, -v));
    box.Max(float3(+v, +v, +v));
    mesh_data.aabb = box;
}
void CreateSphere(MeshData& mesh_data, float radius = 1.0, uint32_t levels = 20, uint32_t slices = 20)
{
    float phi = 0.0f, theta = 0.0f;
    float per_phi = Math::PI / levels;
    float per_theta = Math::PI2 / slices;
    float x, y, z;

    // Top Point
    mesh_data.positions.push_back(float3(0, radius, 0));
    mesh_data.texcoords.push_back(float2(0, 0));
    mesh_data.normals.push_back(float3(0, 1, 0));
    
    // Middle Points
    for (uint32_t i = 1; i < levels; ++i)
    {
        phi = per_phi * i;
        for (uint32_t j = 0; j <= slices; ++j)
        {
            theta = per_theta * j;
            x = radius * sinf(phi) * cosf(theta);
            y = radius * cosf(phi);
            z = radius * sinf(phi) * sinf(theta);

            float3 pos = float3(x, y, z);

            mesh_data.positions.push_back(pos);
            mesh_data.texcoords.push_back(float2(theta / Math::PI2, phi / Math::PI));
            mesh_data.normals.push_back(Math::Normalize(pos));
        }
    }

    // Bottom Point
    mesh_data.positions.push_back(float3(0, -radius, 0));
    mesh_data.texcoords.push_back(float2(0, 1));
    mesh_data.normals.push_back(float3(0, -1, 0));

    // Indices
    if (levels > 1)
    {
        for (uint32_t j = 1; j <= slices; ++j)
        {
            mesh_data.indices.push_back((uint16_t)0);
            mesh_data.indices.push_back((uint16_t)(j % (slices + 1) + 1));
            mesh_data.indices.push_back((uint16_t)j);
        }
    }

    for (uint32_t i = 1; i < levels - 1; ++i)
    {
        for (uint32_t j = 1; j <= slices; ++j)
        {
            mesh_data.indices.push_back((uint16_t)((i - 1) * (slices + 1) + j));
            mesh_data.indices.push_back((uint16_t)((i - 1) * (slices + 1) + j % (slices + 1) + 1));
            mesh_data.indices.push_back((uint16_t)(i * (slices + 1) + j % (slices + 1) + 1));

            mesh_data.indices.push_back((uint16_t)(i * (slices + 1) + j % (slices + 1) + 1));
            mesh_data.indices.push_back((uint16_t)(i * (slices + 1) + j));
            mesh_data.indices.push_back((uint16_t)((i - 1) * (slices + 1) + j));
        }
    }
    mesh_data.eTopoType = MeshTopologyType::Triangles;

    AABBox box;
    box.Min(float3(-radius, -radius, -radius));
    box.Max(float3(+radius, +radius, +radius));
    mesh_data.aabb = box;
}
void CreateCone(MeshData& mesh_data, float radius = 1.0f, float height = 2.0, uint32_t slices = 20)
{
    float h2 = height * 0.5f;
    float theta = 0.0f;
    float pre_theta = Math::PI2 / slices;
    float len = sqrtf(height * height + radius * radius);
    
    // Step1: Cone's side plane
    //      top point (has same postion & texcoord, but diffent normal)
    for (uint32_t i = 0; i < slices; ++i)
    {
        theta = i * pre_theta + pre_theta * 0.5f;
        mesh_data.positions.push_back(float3(0, h2, 0));
        mesh_data.texcoords.push_back(float2(0.5, 0.0));
        mesh_data.normals.push_back(float3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len));
    }
    //      side plane's bottom points
    for (uint32_t i = 0; i < slices; ++i)
    {
        theta = i * pre_theta;

        mesh_data.positions.push_back(float3(radius * cosf(theta),  -h2, radius * sinf(theta)));
        mesh_data.texcoords.push_back(float2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f));
        mesh_data.normals.push_back(float3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len));
    }
    //      side planes's indices
    for (uint32_t i = 0; i < slices; ++i)
    {
        mesh_data.indices.push_back((uint16_t)i);
        mesh_data.indices.push_back((uint16_t)(slices + (i + 1) % slices));
        mesh_data.indices.push_back((uint16_t)(slices + i % slices));
    }

    // Step2: Cone's bottom plane
    //      bottom points
    for (uint32_t i = 0; i < slices; ++i)
    {
        theta = i * pre_theta;

        mesh_data.positions.push_back(float3(radius * cosf(theta), -h2, radius * sinf(theta)));
        mesh_data.texcoords.push_back(float2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f));
        mesh_data.normals.push_back(float3(0, -1, 0));
    }

    //      bottom center point
    mesh_data.positions.push_back(float3(0, -h2, 0));
    mesh_data.texcoords.push_back(float2(0.5, 0.5));
    mesh_data.normals.push_back(float3(0, -1, 0));

    //      indices
    uint32_t offset = 2 * slices;
    for (uint32_t i = 0; i < slices; ++i)
    {
        mesh_data.indices.push_back((uint16_t)(offset + slices));
        mesh_data.indices.push_back((uint16_t)(offset + i     % slices));
        mesh_data.indices.push_back((uint16_t)(offset + (i+1) % slices));
    }
    mesh_data.eTopoType = MeshTopologyType::Triangles;

    AABBox box;
    box.Min(float3(-radius, -radius, -h2));
    box.Max(float3(+radius, +radius, +h2));
    mesh_data.aabb = box;
}
void CreateTerrain(MeshData& mesh_data, float width, float height, uint32_t slices_x = 10, uint32_t slices_z = 10, float max_texcoord_u = 1.0f, float max_texcoord_v = 1.0f,
    const std::function<float (float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
    const std::function<float3(float, float)>& normalFunc = [](float x, float z) { return float3(0.0f, 1.0f, 0.0f); } )
{
    float slice_width = width / slices_x;
    float slice_height = height / slices_z;
    float w2 = width * 0.5f;
    float h2 = height * 0.5f;
    float pos_start_x = -w2;
    float pos_start_z = -h2;
    float texcoord_offset_x = max_texcoord_u / slices_x;
    float texcoord_offset_z = max_texcoord_v / slices_z;

    for (uint32_t z = 0; z <= slices_z; ++z)
    {
        for (uint32_t x = 0; x <= slices_x; ++x)
        {
            float pos_x = pos_start_x + x * slice_width;
            float pos_z = pos_start_z + z * slice_height;
            float3 normal = Math::Normalize(normalFunc(float(x), float(z)));
            float4 tangent = float4(normal.y(), -normal.x(), 0.0, 1.0);
            mesh_data.positions.push_back(float3(pos_x, heightFunc(pos_x, pos_z), pos_z));
            mesh_data.texcoords.push_back(float2(x * texcoord_offset_x, 1.0f - z * texcoord_offset_z));
            mesh_data.normals.push_back(normal);
            mesh_data.tangent.push_back(tangent);
        }
    }

    // indices
    for (uint32_t z = 0; z < slices_z; ++z)
    {
        for (uint32_t x = 0; x < slices_x; ++x)
        {
            mesh_data.indices.push_back((uint16_t)( z      * (slices_x + 1) + x));
            mesh_data.indices.push_back((uint16_t)((z + 1) * (slices_x + 1) + x));
            mesh_data.indices.push_back((uint16_t)((z + 1) * (slices_x + 1) + x + 1));

            mesh_data.indices.push_back((uint16_t)((z + 1) * (slices_x + 1) + x + 1));
            mesh_data.indices.push_back((uint16_t)( z      * (slices_x + 1) + x + 1));
            mesh_data.indices.push_back((uint16_t)( z      * (slices_x + 1) + x));
        }
    }
    mesh_data.eTopoType = MeshTopologyType::Triangles;

    AABBox box;
    box.Min(float3(-w2, 0, -h2));
    box.Max(float3(+w2, 0, +h2));
    mesh_data.aabb = box;
}

RHIMeshPtr CreateMeshFromMeshData(Context* context, MeshData& mesh_data)
{
    RHIMeshPtr pMesh = context->RHIContextInstance().CreateMesh();
    RHIContext& rc = context->RHIContextInstance();

    RHIGpuBufferPtr buf_pos = rc.CreateVertexBuffer(sizeof(float3) * mesh_data.positions.size(), new RHIGpuBufferData(sizeof(float3) * mesh_data.positions.size(), (const void*)&mesh_data.positions[0]) );
    VertexStream vs_pos = { buf_pos, 0, sizeof(float3), {{0, 0, 1, VertexFormat::Float3, VertexElementUsage::Position, false, 0}}, false};
    pMesh->AddVertexStream(vs_pos);

    if (mesh_data.texcoords.size() > 0)
    {
        RHIGpuBufferPtr buf_tc = rc.CreateVertexBuffer(sizeof(float2) * mesh_data.texcoords.size(), new RHIGpuBufferData(sizeof(float2) * mesh_data.texcoords.size(), (const void*)&mesh_data.texcoords[0]));
        VertexStream vs_tc = { buf_tc, 0, sizeof(float2), {{0, 0, 1, VertexFormat::Float2, VertexElementUsage::TexCoord, false, 0}}, false };
        pMesh->AddVertexStream(vs_tc);
    }

    if (mesh_data.normals.size() > 0)
    {
        RHIGpuBufferPtr buf_normal = rc.CreateVertexBuffer(sizeof(float3) * mesh_data.normals.size(), new RHIGpuBufferData(sizeof(float3) * mesh_data.normals.size(), (const void*)&mesh_data.normals[0]));
        VertexStream vs_normal = { buf_normal, 0, sizeof(float3), {{0, 0, 1, VertexFormat::Float3, VertexElementUsage::Normal, false, 0}}, false };
        pMesh->AddVertexStream(vs_normal);
    }

    if (mesh_data.tangent.size() > 0)
    {
        RHIGpuBufferPtr buf_tangent = rc.CreateVertexBuffer(sizeof(float4) * mesh_data.tangent.size(), new RHIGpuBufferData(sizeof(float4) * mesh_data.tangent.size(), (const void*)&mesh_data.tangent[0]) );
        VertexStream vs_tangent = { buf_tangent, 0, sizeof(float4), {{0, 0, 1, VertexFormat::Float4, VertexElementUsage::Tangent, false, 0}}, false };
        pMesh->AddVertexStream(vs_tangent);
    }

    if (mesh_data.indices.size() > 0)
    {
        RHIGpuBufferPtr buf_index = rc.CreateIndexBuffer(sizeof(uint16_t) * mesh_data.indices.size(), new RHIGpuBufferData(sizeof(uint16_t) * mesh_data.indices.size(), (const void*)&mesh_data.indices[0]) );
        pMesh->SetIndexBuffer(buf_index, IndexBufferType::UInt16);
    }

    pMesh->SetTopologyType(mesh_data.eTopoType);
    pMesh->SetAABBox(mesh_data.aabb);

    MaterialPtr material = MakeSharedPtr<Material>();
    pMesh->SetMaterial(material);
    return pMesh;
}


/******************************************************************************
 * CubeMeshComponent
 ******************************************************************************/
CubeMeshComponent::CubeMeshComponent(Context* context)
    :ShapeMeshComponent(context)
{
    this->SetName("Cube Mesh Component");

    // Create Mesh
    CreateCube(m_sMeshData);
    RHIMeshPtr pMesh = CreateMeshFromMeshData(m_pContext, m_sMeshData);

    this->AddMesh(pMesh);
    this->SetAABBox(pMesh->GetAABBox());
}
CubeMeshComponent::~CubeMeshComponent()
{
}


/******************************************************************************
 * SphereMeshComponent
 ******************************************************************************/
SphereMeshComponent::SphereMeshComponent(Context* context, uint32_t x_segment_num, uint32_t y_segment_num)
    :ShapeMeshComponent(context)
{
    this->SetName("Sphere Mesh Component");

    // Create Mesh
    CreateSphere(m_sMeshData);
    RHIMeshPtr pMesh = CreateMeshFromMeshData(m_pContext, m_sMeshData);

    this->AddMesh(pMesh);
    this->SetAABBox(pMesh->GetAABBox());
}
SphereMeshComponent::~SphereMeshComponent()
{
}

/******************************************************************************
 * ConeMeshComponent
 ******************************************************************************/
ConeMeshComponent::ConeMeshComponent(Context* context)
    :ShapeMeshComponent(context)
{
    this->SetName("Cone Mesh Component");

    // Create Mesh
    CreateCone(m_sMeshData);
    RHIMeshPtr pMesh = CreateMeshFromMeshData(m_pContext, m_sMeshData);

    this->AddMesh(pMesh);
    this->SetAABBox(pMesh->GetAABBox());
}
ConeMeshComponent::~ConeMeshComponent()
{
}


/******************************************************************************
 * TerrainMeshComponent
 ******************************************************************************/
TerrainMeshComponent::TerrainMeshComponent(Context* context, float width, float height, uint32_t slices_x, uint32_t slices_z, float max_texcoord_u, float max_texcoord_v, 
    const std::function<float(float, float)>& heightFunc, const std::function<float3(float, float)>& normalFunc)
    :ShapeMeshComponent(context)
{
    this->SetName("Terrain Mesh Component");

    // Create Mesh
    CreateTerrain(m_sMeshData, width, height, slices_x, slices_z, max_texcoord_u, max_texcoord_v, heightFunc, normalFunc);
    RHIMeshPtr pMesh = CreateMeshFromMeshData(m_pContext, m_sMeshData);

    this->AddMesh(pMesh);
    this->SetAABBox(pMesh->GetAABBox());
}
TerrainMeshComponent::~TerrainMeshComponent()
{
}

/******************************************************************************
 * PlaneMeshComponent
 ******************************************************************************/
PlaneMeshComponent::PlaneMeshComponent(Context* context, float width, float height)
    :TerrainMeshComponent(context, width, height, 1, 1)
{
    this->SetName("Plane Mesh Component");
}
PlaneMeshComponent::~PlaneMeshComponent()
{
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
