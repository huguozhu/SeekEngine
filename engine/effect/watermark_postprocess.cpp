#include "effect/watermark_postprocess.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "rhi/base/rhi_program.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_shader.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_context.h"
#include "math/math_utility.h"
#include "math/quad_mesh_process.h"
#include "math/matrix.h"
#include "utils/log.h"


#define SEEK_MACRO_FILE_UID 53     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * WaterMarkPostProcess
 ******************************************************************************/
WaterMarkPostProcess::WaterMarkPostProcess(Context* context)
    : PostProcess(context, "WaterMark", PostProcessRenderType::Render_2D)
{
}

SResult WaterMarkPostProcess::Init()
{
    SResult res = PostProcess::Init("WaterMark");
    m_pWatermarkCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(WaterMarkDesc), RESOURCE_FLAG_CPU_WRITE);
    this->SetParam("waterMarkDesc", m_pWatermarkCBuffer);
    return res;
}

SResult WaterMarkPostProcess::SetSrcTex(RHITexturePtr src)
{
    m_pSrcTex = src;
    this->SetParam("src_rgba", m_pSrcTex);
    return S_Success;
}
SResult WaterMarkPostProcess::SetWaterMarkDesc(WaterMarkDesc desc)
{
    m_sDesc = desc;
    return m_pWatermarkCBuffer->Update(&m_sDesc, sizeof(WaterMarkDesc));
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
