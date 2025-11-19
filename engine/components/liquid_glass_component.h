#pragma once

#include "components/sprite2d_component.h"
#include "math/vector.h"
#include "physics/spring_mass_damper.h"

SEEK_NAMESPACE_BEGIN
#include "shader/shared/LiquidGlass.h"

class LiquidGlassComponent : public Sprite2DComponent
{
public:
    LiquidGlassComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index = 0);
    virtual ~LiquidGlassComponent();

    virtual SResult     OnRenderBegin() override;
    virtual SResult     Render() override;
    virtual SResult     Tick(float delta_time) override;

    void SetBgTex(RHITexturePtr bg) { m_pBgTex = bg; }

private:
    void Reset();
    SResult InitShaders();

private:
    Technique*          m_pLiquildTech = nullptr;
    RHITexturePtr       m_pBgTex = nullptr;
	RHIMeshPtr	        m_pLiquidMesh = nullptr;

    LiquidGlassParam    m_Param;
    RHIGpuBufferPtr     m_pMvpCbBuffer = nullptr;
    RHIGpuBufferPtr     m_pParamCbBuffer = nullptr;

    SpringMassDamperPtr m_pSpringMassDamper_1 = nullptr;
    SpringMassDamperPtr m_pSpringMassDamper_2 = nullptr;

	float               m_fDuration = 0.0f;
};

SEEK_NAMESPACE_END
