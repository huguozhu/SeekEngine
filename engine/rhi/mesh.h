#pragma once

#include "kernel/kernel.h"
#include "rhi/render_definition.h"
#include "rhi/format.h"
#include "rhi/material.h"
#include "rhi/render_state.h"
#include "math/vector.h"
#include "math/aabbox.h"
#include "resource/resource_mgr.h"

SEEK_NAMESPACE_BEGIN
//undef to fix the conflit with X11
#undef None

enum class SkinningJointBindSize
{
    None = 0x00,
    Four = 0x04,
    Eight = 0x08,
    Twelve = 0x0C,
};


class Mesh
{
public:
    MeshTopologyType                    GetTopologyType() const { return m_eTopoType; }
    void                                SetTopologyType(MeshTopologyType t) { m_eTopoType = t; }

    // Index Streams
    bool                                IsUseIndices() const;
    uint32_t                            GetNumIndices() const;
    RenderBufferPtr const&              GetIndexBuffer();
    IndexBufferType                     GetIndexBufferType() const { return m_eIndexBufferType; }
    void                                SetIndexBuffer(RenderBufferPtr buffer, IndexBufferType type);
    void                                SetIndexBufferResource(std::shared_ptr<VertexIndicesResource>& indicesRes);

    // Vertex Streams
    uint32_t                            GetNumVertex() const;
    uint32_t                            NumVertexStream();
    VertexStream&                       GetVertexStreamByIndex(uint32_t i);
    std::vector<VertexStream>&          GetVertexStreams();
    void                                AddVertexStream(RenderBufferPtr render_buffer, uint32_t buffer_offset, uint32_t stride, VertexFormat format, VertexElementUsage usage, uint32_t usage_index);
    void                                AddInstanceVertexStream(RenderBufferPtr render_buffer, uint32_t buffer_offset, uint32_t stride, VertexFormat format, VertexElementUsage usage, uint32_t usage_index, uint32_t instance_count, uint32_t divisor);
    VertexStream*                       GetInstanceVertexStream(VertexElementUsage instance_type, uint32_t usage_index);
    void                                SetVertexAttributeResource(VertexAttributeResource& res);
    VertexAttributeResource&            GetVertexAttributeResource();
    VertexIndicesResource&              GetVertexIndicesResource();
    
    // Material
    void                                SetMaterial(MaterialPtr material) { m_pMaterial = material; }
    MaterialPtr&                        GetMaterial();
    void                                SetMaterialResource(std::shared_ptr<MaterialResource>& res) { m_materialRes = res; }
    std::shared_ptr<MaterialResource>   GetMaterialResource() { return m_materialRes; }

    // Render State
    void                                SetRenderState(RenderStatePtr rs) { m_pRenderState = rs; }
    RenderStatePtr&                     GetRenderState() { return m_pRenderState; }

    // Morph targets
    MorphInfo&                          GetMorphInfo();
    bool                                HasMorphTarget();
    const RenderBufferPtr&              GetMorphWeightsCBuffer();
    const RenderBufferPtr&              GetMorphSizeCBuffer();
    RenderBufferPtr&                    GetMaterialInfoCBuffer();

    // Skin
    SkinningJointBindSize               GetSkinningJointBindSize() { return m_eJointBindSize; }
    void                                SetSkinningJointBindSize(SkinningJointBindSize s) { m_eJointBindSize = s; }

    // AABBox
    void                                SetAABBox(AABBox const& box) { m_cAABBox = box; }
    AABBox const&                       GetAABBox() const { return m_cAABBox; }
    void                                SetAABBoxWorld(AABBox const& box) { m_cAABBoxWorld = box; }
    AABBox const&                       GetAABBoxWorld() const { return m_cAABBoxWorld; }

    // Is Draw
    void                                SetVisible(bool isVisible) { m_bIsVisible = isVisible; }
    bool                                IsVisible() const { return m_bIsVisible; }

    void                                SetMorphTargetResource(MorphTargetResource& res) { m_morphTargetRes = res; }
    MorphTargetResource&                GetMorphTargetResource();

    bool                                IsInstanceRendering() { return m_bIsInstance; }
    uint32_t                            GetInstanceCount() { return m_uInstancCount; }
    void                                SetInstanceCount(uint32_t count) { m_uInstancCount = count; }
    
    bool                                m_bFrontFaceCCW = false;
    bool                                m_bUpdateFrontFaceCCW = true;

protected:
    Mesh(Context* context) :m_pContext(context) {}
    virtual ~Mesh() {}

    Context*                    m_pContext = nullptr;
    mutable bool                m_bDataDirty = true;

    // index buffers
    RenderBufferPtr             m_pIndexBuffer = nullptr;
    IndexBufferType             m_eIndexBufferType = IndexBufferType::UInt16;

    // vertex buffers
    MeshTopologyType            m_eTopoType = MeshTopologyType::Triangles;
    std::vector<VertexStream>   m_vVertexStreams;

    // Material
    MaterialPtr                 m_pMaterial = nullptr;

    // RenderState
    RenderStatePtr              m_pRenderState = nullptr;

    // Morph Targets
    MorphInfo                   m_stMorphInfo;

    RenderBufferPtr             m_morphWeightsCBuffer;
    RenderBufferPtr             m_morphSizeCBuffer;
    RenderBufferPtr             m_MaterialInfoCBuffer;

    // Skin
    SkinningJointBindSize       m_eJointBindSize = SkinningJointBindSize::None;

    // AABBox
    AABBox                      m_cAABBox;
    AABBox                      m_cAABBoxWorld;

    // Whether rendering
    bool                        m_bIsVisible = true;

    std::shared_ptr<VertexIndicesResource> m_indicesRes;
    MorphTargetResource m_morphTargetRes;
    VertexAttributeResource m_vertexAttributeRes;
    std::shared_ptr<MaterialResource> m_materialRes;

    // Instance rendering
    bool                        m_bIsInstance = false;
    uint32_t                    m_uInstancCount = 1;
};

SEEK_NAMESPACE_END
