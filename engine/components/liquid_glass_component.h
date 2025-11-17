#pragma once

#include "components/sprite2d_component.h"
#include "math/vector.h"

SEEK_NAMESPACE_BEGIN
#include "shader/shared/LiquidGlass.h"

class LiquidGlassComponent : public Sprite2DComponent
{
public:
    LiquidGlassComponent(Context* context);
    virtual ~LiquidGlassComponent();

    virtual SResult     OnRenderBegin() override;
    virtual SResult     Render() override;
    virtual SResult     Tick(float delta_time) override;

    SResult InitShaders();

    void SetBgTex(RHITexturePtr bg) { m_pBgTex = bg; }

private:
    Technique*      m_pDrawBgTech = nullptr;
    Technique*      m_pLiquildTech = nullptr;
    RHITexturePtr   m_pBgTex = nullptr;

	RHIMeshPtr	    m_pBgMesh = nullptr;
	RHIMeshPtr	    m_pShpereMesh = nullptr;

    LiquidGlassParam m_Param;
    RHIGpuBufferPtr  m_pParamCbBuffer = nullptr;
};

SEEK_NAMESPACE_END
