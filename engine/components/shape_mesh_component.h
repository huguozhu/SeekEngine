#pragma once

#include "components/mesh_component.h"
#include "rhi/base/rhi_definition.h"
#include "math/matrix.h"
#include "utils/shape_mesh.h"

SEEK_NAMESPACE_BEGIN

class ShapeMeshComponent : public MeshComponent
{
public:
    ShapeMeshComponent(Context* context)
        :MeshComponent(context) {}
    ~ShapeMeshComponent() {}

protected:
    MeshData m_sMeshData;
};

/******************************************************************************
 * CubeMeshComponent
 ******************************************************************************/
class CubeMeshComponent : public ShapeMeshComponent
{
public:
    CubeMeshComponent(Context* context);
    virtual ~CubeMeshComponent();
};

/******************************************************************************
 * SphereMeshComponent
 ******************************************************************************/
class SphereMeshComponent : public ShapeMeshComponent
{
public:
    SphereMeshComponent(Context* context, uint32_t x_segment_num = 64, uint32_t y_segment_num = 64);
    virtual ~SphereMeshComponent();
};


/******************************************************************************
 * ConeMeshComponent
 ******************************************************************************/
class ConeMeshComponent : public ShapeMeshComponent
{
public:
    ConeMeshComponent(Context* context);
    virtual ~ConeMeshComponent();
};

/******************************************************************************
 * TerrainMeshComponent
 ******************************************************************************/
class TerrainMeshComponent : public ShapeMeshComponent
{
public:
    TerrainMeshComponent(Context* context, float width, float height, uint32_t slices_x = 10, uint32_t slices_z = 10, float max_texcoord_u = 1.0f, float max_texcoord_v = 1.0f,
        const std::function<float(float, float)>& heightFunc = [](float x, float z) { return 0.0f; },
        const std::function<float3(float, float)>& normalFunc = [](float x, float z) { return float3(0.0f, 1.0f, 0.0f); });
    virtual ~TerrainMeshComponent();
};

/******************************************************************************
 * PlaneMeshComponent
 ******************************************************************************/
class PlaneMeshComponent : public TerrainMeshComponent
{
public:
    PlaneMeshComponent(Context* context, float width, float height);
    virtual ~PlaneMeshComponent();
};

SEEK_NAMESPACE_END
