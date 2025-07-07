#include "effect/forward_shading_renderer.h"
#include "effect/technique.h"
#include "effect/postprocess.h"
#include "effect/effect.h"
#include "effect/shadow_layer.h"
#include "kernel/context.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_render_view.h"
#include "components/mesh_component.h"
#include "components/light_component.h"
#include "components/skeletal_mesh_component.h"
#include "components/camera_component.h"
#include "components/skybox_component.h"
#include "components/particle_component.h"
#include "components/watermark_component.h"
#include "scene_manager/scene_manager.h"
#include <algorithm>

#include "shader/shared/WaterMark.h"

#define SEEK_MACRO_FILE_UID 31     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

ForwardShadingRenderer::ForwardShadingRenderer(Context* context)
    : SceneRenderer(context)
{
    m_eRendererType = RendererType::Forward;
    m_pRenderSceneFB = m_pContext->RHIContextInstance().CreateRHIFrameBuffer(); 
}

SResult ForwardShadingRenderer::GetEffectTechniqueToRender(RHIMeshPtr mesh, Technique** tech)
{
    if (!tech)
        return ERR_INVALID_ARG;

    MorphInfo&      morph_info = mesh->GetMorphInfo();
    MorphTargetType morph_target_type = morph_info.morph_target_type;
    uint32_t        morph_count = (uint32_t)morph_info.morph_target_weights.size();

    auto& effect = m_pContext->EffectInstance();

    // Predefines
    std::vector<EffectPredefine> predefines;

    EffectPredefine morphTypePredefine;
    morphTypePredefine.name = "MORPH_TYPE";
    morphTypePredefine.value = std::to_string((int)morph_target_type);

    EffectPredefine jointBindSizePredefine;
    jointBindSizePredefine.name = "JOINT_BIND_SIZE";
    jointBindSizePredefine.value = std::to_string((int)mesh->GetSkinningJointBindSize());

    predefines.push_back({ "ENABLE_TAA" , m_pContext->GetAntiAliasingMode() == AntiAliasingMode::TAA ? "1" : "0" });

    VirtualTechnique* virtualTech = nullptr;

    switch (m_eCurRenderStage)
    {
        case RenderStage::GenerateShadowMap:
        case RenderStage::GenerateCubeShadowMap:
        case RenderStage::GenerateCascadedShadowMap:
        {
            predefines.push_back(jointBindSizePredefine);
            predefines.push_back(morphTypePredefine);
            
            if (m_eCurRenderStage == RenderStage::GenerateShadowMap)
                virtualTech = effect.GetVirtualTechnique("GenerateShadowMap");
            else if (m_eCurRenderStage == RenderStage::GenerateCubeShadowMap)
                virtualTech = effect.GetVirtualTechnique("GenerateCubeShadowMap");
            else
                virtualTech = effect.GetVirtualTechnique("GenerateCascadedShadowMap");
            break;
        }       

        case RenderStage::RenderScene:
        {
            EffectPredefine lightModePredefine;
            lightModePredefine.name = "LIGHT_MODE";
            lightModePredefine.value = std::to_string((int)m_pContext->GetLightingMode());

            uint32_t has_tex_normal = mesh->GetMaterial()->normal_tex ? 1 : 0;
            EffectPredefine hasNormalTexPredefine;
            hasNormalTexPredefine.name = "HAS_MATERIAL_NORMAL";
            hasNormalTexPredefine.value = std::to_string(has_tex_normal);

            predefines.push_back(jointBindSizePredefine);
            predefines.push_back(morphTypePredefine);
            predefines.push_back(lightModePredefine);
            predefines.push_back(hasNormalTexPredefine);

            if (mesh->GetMaterial()->albedo_tex)
                predefines.push_back({ "HAS_MATERIAL_ALBEDO", "1" });
            else
                predefines.push_back({ "HAS_MATERIAL_ALBEDO", "0" });

            if (mesh->GetMaterial()->metallic_roughness_tex)
                predefines.push_back({ "HAS_MATERIAL_METALLIC_ROUGHNESS", "1" });
            else
                predefines.push_back({ "HAS_MATERIAL_METALLIC_ROUGHNESS", "0" });

            if (mesh->GetMaterial()->normal_mask_tex)
                predefines.push_back({ "HAS_MATERIAL_NORMAL_MASK", "1" });
            else
                predefines.push_back({ "HAS_MATERIAL_NORMAL_MASK", "0" });
            /*if (mesh->GetMaterial()->occlusion_tex)
                predefines.push_back({ "HAS_MATERIAL_OCCLUSION", "1" });
            else
                predefines.push_back({ "HAS_MATERIAL_OCCLUSION", "0" });
            if (m_pContext->HasPrecomputedIBL())
                predefines.push_back({ "HAS_IBL", "1" });
            else
                predefines.push_back({ "HAS_IBL", "0" });*/

            virtualTech = effect.GetVirtualTechnique("ForwardRenderingCommon");
            break;
        }
        case RenderStage::None:
        case RenderStage::GenerateGBuffer:
        case RenderStage::PreZ:
        case RenderStage::GenerateReflectiveShadowMap:
            break;
    }

    if (virtualTech == nullptr)
    {
        LOG_ERROR("ForwardShadingRenderer::GetEffectTechniqueToRender no valid technique in %d!", m_eCurRenderStage);
        return ERR_INVALID_SHADER;
    }

    *tech = virtualTech->Concrete(predefines);
    return *tech ? S_Success : ERR_INVALID_SHADER;
}
SResult ForwardShadingRenderer::Init()
{
    if (!m_pShadowLayer)
    {
        m_pShadowLayer = MakeSharedPtr<ForwardShadowLayer>(m_pContext);
        SResult ret = m_pShadowLayer->InitResource();
        if (SEEK_CHECKFAILED(ret))
        {
            LOG_ERROR_PRIERR(ret, "ForwardShadingRenderer::Init Shadow InitResource fail.");
            return ret;
        }
    }
    return SceneRenderer::Init();
}
SResult ForwardShadingRenderer::BuildRenderJobList()
{
    m_vRenderingJobs.clear();

    m_renderableMeshes = m_pContext->SceneManagerInstance().QueryMesh([](const MeshPair& mesh)->bool {
        const auto& mesh_ = mesh.first->GetMeshByIndex(mesh.second);
        if (mesh_->IsVisible())
            return true;
        else
            return false;
    });
    if (m_renderableMeshes.empty() &&
        m_pContext->SceneManagerInstance().GetSkyBoxComponent() == nullptr &&
        m_pContext->SceneManagerInstance().GetParticleComponents().size() == 0)
        return S_Success;

    // analyze the whole pipeline by the configuration, and prepare the framebuffer for each render pass
    // in furture, need a framegraph to do this
    SResult ret = PrepareFrameBuffer();


    // Shadow Map jobs
    SceneManager& sm = m_pContext->SceneManagerInstance();
    size_t light_count = sm.NumLightComponent();
	if (light_count > 0)
	{
        m_pShadowLayer->AnalyzeLightShadow();
        for (size_t i = 0; i < light_count; i++)
        {
            LightComponent* pLight = sm.GetLightComponentByIndex(i);
            if (pLight->IsEnable() && pLight->CastShadow())
            {
                this->AppendShadowMapJobs((uint32_t)i);
            }
        }
	}

    if (m_pContext->SceneManagerInstance().GetSkyBoxComponent())
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ForwardShadingRenderer::RenderSkyBoxJob, this)));
    if (m_pContext->SceneManagerInstance().GetParticleComponents().size() > 0)
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ForwardShadingRenderer::RenderParticlesJob, this)));
    
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ForwardShadingRenderer::RenderSceneJob, this)));
    
    if (m_pContext->SceneManagerInstance().GetWaterMarkComponents().size() > 0)
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ForwardShadingRenderer::WatermarkJob, this)));
    if (m_pContext->IsHDR())
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::ToneMappingJob, this)));    

    // the last job should be FinishJob()
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::FinishJob, this)));
    return S_Success;
}

