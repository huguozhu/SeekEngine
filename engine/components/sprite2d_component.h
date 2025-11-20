#pragma once

#include "kernel/kernel.h"
#include "components/scene_component.h"

SEEK_NAMESPACE_BEGIN

class Sprite2DComponent : public SceneComponent
{
public:
    Sprite2DComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index = 0);
    virtual ~Sprite2DComponent();

    void SetCenterPos(float2 pos);

    virtual SResult OnRenderBegin() { return S_Success; }
    virtual SResult OnRenderEnd() { return S_Success; }
    virtual SResult Render() = 0;

    virtual void    SetImage(RHITexturePtr image) { m_pImage = image; }

protected:
    uint32_t        m_iWidth;
    uint32_t        m_iHeight;
	uint32_t	    m_iDrawIndex = 0;
    RHITexturePtr	m_pImage = nullptr;
};

SEEK_NAMESPACE_END
