#include "effect/forward_shading_renderer.h"
#include "effect/technique.h"
#include "effect/postprocess.h"
#include "effect/effect.h"
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
#include "scene_manager/scene_manager.h"
#include <algorithm>

#define SEEK_MACRO_FILE_UID 31     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

ForwardShadingRenderer::ForwardShadingRenderer(Context* context)
    : SceneRenderer(context)
{
    m_eRendererType = RendererType::Forward;
    m_pRenderSceneFB = m_pContext->RHIContextInstance().CreateEmptyRHIFrameBuffer();
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

    VirtualTechnique* virtualTech = nullptr;

    switch (m_eCurRenderStage)
    {
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
            if (mesh->GetMaterial()->occlusion_tex)
                predefines.push_back({ "HAS_MATERIAL_OCCLUSION", "1" });
            else
                predefines.push_back({ "HAS_MATERIAL_OCCLUSION", "0" });
            if (m_pContext->HasPrecomputedIBL())
                predefines.push_back({ "HAS_IBL", "1" });
            else
                predefines.push_back({ "HAS_IBL", "0" });

            virtualTech = effect.GetVirtualTechnique("ForwardRenderingCommon");
            break;
        }
        case RenderStage::None:
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
    if (m_renderableMeshes.empty())
        return S_Success;

    // analyze the whole pipeline by the configuration, and prepare the framebuffer for each render pass
    // in furture, need a framegraph to do this
    SResult ret = PrepareFrameBuffer();
    
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ForwardShadingRenderer::RenderSceneJob, this)));
    
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
    if (m_renderableMeshes.empty())
        return RRV_NextJob;

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
    
    RHIContext::RenderPassInfo info;
    info.name = "RenderScene";
    info.fb = m_pRenderSceneFB.get();
    m_pContext->RHIContextInstance().BeginRenderPass(info);
    for (MeshPair& mesh_id : m_renderableMeshes)
    {
        res = mesh_id.first->RenderMesh(mesh_id.second);
    }
    m_pContext->RHIContextInstance().EndRenderPass();
    
    m_eCurRenderStage = RenderStage::None;
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
            desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE;
            m_pRenderSceneColorTex = rc.CreateTexture2D(desc);
        }
        
        if (!m_pRenderSceneDepthTex)
        {
            desc.format = PixelFormat::D32F;
            desc.flags = RESOURCE_FLAG_RENDER_TARGET;
            m_pRenderSceneDepthTex = rc.CreateTexture2D(desc);
        }
        
        m_pRenderSceneFB->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pRenderSceneColorTex));
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

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