RendererReturnValue ForwardShadingRenderer::RenderSceneJob()
{
    SResult res;
    m_eCurRenderStage = RenderStage::RenderScene;
    m_pContext->RHIContextInstance().BeginRenderPass({"RenderScene" , m_pRenderSceneFB.get()});
    this->RenderScene();
    m_pContext->RHIContextInstance().EndRenderPass();
    m_eCurRenderStage = RenderStage::None;
    m_pRenderSceneFB->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, RHIFrameBuffer::LoadAction::Load);
    m_pRenderSceneFB->SetDepthLoadOption(RHIFrameBuffer::LoadAction::Load);
    return RRV_NextJob;
}
RendererReturnValue ForwardShadingRenderer::RenderSkyBoxJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_pContext->RHIContextInstance().BeginRenderPass({ "RenderSkybox", m_pRenderSceneFB.get()});
    SkyBoxComponent* skybox = m_pContext->SceneManagerInstance().GetSkyBoxComponent();
    if (skybox)
        skybox->Render();
    m_pContext->RHIContextInstance().EndRenderPass();
	m_pRenderSceneFB->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, RHIFrameBuffer::LoadAction::Load);
    m_pRenderSceneFB->SetDepthLoadOption(RHIFrameBuffer::LoadAction::Load);

    return RRV_NextJob;
}
RendererReturnValue ForwardShadingRenderer::WatermarkJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_pContext->RHIContextInstance().BeginRenderPass({ "Watermark", m_pRenderSceneFB.get() });
    std::vector<WaterMarkComponent*>& watermarks = m_pContext->SceneManagerInstance().GetWaterMarkComponents();
    for (uint32_t i = 0; i < watermarks.size(); ++i)
    {
        WaterMarkComponent* watermark = watermarks[i];
        if (watermark)
        {
            watermark->Render();
        }
    }
    m_pContext->RHIContextInstance().EndRenderPass();
    return RRV_NextJob;
}
RendererReturnValue ForwardShadingRenderer::RenderParticlesJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_pContext->RHIContextInstance().BeginRenderPass({ "RenderParticles", m_pRenderSceneFB.get() });
    std::vector<ParticleComponent*>& particles = m_pContext->SceneManagerInstance().GetParticleComponents();
    for (uint32_t i = 0; i < particles.size(); ++i)
    {
        ParticleComponent* particle = particles[i];
        if (particle)
            particle->Render();
    }
    m_pContext->RHIContextInstance().EndRenderPass();
    m_pRenderSceneFB->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, RHIFrameBuffer::LoadAction::Load);
    m_pRenderSceneFB->SetDepthLoadOption(RHIFrameBuffer::LoadAction::Load);
    return RRV_NextJob;
}
SResult ForwardShadingRenderer::PrepareFrameBuffer()
{
    // now, we will config framebuffer everytime
    
    // when render size changed, all the intermediate resources need to rebuild
    if (m_bRenderSizeChanged)
    {
        m_pRenderSceneColorTex = nullptr;
        m_pRenderSceneDepthTex = nullptr;
    }
    
    m_pRenderSceneFB->Reset();

    // TODO: need a better solution to handle the intermediate resources
    RHIContext& rc = m_pContext->RHIContextInstance();
    if (m_pContext->IsHDR())
    {
        if (!m_pToneMappingPostProcess)
        {
            m_pToneMappingPostProcess = MakeSharedPtr<PostProcess>(m_pContext, "ToneMapping");
            m_pToneMappingPostProcess->Init("ToneMapping");
        }
        else
        {
            m_pToneMappingPostProcess->GetFrameBuffer()->Reset();
        }
        
        RHITexture::Desc desc;
        if (!m_pRenderSceneColorTex)
        {
            PixelFormat pf = PixelFormat::R16G16B16A16_FLOAT;
            if (!rc.GetCapabilitySet().IsTextureSupport(pf, TextureFormatSupportType::RenderTarget))
                pf = PixelFormat::R32G32B32A32_FLOAT;
            
            desc.type = TextureType::Tex2D;
            desc.width = m_RenderViewport.width;
            desc.height = m_RenderViewport.height;
            desc.depth = 1;
            desc.num_mips = 1;
            desc.num_samples = 1;
            desc.format = pf;
            desc.flags = RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_GPU_READ;
            m_pRenderSceneColorTex = rc.CreateTexture2D(desc);
        }
        
        if (!m_pRenderSceneDepthTex)
        {
            desc.format = PixelFormat::D32F;
            desc.flags = RESOURCE_FLAG_GPU_WRITE;
            m_pRenderSceneDepthTex = rc.CreateTexture2D(desc);
        }
        
        m_pRenderSceneFB->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.Create2DRtv(m_pRenderSceneColorTex));
        m_pRenderSceneFB->AttachDepthStencilView(rc.CreateDepthStencilView(m_pRenderSceneDepthTex));
        
        m_pToneMappingPostProcess->SetParam("src_rgba", m_pRenderSceneColorTex);
        const RHIFrameBuffer* finalFB = rc.GetFinalRHIFrameBuffer().get();
        m_pToneMappingPostProcess->SetOutput(0, finalFB->GetRenderTarget(RHIFrameBuffer::Attachment::Color0));
        m_pToneMappingPostProcess->GetFrameBuffer()->SetViewport(m_RenderViewport);
    }     
    else
    {
        const RHIFrameBuffer* finalFB = rc.GetFinalRHIFrameBuffer().get();
        m_pRenderSceneFB->AttachTargetView(RHIFrameBuffer::Attachment::Color0, finalFB->GetRenderTarget(RHIFrameBuffer::Attachment::Color0));
        m_pRenderSceneFB->AttachDepthStencilView(finalFB->GetDepthStencilView());
    }
    
    m_pRenderSceneFB->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, { m_pContext->GetClearColor() });
    m_pRenderSceneFB->SetColorStoreOption(RHIFrameBuffer::Attachment::Color0, { RHIFrameBuffer::StoreAction::Store });
    m_pRenderSceneFB->SetDepthLoadOption({ 1.0f });
    m_pRenderSceneFB->SetDepthStoreOption({ RHIFrameBuffer::StoreAction::DontCare });
    m_pRenderSceneFB->SetSampleNum(m_pContext->GetNumSamples());
    m_pRenderSceneFB->SetViewport(m_FinalViewport);
    
    return S_Success;
}
void ForwardShadingRenderer::AppendShadowMapJobs(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);
    if (!pLight) return;

    LightType type = pLight->GetLightType();
    switch (type)
    {
    case LightType::Point:
    {
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ShadowLayer::GenerateShadowMapJob, m_pShadowLayer.get(), light_index)));
        break;
    }
    case LightType::Directional:
    case LightType::Spot:
    {
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ShadowLayer::GenerateShadowMapJob, m_pShadowLayer.get(), light_index)));
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ForwardShadowLayer::PostProcessShadowMapJob, (ForwardShadowLayer*)m_pShadowLayer.get(), light_index)));
        break;
    }
    case LightType::Ambient:
    case LightType::Unknown:
    case LightType::Num:
        break;
    }
    return;
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
