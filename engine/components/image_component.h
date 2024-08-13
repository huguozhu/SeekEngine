#pragma once

#include "components/sprite2d_component.h"
#include "rhi/mesh.h"

SEEK_NAMESPACE_BEGIN

class ImageComponent : public Sprite2DComponent
{
public:
    ImageComponent(Context* context);
    virtual ~ImageComponent() {}

    virtual SResult   Render() override;

    void                SetVisible(bool b) { m_bVisible = b; }
    void                SetAlpha(float alpha);
    void                SetPos(float x1, float x2, float y1, float y2);
    void                SetAngle(int angle);

    void                SetMaterial2D(Material2DPtr material) { m_pMesh->SetMaterial2D(material); }
    Material2DPtr       GetMaterial2D() const { return m_pMesh->GetMaterial2D(); }

protected:
    virtual SResult   OnRenderBegin(Technique* tech, MeshPtr mesh) override;
    virtual SResult   OnRenderEnd() override;

    std::vector<float>  m_VerticesInit;
    MeshPtr             m_pMesh = nullptr;

    bool                m_bDirty = false;
    bool                m_bVisible = true;
    int                 m_iAngle = 0;

    RenderBufferPtr     m_GlobalParamsCBuffer;
};

SEEK_NAMESPACE_END
