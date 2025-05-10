#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/postprocess.h"
#include "components/camera_component.h"
#include "components/light_component.h"
#include "math/matrix.h"
#include "kernel/context.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_query.h"
#include "utils/log.h"

#include <algorithm>

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
SResult SceneRenderer::RenderScene(uint32_t scope)
{
    if (m_renderableMeshes.empty())
        return S_Success;

    // to delete these codes, repeat at MeshComponent::RenderMesh()
    {
        for (auto& mesh : m_renderableMeshes)
        {
            Technique* tech = nullptr;
            RHIMeshPtr pMesh = mesh.first->GetMeshByIndex(mesh.second);
            if (!pMesh->GetTechnique())
            {
                // auto choose a technique
                GetEffectTechniqueToRender(pMesh, &tech);
                pMesh->SetTechnique(tech);
            }
        }
    }

    // the SceneManager can supply the sort method
    float3 base = m_pContext->SceneManagerInstance().GetActiveCamera()->GetWorldTransform().GetTranslation();
    std::sort(m_renderableMeshes.begin(), m_renderableMeshes.end(), [base](const MeshPair& m1, const MeshPair& m2)->bool {
        float dis1 = Math::Distance(base, m1.first->GetMeshByIndex(m1.second)->GetAABBoxWorld().Center());
        float dis2 = Math::Distance(base, m2.first->GetMeshByIndex(m2.second)->GetAABBoxWorld().Center());

        if (m1.first->GetMeshByIndex(m1.second)->GetMaterial()->alpha_mode == AlphaMode::Blend &&
            m2.first->GetMeshByIndex(m2.second)->GetMaterial()->alpha_mode == AlphaMode::Blend)
        {
            return dis1 > dis2;
        }
        else if (m1.first->GetMeshByIndex(m1.second)->GetMaterial()->alpha_mode == AlphaMode::Blend)
        {
            return false;
        }
        else if (m2.first->GetMeshByIndex(m2.second)->GetMaterial()->alpha_mode == AlphaMode::Blend)
        {
            return true;
        }
        else
        {
            return dis1 < dis2;
        }
        });

    
    for (MeshPair& mesh_id : m_renderableMeshes)
    {
        SEEK_RETIF_FAIL(mesh_id.first->RenderMesh(mesh_id.second));
    }
    return S_Success;
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
