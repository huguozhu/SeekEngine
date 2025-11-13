#include "components/shape_mesh_component.h"
#include "components/camera_component.h"
#include "kernel/context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "utils/shape_mesh.h"

#define SEEK_MACRO_FILE_UID 99     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

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
