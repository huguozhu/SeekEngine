#pragma once

#include "components/scene_component.h"
#include "math/aabbox.h"

SEEK_NAMESPACE_BEGIN

#include "shader/shared/common.h"

#define MAX_LIGHT           6

enum class VisibleMark : uint8_t
{
    Yes,
    No,
    Partial,
};

/*
 * MeshComponent is an instance of a renderable collection of triangles.
 *
 * @see SkeletalMeshComponent
 */
class MeshComponent : public SceneComponent
{
public:
    MeshComponent(Context* context);
    virtual ~MeshComponent();

    void                        AddMesh(RHIMeshPtr mesh) { m_vMeshes.push_back(mesh); }
    void                        InsertMesh(size_t idx, RHIMeshPtr mesh) { m_vMeshes.insert(m_vMeshes.begin() + idx, mesh); }
    void                        DelMesh(RHIMeshPtr mesh);
    size_t                      NumMeshes() const { return m_vMeshes.size(); }
    RHIMeshPtr                  GetMeshByIndex(size_t index);
    std::vector<RHIMeshPtr>&    GetMeshes() { return m_vMeshes; }

    void                        SetVisible(bool b);

    VisibleMark                 GetVisibleMark() const { return m_eVisibleMark; }
    void                        SetVisibleMark(VisibleMark mark) { m_eVisibleMark = mark; }

    void                        SetAABBox(AABBox const& box) { m_cAABBox = box; }
    AABBox const&               GetAABBox() const { return m_cAABBox; }
    void                        SetAABBoxWorld(AABBox const& box) { m_cAABBoxWorld = box; }
    AABBox const&               GetAABBoxWorld() const { return m_cAABBoxWorld; }

    virtual SResult             OnRenderBegin(Technique* tech, RHIMeshPtr mesh);
    virtual SResult             OnRenderEnd();
    virtual SResult             Render();
    virtual SResult             RenderMesh(uint32_t i);

    bool                        IsInstanceMesh() { return m_bIsInstance; }

private:
    void                        FillMaterialParam(Technique* tech, RHIMeshPtr& pMesh);

protected:
    std::vector<RHIMeshPtr>     m_vMeshes;

    AABBox                      m_cAABBox;
    AABBox                      m_cAABBoxWorld;
    VisibleMark                 m_eVisibleMark = VisibleMark::Yes;
    bool                        m_bIsInstance = false;

    RHIRenderBufferPtr          m_ModelInfoCBuffer;
    RHIRenderBufferPtr          m_GenCubeShaodowCBuffer;
};

SEEK_NAMESPACE_END
