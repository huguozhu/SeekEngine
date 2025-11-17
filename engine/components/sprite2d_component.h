#pragma once

#include "kernel/kernel.h"
#include "components/component.h"

SEEK_NAMESPACE_BEGIN

class Sprite2DComponent : public Component
{
public:
    Sprite2DComponent(Context* context, uint32_t draw_index = 0);
    virtual ~Sprite2DComponent();

    virtual SResult             OnRenderBegin() { return S_Success; }
    virtual SResult             OnRenderEnd() { return S_Success; }
    virtual SResult             Render() { return S_Success; }

private:
	Context*        m_pContext = nullptr;
	uint32_t	    m_iDrawIndex = 0;
    RHITexturePtr	m_pImage = nullptr;

};

SEEK_NAMESPACE_END
