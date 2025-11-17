#pragma once

#include "components/sprite2d_component.h"
#include "math/vector.h"
#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

#include "shader/shared/common.h"
#include "shader/shared/WaterMark.h"

class WaterMarkComponent : public Sprite2DComponent
{
public:
    WaterMarkComponent(Context* context);
    virtual ~WaterMarkComponent();

    SResult SetWaterMarkTex(RHITexturePtr watermark_tex);
    SResult SetWaterMarkDesc(WaterMarkDesc desc);
    virtual SResult       OnRenderBegin() override;
    virtual SResult       Render() override;

    SResult WaterMarkGenerate();


private:
    Technique*          m_pTechWaterMarkRender = nullptr;
    Technique*          m_pTechWaterMarkGenerate = nullptr;

    RHIMeshPtr	        m_pDrawMesh = nullptr;

    WaterMarkDesc       m_sDesc = { 0 };
    RHIGpuBufferPtr     m_pWaterMarkDescCBuffer = nullptr;
    RHIGpuBufferPtr     m_pWaterMarkTargetSizeCBuffer = nullptr;
    RHITexturePtr       m_pWaterMarkTex = nullptr;
    bool                m_bDirty = true;
    RHITexturePtr       m_pRepeatWaterMark = nullptr;
};

SEEK_NAMESPACE_END
