#include "effect/sprite2d_renderer.h"
#include "effect/technique.h"
#include "effect/effect.h"
#include "kernel/context.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/format.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/base/rhi_definition.h"    
#include "utils/error.h"
//#include "components/image_component.h"

#define DVF_MACRO_FILE_UID 30     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

Sprite2DRenderer::Sprite2DRenderer(Context* context)
    :SceneRenderer(context)
{
}

Sprite2DRenderer::~Sprite2DRenderer()
{
}

SResult Sprite2DRenderer::Init()
{
    Effect& effect = m_pContext->EffectInstance();
    return S_Success;
}

SResult Sprite2DRenderer::BuildRenderJobList()
{
    m_vRenderingJobs.clear();
    // the last job should be FinishJob()
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&Sprite2DRenderer::RenderSprite2DJob, this)));
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&Sprite2DRenderer::FinishJob, this)));
    return S_Success;
}

RendererReturnValue Sprite2DRenderer::RenderSprite2DJob()
{
    m_eCurRenderStage = RenderStage::Sprite2D;
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHIFrameBuffer* finalFB = rc.GetFinalRHIFrameBuffer().get();
    rc.BeginRenderPass({ "RenderSprite", finalFB });

    SResult res = m_pContext->SceneManagerInstance().RenderSprite2DScene();
    if (res != S_Success)
    {
        LOG_ERROR_PRIERR(res, "Sprite2DRenderer::RenderSprite2DJob() failed.");
    }
    rc.EndRenderPass();

    return RRV_NextJob;
}

RendererReturnValue Sprite2DRenderer::FinishJob()
{
    return RRV_Finish;
}

SEEK_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
