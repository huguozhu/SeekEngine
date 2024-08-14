#pragma once

#include "components/scene_component.h"
//#include "effect/scene_renderer.h"
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

struct BSConfig
{
    enum class Method
    {
        T0,
        T1,
        NUM,
    };

    std::string                 name;
    Method                      method = Method::T0;
    std::vector<std::string>    relatedBSNames;
    std::vector<float>          coef;

    int32_t                     nameIdx = -1;
    std::vector<int32_t>        relatedBSNamesIdx;
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

    void                    AddMesh(MeshPtr mesh) { m_vMeshes.push_back(mesh); }
    void                    InsertMesh(size_t idx, MeshPtr mesh) { m_vMeshes.insert(m_vMeshes.begin() + idx, mesh); }
    void                    DelMesh(MeshPtr mesh);
    size_t                  NumMeshes() const { return m_vMeshes.size(); }
    MeshPtr                 GetMeshByIndex(size_t index);
    std::vector<MeshPtr>&   GetMeshes() { return m_vMeshes; }

    void                    SetVisible(bool b);

    VisibleMark             GetVisibleMark() const { return m_eVisibleMark; }
    void                    SetVisibleMark(VisibleMark mark) { m_eVisibleMark = mark; }

    void                    SetAABBox(AABBox const& box) { m_cAABBox = box; }
    AABBox const&           GetAABBox() const { return m_cAABBox; }
    void                    SetAABBoxWorld(AABBox const& box) { m_cAABBoxWorld = box; }
    AABBox const&           GetAABBoxWorld() const { return m_cAABBoxWorld; }

    void                    SetBSConfig(std::vector<BSConfig>& bsConfig);
    virtual SResult       Tick(float delta_time);

    bool                    IsInstanceMesh() { return m_bIsInstance; }

protected:
    std::vector<MeshPtr>    m_vMeshes;

    AABBox                  m_cAABBox;
    AABBox                  m_cAABBoxWorld;
    VisibleMark             m_eVisibleMark = VisibleMark::Yes;
    std::vector<BSConfig>   m_vBSConfig;
    bool                    m_bIsInstance = false;

    RenderBufferPtr         m_ModelInfoCBuffer;
};

SEEK_NAMESPACE_END
