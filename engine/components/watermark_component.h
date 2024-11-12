#pragma once

#include "components/mesh_component.h"

SEEK_NAMESPACE_BEGIN

#include "shader/shared/common.h"
#include "shader/shared/WaterMark.h"

class WaterMarkComponent : public MeshComponent
{
public:
    WaterMarkComponent(Context* context);
    virtual ~WaterMarkComponent();

    SResult SetWaterMarkTex(RHITexturePtr watermark_tex);
    SResult SetWaterMarkDesc(WaterMarkDesc desc);
    virtual SResult       OnRenderBegin(Technique* tech, RHIMeshPtr mesh) override;
    virtual SResult       Render() override;


private:
    Technique*          m_pTechWaterMark = nullptr;
    WaterMarkDesc       m_sDesc = { 0 };
    RHIRenderBufferPtr  m_pWaterMarkDescCBuffer = nullptr;
    RHIRenderBufferPtr  m_pWaterMarkTargetSizeCBuffer = nullptr;
    RHITexturePtr       m_pWaterMarkTex = nullptr;
};

SEEK_NAMESPACE_END
