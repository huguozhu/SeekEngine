#include "effect/taa.h"
#include "effect/effect.h"
#include "effect/scene_renderer.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_render_view.h"

SEEK_NAMESPACE_BEGIN

#define DVF_MACRO_FILE_UID 83     // this code is auto generated, don't touch it!!!

static const std::vector<std::string> _taa_param_names = {
    "currentTex",
    "historyTex",
    "velocityTex",
    "GlobalParams",
};

SResult TaaPostProcess::Init()
{
	static const std::string tech_name = "TAA";
    m_pContext->EffectInstance().LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "PostProcessVS", "TaaPS", nullptr);
    SEEK_RETIF_FAIL(PostProcess::Init(tech_name, NULL_PREDEFINES));

    m_globalParamCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(TAAGlobalParams), RESOURCE_FLAG_CPU_WRITE);
    return S_Success;
}

SResult TaaPostProcess::Run()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();

    //TIMER_BEG(t1);
    auto texDesc = rc.GetCurRHIFrameBuffer()->GetRenderTargetDesc(RHIFrameBuffer::Attachment::Color0);
    if (!m_pHistoryTex)
    {
        texDesc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_GPU_WRITE;
        m_pHistoryTex = rc.CreateTexture2D(texDesc);
    }
    
    SetParam(_taa_param_names[1], m_pHistoryTex);
    RHITexture::Desc& desc = rc.GetScreenRHIFrameBuffer()->GetRenderTargetDesc(RHIFrameBuffer::Attachment::Color0);
    TAAGlobalParams global_params;
    global_params.statuses.x() = m_isFirstFrame ? 1 : 0;
    global_params.jitter = m_pContext->GetJitter();
    global_params.invScreenSize = float2{ 1.0f / desc.width, 1.0f / desc.height };
    m_globalParamCBuffer->Update(&global_params, sizeof(global_params));

    SResult ret = PostProcess::Run();
    // copy from m_pCurrentTex to m_pHistoryTex
    if (m_pHistoryTex->Width() != texDesc.width || m_pHistoryTex->Height() != texDesc.height)
    {
        m_pHistoryTex.reset();
        texDesc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_GPU_WRITE;
        m_pHistoryTex = rc.CreateTexture2D(texDesc);
    }
    ret = rc.GetCurRHIFrameBuffer()->CopyRenderTarget(RHIFrameBuffer::Attachment::Color0, m_pHistoryTex);
    //TIMER_END(t1, "TAA");
    m_isFirstFrame = false;
    return ret;
}

SEEK_NAMESPACE_END


#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
