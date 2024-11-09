#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/postprocess.h"
#include "effect/watermark_postprocess.h"
#include "components/camera_component.h"
#include "components/light_component.h"
#include "math/matrix.h"
#include "kernel/context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_query.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 28     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SceneRenderer::SceneRenderer(Context* context)
    :m_pContext(context)
{
}

SceneRenderer::~SceneRenderer()
{
}

SResult SceneRenderer::Init()
{
    SResult ret = S_Success;

    return ret;
}

bool SceneRenderer::HasRenderJob()
{
    return !m_vRenderingJobs.empty();
}

RendererReturnValue SceneRenderer::DoRenderJob()
{
    RendererReturnValue rrv = RRV_NextJob;
    for (auto& job : m_vRenderingJobs)
    {
        rrv = job->Run();
        if (rrv != RRV_NextJob)
            break;
    }
    return rrv;
}

RendererReturnValue SceneRenderer::ToneMappingJob()
{
    m_eCurRenderStage = RenderStage::None;
    RHIContext& rc = m_pContext->RHIContextInstance();
    
    SResult ret = m_pToneMappingPostProcess->Run();
    if (ret != S_Success)
        LOG_ERROR_PRIERR(ret, "SceneRenderer::ToneMappingJob() m_pToneMappingPostProcess->Run() failed.");

    return RRV_NextJob;
}
RendererReturnValue SceneRenderer::WatermarkJob()
{
    m_eCurRenderStage = RenderStage::None;
    RHIContext& rc = m_pContext->RHIContextInstance();

    SResult ret = m_pWatermarkPostProcess->Run();
    if (ret != S_Success)
        LOG_ERROR_PRIERR(ret, "SceneRenderer::WatermarkJob() m_pWatermarkPostProcess->Run() failed.");
    return RRV_NextJob;
}

RendererReturnValue SceneRenderer::FinishJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_bRenderSizeChanged = false;
    return RRV_Finish;
}

SResult SceneRenderer::FillLightInfoByLightIndex(LightInfo& info, CameraComponent* pCamera, size_t light_index)
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    LightComponent* light = sm.GetLightComponentByIndex(light_index);
    if (!light || !pCamera)
        return ERR_INVALID_ARG;

    float exposure = 1.0;
    if (m_pContext->GetLightingMode() == LightingMode::PBR)
        exposure = pCamera->GetExposure() * PBR_INTENSITY_COEFF;

    LightType type = light->GetLightType();
    info.color = light->GetColor().ToFloat3();
    info.type = (int)type;
    info.direction = light->GetDirection();
    info.falloffRadius = light->GetFalloffRadius();
    info.posWorld = light->GetLightPos();
    info.intensity = light->GetIntensity() * exposure;

    if (LightType::Spot == type)
    {
        float2 inOutCutoff = ((SpotLightComponent*)light)->GetInOutCutoff();
        info.inOutCutoff = float2(cos(inOutCutoff.x()), cos(inOutCutoff.y()));
    }
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
