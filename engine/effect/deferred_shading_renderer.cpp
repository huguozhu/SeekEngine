#include "effect/deferred_shading_renderer.h"
#include "effect/technique.h"
#include "effect/postprocess.h"
#include "effect/effect.h"
//#include "effect/hdr_postprocess.h"
#include "effect/shadow_layer.h"
#include "effect/effect.h"
//#include "effect/blur.h"
//#include "effect/gi.h"
#include "kernel/context.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_query.h"
#include "components/mesh_component.h"
#include "components/light_component.h"
#include "components/skeletal_mesh_component.h"
#include "components/skybox_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"


#define DVF_MACRO_FILE_UID 87     // this code is auto generated, don't touch it!!!

#define SSAO_KERNEL_COUNT 64

SEEK_NAMESPACE_BEGIN

static bool use_tile_culling = 0;

LightInfo DeferredShadingRenderer::s_LightInfos[MAX_DEFERRED_LIGHTS_NUM];

DeferredShadingRenderer::DeferredShadingRenderer(Context* context)
    : SceneRenderer(context)
{
    m_eRendererType = RendererType::Deferred;
}

SResult DeferredShadingRenderer::Init()
{
    SResult ret = S_Success;
    RHIContext& rc = m_pContext->RHIContextInstance();

    m_pQuadMesh = QuadMesh_GetMesh(rc);

    // Step1: Base Init & Create Shadow Layer
    ret = SceneRenderer::Init();

    if (m_pContext->EnableShadow())
    {
        if (!m_pShadowLayer)
        {
            m_pShadowLayer = MakeSharedPtr<DeferredShadowLayer>(m_pContext);
            SResult ret = m_pShadowLayer->InitResource();
            if (SEEK_CHECKFAILED(ret))
            {
                LOG_ERROR_PRIERR(ret, "DeferredShadingRenderer::Init Shadow InitResource fail.");
                return ret;
            }
        }
    }

    // to do: temp w/h to use
	uint32_t tmp_w = 1280;
	uint32_t tmp_h = 720;

    // Step2: Create Texture
    RHITexture::Desc desc;
    desc.type = TextureType::Tex2D;
    desc.width = tmp_w;
    desc.height = tmp_h;
    desc.depth = 1;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
    m_pGBufferColor0 = rc.CreateTexture2D(desc);
    m_pGBufferColor1 = rc.CreateTexture2D(desc);
    m_pGBufferColor2 = rc.CreateTexture2D(desc);
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pPreZColor = rc.CreateTexture2D(desc);
    desc.format = PixelFormat::R8_UNORM;
    m_pSsaoColor = rc.CreateTexture2D(desc);

    // Step3 Create FrameBuffer
    RHIRenderViewPtr ds_view = rc.CreateDepthStencilView(m_pSceneDepthStencil);
    m_pPreZFb = rc.CreateEmptyRHIFrameBuffer();
    m_pPreZFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pPreZColor));
    m_pPreZFb->AttachDepthStencilView(ds_view);

    m_pGBufferFb = rc.CreateEmptyRHIFrameBuffer();
    m_pGBufferFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pGBufferColor0));
    m_pGBufferFb->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pGBufferColor1));
    m_pGBufferFb->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pGBufferColor2));
    m_pGBufferFb->AttachDepthStencilView(ds_view);


    desc.format = PixelFormat::D32F;
    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
    RHITexturePtr lighting_ds_tex = rc.CreateTexture2D(desc);
    m_pLightingFb = rc.CreateEmptyRHIFrameBuffer();
    m_pLightingFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pHDRColor));
    m_pLightingFb->AttachDepthStencilView(rc.CreateDepthStencilView(lighting_ds_tex));

    m_pSsaoFb = rc.CreateEmptyRHIFrameBuffer();
    m_pSsaoFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pSsaoColor));

    // Step4: Shadowing
    desc.width = tmp_w / 2;
    desc.height = tmp_h / 2;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pShadowTex = rc.CreateTexture2D(desc);
    m_pShadowingFb = rc.CreateEmptyRHIFrameBuffer();
    m_pShadowingFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pShadowTex));


    // Step5: Effect & Technique & EffectParam
    //m_pEffect0 = this->GetDeferredEffect(0);
    Effect& effect = m_pContext->EffectInstance();
    
    std::vector<EffectPredefine> has_shadow_predefines = { {"HAS_SHADOW" , "1"}, {"TILE_CULLING", "0"} };
    m_pLightingTech_HasShadow = m_pEffect0->GetTechnique("DeferredLighting", has_shadow_predefines);

    std::vector<EffectPredefine> no_shadow_predefines = { {"HAS_SHADOW" , "0"}, {"TILE_CULLING", "0"} };
    m_pLightingTech_NoShadow = m_pEffect0->GetTechnique("DeferredLighting", no_shadow_predefines);

    m_pParamCameraInfo = m_pEffect0->GetParamByName("cameraInfo");
    m_pParamGBuffer0 = m_pEffect0->GetParamByName("gbuffer0");
    m_pParamGBuffer1 = m_pEffect0->GetParamByName("gbuffer1");
    m_pParamDepthTex = m_pEffect0->GetParamByName("depth_tex");
    m_pParamShadowingTex = m_pEffect0->GetParamByName("shadowing_tex");
    m_pParamDeferredInfo = m_pEffect0->GetParamByName("deferredLightingInfo");
    m_pParamLightInfos = m_pEffect0->GetParamByName("light_infos");

    std::vector<float4> ssao_kernels;
    for (uint32_t i = 0; i < SSAO_KERNEL_COUNT; ++i)
    {
        // Semisphere
        float4 random4 = Math::GenerateRandom(float4(0.0), float4(1.0));
        float3 sample(random4[0] * 2.0 - 1.0, random4[1] * 2.0 - 1.0, random4[2]);
        sample = Math::Normalize(sample);
        sample *= random4[3];

        // scale samples s.t. they're more aligned to center of kernel
        float scale = float(i) / (float)SSAO_KERNEL_COUNT;
        scale = Math::Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssao_kernels.push_back(float4(sample[0], sample[1], sample[2], 0.0f));
    }

    std::vector<float2> ssao_noise;
    for (uint32_t i = 0; i < 16; i++)
    {
        float4 random2 = Math::GenerateRandom(float4(-1.0), float4(1.0));
        float2 noise(random2[0], random2[1]);
        ssao_noise.push_back(noise);
    }
    BitmapBufferPtr ssao_noise_bitmap = MakeSharedPtr<BitmapBuffer>(4, 4, PixelFormat::R32G32F, (uint8_t*)ssao_noise.data(), (uint32_t)(sizeof(float2) * 4));
    desc.type = TextureType::Tex2D;
    desc.width = desc.height = 4;
    desc.depth = 1;
    desc.num_mips = desc.num_samples = 1;
    desc.format = PixelFormat::R32G32F;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE;
    m_pSsaoNoise = rc.CreateTexture2D(desc, ssao_noise_bitmap);

    m_pSsaoEffect = MakeSharedPtr<Effect>(m_pContext);
    m_pSsaoEffect->Load("ssao.effect");
    m_pSsaoTech = m_pSsaoEffect->GetTechnique("SSAO");

    *(m_pSsaoEffect->GetParamByName("gbuffer0")) = m_pGBufferColor0;
    *(m_pSsaoEffect->GetParamByName("depth_tex")) = m_pSceneDepthStencil;
    *(m_pSsaoEffect->GetParamByName("ssao_noise")) = m_pSsaoNoise;
    m_pSsaoEffect->GetParamByName("ssao_sample_kernels")->UpdateConstantBuffer(&ssao_kernels[0], sizeof(ssao_kernels[0]) * ssao_kernels.size());
    float2 ssao_scale = float2((float)renderer_width / 4.0, (float)renderer_height / 4.0);
    m_pSsaoEffect->GetParamByName("ssao_scale")->UpdateConstantBuffer(&ssao_scale, sizeof(ssao_scale));
    m_pParamSsaoCameraInfo = m_pSsaoEffect->GetParamByName("cameraInfo");
    m_pParamSsaoViewMatrix = m_pSsaoEffect->GetParamByName("view_matrix");
    m_pParamSsaoProjMatrix = m_pSsaoEffect->GetParamByName("proj_matrix");
    m_pParamSsaoInvProjMatrix = m_pSsaoEffect->GetParamByName("inv_proj_matrix");

    m_pGaussianBlur = MakeSharedPtr<GaussianBlur>(m_pContext);
    m_pGaussianBlur->SetSrcTexture(m_pSsaoColor);
    m_pGaussianBlur->SetDstTexture(m_pSsaoColor);

    GlobalIlluminationMode mode = m_pContext->GetRenderInitInfo().gi_mode;
    if (mode == GlobalIlluminationMode::RSM)
    {
        m_pGI = MakeSharedPtrMacro(RSM, m_pContext);
        ((RSM*)m_pGI.get())->Init(m_pGBufferColor0, m_pGBufferColor1, m_pSceneDepthStencil);
    }

    if (m_pContext->IsProfile())
    {
        m_pTimeQueryGenShadowMap = rc.CreateTimeQuery();
        m_pTimeQueryGenGBuffer = rc.CreateTimeQuery();
        m_pTimeQuerySSAO = rc.CreateTimeQuery();
        m_pTimeQueryLihgtCulling = rc.CreateTimeQuery();
        m_pTimeQueryLighting = rc.CreateTimeQuery();
        m_pTimeQueryGI = rc.CreateTimeQuery();
        m_pTimeQuerySkybox = rc.CreateTimeQuery();
        m_pTimeQueryHDR = rc.CreateTimeQuery();
        m_pTimeQueryLDR = rc.CreateTimeQuery();
    }
    m_pLightInfoBuffer = rc.CreateByteAddressBuffer(sizeof(LightInfo) * MAX_DEFERRED_LIGHTS_NUM, RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_CPU_WRITE, nullptr);

    // Tile Culling
    if (use_tile_culling)
    {
        uint32_t tile_width =  (renderer_width  + TILE_SIZE - 1) / TILE_SIZE;
        uint32_t tile_height = (renderer_height + TILE_SIZE - 1) / TILE_SIZE;
        m_pTileInfoBuffer = rc.CreateRWByteAddressBuffer(sizeof(TileInfo) * tile_width * tile_height, RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_SHADER_WRITE | RESOURCE_FLAG_COPY_BACK, nullptr);

        m_pTileCullingTech = m_pEffect0->GetTechnique("LightCullingCS");

        std::vector<EffectPredefine> no_shadow_tile_culling_predefines = { {"HAS_SHADOW" , "1"}, {"TILE_CULLING", "1"} };
        m_pLightingTech_Shadow_TileCulling = m_pEffect0->GetTechnique("DeferredLighting", no_shadow_tile_culling_predefines);

        m_pParamViewMatrix = m_pEffect0->GetParamByName("c_view_matrix");
        m_pParamProjMatrix = m_pEffect0->GetParamByName("c_proj_matrix");
        m_pParamFrameSize  = m_pEffect0->GetParamByName("c_frame_size");
        m_pParamTileInfoRW = m_pEffect0->GetParamByName("tile_infos_rw");
        m_pParamTileInfo   = m_pEffect0->GetParamByName("tile_infos");
    }
    return ret;
}
DVFResult DeferredShadingRenderer::BuildRenderJobList()
{
    m_vRenderingJobs.clear();
    SceneManager& sm = m_pContext->SceneManagerInstance();
    size_t light_count = sm.NumLightComponent();
    if (light_count == 0)
        return DVF_Success;

    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::RenderPrepareJob, this)));
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::RenderPreZJob, this)));

    // Shadow Map job
    BEGIN_TIMEQUERY(m_pTimeQueryGenShadowMap);
    if (m_pContext->IsUseShadow())
    {
        m_pShadowLayer->AnalyzeLightShadow();
        for (uint32_t i = 0; i < light_count; i++)
        {
            LightComponent* pLight = sm.GetLightComponentByIndex((uint32_t)i);
            if (pLight->IsEnable() && pLight->CastShadow())
            {
                this->AppendShadowMapJobs(i);
            }
        }
    }
    END_TIMEQUERY(m_pTimeQueryGenShadowMap);
    
    BEGIN_TIMEQUERY(m_pTimeQueryGenGBuffer);
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::GenerateGBufferJob, this)));
    END_TIMEQUERY(m_pTimeQueryGenGBuffer);

    BEGIN_TIMEQUERY(m_pTimeQuerySSAO);
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::SSAOJob, this)));
    END_TIMEQUERY(m_pTimeQuerySSAO);

    if (use_tile_culling)
    {
        BEGIN_TIMEQUERY(m_pTimeQueryLihgtCulling);
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::LightingTileCullingJob, this)));
        END_TIMEQUERY(m_pTimeQueryLihgtCulling);
    }
    else
    {
        BEGIN_TIMEQUERY(m_pTimeQueryLighting);
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::LightingJob, this)));
        END_TIMEQUERY(m_pTimeQueryLighting);
    }
    

    BEGIN_TIMEQUERY(m_pTimeQueryGI);
    if (m_pContext->GetRenderInitInfo().gi_mode != GlobalIlluminationMode::None)
        for (uint32_t i = 0; i < light_count; i++)
            this->AppendGIJobs(i);
    END_TIMEQUERY(m_pTimeQueryGI);
    
    
    BEGIN_TIMEQUERY(m_pTimeQuerySkybox);
    if (sm.GetSkyBoxComponent())
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::RenderSkyBoxJob, this)));
    END_TIMEQUERY(m_pTimeQuerySkybox);
    
    BEGIN_TIMEQUERY(m_pTimeQueryHDR);
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::HDRJob, this)));
    END_TIMEQUERY(m_pTimeQueryHDR);

    BEGIN_TIMEQUERY(m_pTimeQueryLDR);
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::LDRJob, this)));
    END_TIMEQUERY(m_pTimeQueryLDR);

    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::CalculateRenderRectJob, this)));

    if (m_pContext->IsProfile())
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::PrintTimeQueryJob, this))); ;

    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::FinishJob, this)));
    m_pCurJobIterator = m_vRenderingJobs.begin();

    return DVF_Success;
}
void DeferredShadingRenderer::AppendShadowMapJobs(uint32_t light_index)
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
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadowLayer::PostProcessShadowMapJob, (DeferredShadowLayer*)m_pShadowLayer.get(), light_index)));
        break;
    }
    case LightType::Directional:
    {
        if (pLight->CascadedShadow())
            m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ShadowLayer::GenerateCascadedShadowMapJob, m_pShadowLayer.get(), light_index)));
        else
            m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ShadowLayer::GenerateShadowMapJob, m_pShadowLayer.get(), light_index)));
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadowLayer::PostProcessShadowMapJob, (DeferredShadowLayer*)m_pShadowLayer.get(), light_index)));
        break;
    }
    case LightType::Spot:
    {
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&ShadowLayer::GenerateShadowMapJob, m_pShadowLayer.get(), light_index)));
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadowLayer::PostProcessShadowMapJob, (DeferredShadowLayer*)m_pShadowLayer.get(), light_index)));
        break;
    }
    case LightType::Ambient:
    case LightType::Unknown:
    case LightType::Num:
        break;
    }
    return;
}
DVFResult DeferredShadingRenderer::GetEffectTechniqueToRender(MeshPtr mesh, Effect** effect, Technique** tech)
{
    if (!effect || !tech)
        return ERR_INVALID_ARG;

    MorphInfo& morph_info = mesh->GetMorphInfo();
    MorphTargetType morph_target_type = morph_info.morph_target_type;
    uint32_t        morph_count = (uint32_t)morph_info.morph_target_weights.size();

    if (IsNowShadowStage())
        *effect = m_pShadowLayer->GetShadowEffect(morph_count);
    else
        *effect = this->GetDeferredEffect(morph_count);
    if (*effect == nullptr)
    {
        LOG_ERROR("DeferredShadingRenderer::GetEffectTechniqueToRender Invalid effect!");
        return ERR_INVALID_SHADER;
    }

    // Predefines
    std::vector<EffectPredefine> predefines;

    EffectPredefine jointBindSizePredefine;
    jointBindSizePredefine.name = "JOINT_BIND_SIZE";
    jointBindSizePredefine.value = std::to_string((int)mesh->GetSkinningJointBindSize());

    EffectPredefine morphTypePredefine;
    morphTypePredefine.name = "MORPH_TYPE";
    morphTypePredefine.value = std::to_string((int)morph_target_type);
    
    predefines.push_back(jointBindSizePredefine);
    predefines.push_back(morphTypePredefine);

    switch (m_eCurRenderStage)
    {
    case RenderStage::GenerateShadowMap:            *tech = (*effect)->GetTechnique("GenerateShadowMap",            predefines);    break;
    case RenderStage::GenerateCubeShadowMap:        *tech = (*effect)->GetTechnique("GenerateCubeShadowMap",        predefines);    break;    
    case RenderStage::PreZ:                         *tech = (*effect)->GetTechnique("PreZ",                         predefines);    break;
    case RenderStage::GenerateGBuffer:
    case RenderStage::GenerateReflectiveShadowMap:
    case RenderStage::GenerateCascadedShadowMap:
    {
        uint32_t has_tex_normal = mesh->GetMaterial()->normal_tex ? 1 : 0;
        EffectPredefine hasNormalTexPredefine;
        hasNormalTexPredefine.name = "HAS_NORMAL_TEX";
        hasNormalTexPredefine.value = std::to_string(has_tex_normal);

        EffectPredefine enableTAAPredefine;
        enableTAAPredefine.name = "ENABLE_TAA";
        enableTAAPredefine.value = m_pContext->GetAntiAliasingMode() == AntiAliasingMode::TAA ? "1" : "0";

        predefines.push_back(hasNormalTexPredefine);
        predefines.push_back(enableTAAPredefine);

        if (m_eCurRenderStage == RenderStage::GenerateGBuffer)
            *tech = (*effect)->GetTechnique("GenerateGBuffer", predefines);
        else if (m_eCurRenderStage == RenderStage::GenerateReflectiveShadowMap)
            *tech = (*effect)->GetTechnique("GenerateReflectiveShadowMap", predefines);
        else
            *tech = (*effect)->GetTechnique("GenerateCascadedShadowMap", predefines);
        break;
    }
    case RenderStage::None:
    case RenderStage::Sprite2D:
    case RenderStage::RenderScene:
    default:
        LOG_ERROR("DeferredShadingRenderer::GetEffectTechniqueToRender invalid RenderStage");
    }
    if (*tech == nullptr)
    {
        LOG_ERROR("DeferredShadingRenderer::GetEffectTechniqueToRender Invalid technique!");
        return ERR_INVALID_SHADER;
    }

    return DVF_Success;
}
void DeferredShadingRenderer::AppendGIJobs(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);
    if (!pLight || !pLight->IndirectLighting()) 
        return;

    LightType type = pLight->GetLightType();
    switch (type)
    {
    case LightType::Point:
    {
        break;
    }
    case LightType::Directional:
    case LightType::Spot:
    {
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&RSM::GenerateReflectiveShadowMapJob, (RSM*)m_pGI.get(), light_index)));
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&RSM::PostProcessReflectiveShadowMapJob, (RSM*)m_pGI.get(), light_index)));
        break;
    }
    case LightType::Ambient:
    case LightType::Unknown:
    case LightType::Num:
        break;
    }
    return;
}
bool DeferredShadingRenderer::IsNeedShaderInvariant(RenderStage stage)
{
    switch (stage)
    {
    case dvf::RenderStage::PreZ:
    case dvf::RenderStage::GenerateGBuffer:
        return true;
        break;
    case dvf::RenderStage::None:
    case dvf::RenderStage::GenerateShadowMap:
    case dvf::RenderStage::GenerateCubeShadowMap:
    case dvf::RenderStage::GenerateCascadedShadowMap:
    case dvf::RenderStage::RenderScene:
    case dvf::RenderStage::Sprite2D:
    default:
        return false;
        break;
    }
}
Effect* DeferredShadingRenderer::GetDeferredEffect(uint32_t morph_count)
{
    Effect* pEffect = nullptr;
    if (m_Effects.find(morph_count) != m_Effects.end())
        pEffect = m_Effects[morph_count].get();
    else
    {
        static std::string morhp_size_macro_name = "MORPH_SIZE";
        std::string morph_size_value = std::to_string(morph_count);
        std::vector<std::pair<std::string, std::string>> v;
        v.push_back(std::make_pair(morhp_size_macro_name, morph_size_value));

        // FIXME: BAD! we load same effect files many times
        EffectPtr effect_new = MakeSharedPtr<Effect>(m_pContext);
        DVFResult res = effect_new->Load("deferred_shading.effect", &v);
        if (res != DVF_Success)
        {
            LOG_ERROR("DeferredShadingRenderer::LightingJob() Load failed.");
            return nullptr;
        }
        m_Effects[morph_count] = effect_new;
        pEffect = effect_new.get();
    }
    return pEffect;
}
MeshPtr DeferredShadingRenderer::GetLightVolumeMesh(LightType type)
{
    MeshPtr pMesh = nullptr;
    RenderContext& rc = m_pContext->RenderContextInstance();
    switch (type)
    {
    case LightType::Directional:    pMesh = m_pQuadMesh;            break;
    case LightType::Spot:           pMesh = rc.GetConeMesh();       break;
    case LightType::Point:          pMesh = rc.GetCubeMesh();       break;
    case LightType::Ambient:        pMesh = m_pQuadMesh;            break;
    case LightType::Unknown:
    case LightType::Num:            LOG_ERROR("Invalid LightType"); break;
    }
    return pMesh;
}
Technique* DeferredShadingRenderer::GetLightingTechByLightType(LightType type)
{
    Effect* pEffect = this->GetDeferredEffect(0);
    return nullptr;
}
RendererReturnValue DeferredShadingRenderer::RenderPrepareJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_pPreZFb->Clear();
    m_pGBufferFb->Clear();
    m_pLightingFb->Clear();
    m_pSsaoFb->Clear();
    if (m_pGI)
        m_pGI->OnBegin();
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::RenderPreZJob()
{
    m_eCurRenderStage = RenderStage::PreZ;

    DVFResult res = m_pContext->RenderContextInstance().BindFrameBuffer(m_pPreZFb);
    if (res != DVF_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::RenderPreZJob() BindFrameBuffer failed.");
    }

    res = m_pContext->SceneManagerInstance().RenderScene((uint32_t)RenderScope::Opacity);
    if (res != DVF_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::RenderPreZJob() RenderScene failed.");
    }
    m_eCurRenderStage = RenderStage::None;

#if 0
    static int draw = 1;
    if (draw)
    {
        m_pSceneDepthStencil->DumpToFile("d:\\depth_map.g16l");
        draw--;
    }
#endif
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::GenerateGBufferJob()
{
    m_eCurRenderStage = RenderStage::GenerateGBuffer;
    DVFResult res = m_pContext->RenderContextInstance().BindFrameBuffer(m_pGBufferFb);
    if (res != DVF_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::GenerateGBufferJob() BindFrameBuffer failed.");
    }

    res = m_pContext->SceneManagerInstance().RenderScene((uint32_t)RenderScope::Opacity);
    if (res != DVF_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::GenerateGBufferJob() RenderScene failed.");
    }
    m_eCurRenderStage = RenderStage::None;

#if 0
    static int draw = 1;
    if (draw)
    {
        m_pGBufferColor0->DumpToFile("d:\\GBuffer_RT0.rgba");
        m_pGBufferColor1->DumpToFile("d:\\GBuffer_RT1.rgba");
        m_pGBufferColor2->DumpToFile("d:\\GBuffer_RT2.rgba");
        draw--;
    }
#endif
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::SSAOJob()
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RenderContext& rc = m_pContext->RenderContextInstance();    
    DVFResult res = m_pContext->RenderContextInstance().BindFrameBuffer(m_pSsaoFb);
    if (res != DVF_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::SSAOJob() BindFrameBuffer failed.");
        return RRV_NextJob;
    }
    CameraComponent* cam = sm.GetActiveCamera();
    CameraInfo cameraInfo;
    if (cam)
    {
        cameraInfo.posWorld = cam->GetWorldTransform().GetTranslation();
        cameraInfo.nearFarPlane = cam->GetNearFarPlane();
    }

    if (m_pQuadMesh || m_pSsaoEffect)
    {
        Matrix4 view_matrix = cam->GetViewMatrix().Transpose();
        Matrix4 proj_matrix = cam->GetProjMatrix().Transpose();
        Matrix4 inv_proj_matrix = cam->GetInvProjMatrix().Transpose();
        m_pParamSsaoCameraInfo->UpdateConstantBuffer(&cameraInfo, sizeof(cameraInfo));        
        m_pParamSsaoViewMatrix->UpdateConstantBuffer(&view_matrix, sizeof(view_matrix));        
        m_pParamSsaoProjMatrix->UpdateConstantBuffer(&proj_matrix, sizeof(proj_matrix));        
        m_pParamSsaoInvProjMatrix->UpdateConstantBuffer(&inv_proj_matrix, sizeof(inv_proj_matrix));
        
        m_pQuadMesh->SetRenderState(m_pSsaoTech->GetRenderState());
        res = rc.Render(m_pSsaoTech->GetProgram().get(), m_pQuadMesh);
        if (res != DVF_Success)
        {
            LOG_ERROR("DeferredShadingRenderer::LightingJob() Render() failed.");
            return RRV_NextJob;
        }
        m_pGaussianBlur->Run();
#if 0
        static int draw = 1;
        if (draw)
        {
            m_pSsaoColor->DumpToFile("d:\\ssao.gray");
            draw--;
        }
#endif
    }
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::LightingTileCullingJob()
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RenderContext& rc = m_pContext->RenderContextInstance();

    // Step1: Set Params
    CameraComponent* cam = sm.GetActiveCamera();
    CameraInfo cameraInfo;
    if (cam)
    {
        cameraInfo.posWorld = cam->GetWorldTransform().GetTranslation();
        cameraInfo.nearFarPlane = cam->GetNearFarPlane();
    }
    m_pParamCameraInfo->UpdateConstantBuffer(&cameraInfo, sizeof(cameraInfo));

    *m_pParamGBuffer0 = m_pGBufferColor0;
    *m_pParamGBuffer1 = m_pGBufferColor1;
    *m_pParamDepthTex = m_pSceneDepthStencil;
    *m_pParamShadowingTex = ((DeferredShadowLayer*)m_pContext->SceneRendererInstance().GetShadowLayer().get())->GetShadowingTex();

    Matrix4 view_matrix = cam->GetViewMatrix().Transpose();
    m_pParamViewMatrix->UpdateConstantBuffer(&view_matrix, sizeof(view_matrix));

    Matrix4 proj_matrix = cam->GetProjMatrix().Transpose();
    m_pParamProjMatrix->UpdateConstantBuffer(&proj_matrix, sizeof(proj_matrix));

    float2 frame_size = float2(m_pGBufferColor0->Width(), m_pGBufferColor0->Height());
    m_pParamFrameSize->UpdateConstantBuffer(&frame_size, sizeof(frame_size));


    DeferredLightingInfo deferred_info;
    deferred_info.lightVolumeMV = cam ? cam->GetInvProjMatrix().Transpose() : Matrix4::Identity();
    deferred_info.lightVolumeInvView = cam ? cam->GetInvViewMatrix().Transpose() : Matrix4::Identity();
    deferred_info.light_index_start = 0;
    deferred_info.light_num = (int)sm.NumLightComponent();
    m_pParamDeferredInfo->UpdateConstantBuffer(&deferred_info, sizeof(deferred_info));

    for (uint32_t i = 0; i < sm.NumLightComponent(); i++)
    {
        LightComponent* pLight = sm.GetLightComponentByIndex(i);
        this->FillLightInfoByLightIndex(s_LightInfos[i], cam, i);
    }
    RenderBufferData light_light_data(uint32_t(MAX_DEFERRED_LIGHTS_NUM * sizeof(LightInfo)), &s_LightInfos[0]);
    m_pLightInfoBuffer->Update(&light_light_data);
    *m_pParamLightInfos = m_pLightInfoBuffer;
    *m_pParamTileInfoRW = m_pTileInfoBuffer;
    *m_pParamTileInfo = m_pTileInfoBuffer;
    
    //Step1: Light culling
    uint32_t x = (m_pGBufferColor0->Width()  + TILE_SIZE - 1) / TILE_SIZE;
    uint32_t y = (m_pGBufferColor0->Height() + TILE_SIZE - 1) / TILE_SIZE;
    DVFResult res = rc.Dispatch(m_pTileCullingTech->GetProgram().get(), x, y, 1);
    if (res != DVF_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::LightingTileCullingJob() Culling failed.");
        return RRV_NextJob;
    }
#if 0
    static int draw = 1;
    if (draw)
    {
          
        uint32_t tile_width  = (m_pContext->GetRenderInitInfo().width  + TILE_SIZE - 1) / TILE_SIZE;
        uint32_t tile_height = (m_pContext->GetRenderInitInfo().height + TILE_SIZE - 1) / TILE_SIZE;
        static const uint32_t DATA_SIZE = 3600;
        static TileInfo data[DATA_SIZE] = {0};
        BufferPtr buf = MakeSharedPtr<Buffer>(sizeof(TileInfo) * DATA_SIZE, (uint8_t*)(&data[0]) );
        res = m_pTileInfoBuffer->CopyBack(buf, 0, -1);
        for (uint32_t i = 0; i < DATA_SIZE; i++)
        {
            if (data[i].light_num != 0)
                data[i].light_num = data[i].light_num;
        }
        draw--;
    }
#endif

    //Step2: Rendering after light culling
    res = m_pContext->RenderContextInstance().BindFrameBuffer(m_pLightingFb);
    if (res != DVF_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::LightingTileCullingJob() BindFrameBuffer failed.");
        return RRV_NextJob;
    }

    m_pQuadMesh->SetRenderState(m_pLightingTech_Shadow_TileCulling->GetRenderState());
    res = rc.Render(m_pLightingTech_Shadow_TileCulling->GetProgram().get(), m_pQuadMesh);
    if (res != DVF_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::LightingTileCullingJob() Render() failed.");
        return RRV_NextJob;
    }

    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::LightingJob()
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RenderContext& rc = m_pContext->RenderContextInstance();
        
    // Step1: Set Params
    Effect* pEffect = this->GetDeferredEffect(0);
    CameraComponent* cam = sm.GetActiveCamera();
    CameraInfo cameraInfo;
    if (cam)
    {
        cameraInfo.posWorld = cam->GetWorldTransform().GetTranslation();
        cameraInfo.nearFarPlane = cam->GetNearFarPlane();
    }
    m_pParamCameraInfo->UpdateConstantBuffer(&cameraInfo, sizeof(cameraInfo));

    *m_pParamGBuffer0 = m_pGBufferColor0;
    *m_pParamGBuffer1 = m_pGBufferColor1;
    *m_pParamDepthTex = m_pSceneDepthStencil;
    *m_pParamShadowingTex = ((DeferredShadowLayer*)m_pContext->SceneRendererInstance().GetShadowLayer().get())->GetShadowingTex();
    *m_pParamLightInfos = m_pLightInfoBuffer;
        
    DeferredLightingInfo deferred_info;    
    RenderBufferData light_light_data(uint32_t(MAX_DEFERRED_LIGHTS_NUM * sizeof(LightInfo)), &s_LightInfos[0]);
    deferred_info.lightVolumeMV = cam ? cam->GetInvProjMatrix().Transpose() : Matrix4::Identity();
    deferred_info.lightVolumeInvView = cam ? cam->GetInvViewMatrix().Transpose() : Matrix4::Identity();
    m_pParamDeferredInfo->UpdateConstantBuffer(&deferred_info, sizeof(deferred_info));

    // Step2: Calc Has-Shadow & No-Shadow lights
    size_t light_num = sm.NumLightComponent();
    std::vector<LightComponent*> light_has_shadow_list;
    std::vector<LightComponent*> light_no_shadow_list;
    light_num = light_num < MAX_DEFERRED_LIGHTS_NUM ? light_num : MAX_DEFERRED_LIGHTS_NUM;
    std::map<LightComponent*, uint32_t> cached_light_index;
    for (uint32_t i = 0; i < light_num; i++)
    {
        LightComponent* pLight = sm.GetLightComponentByIndex(i);
        if (!pLight->IsEnable())
            continue;
        if (pLight->CastShadow())
            light_has_shadow_list.push_back(pLight);
        else
            light_no_shadow_list.push_back(pLight);
        cached_light_index[pLight] = i;
    }
    uint32_t light_num_has_shadow = (uint32_t)light_has_shadow_list.size();
    uint32_t light_num_no_shadow = (uint32_t)light_no_shadow_list.size();
        
    // Step3: Render shadow light
    DVFResult res = m_pContext->RenderContextInstance().BindFrameBuffer(m_pLightingFb);
    if (light_num_has_shadow > 0)
    {
        for (uint32_t i = 0; i < light_num_has_shadow; i++)
        {
            LightComponent* pLight = light_has_shadow_list[i];
            this->FillLightInfoByLightIndex(s_LightInfos[i], cam, cached_light_index[pLight]);
        }
        deferred_info.light_index_start = 0;
        deferred_info.light_num = (int)light_num_has_shadow;
        m_pParamDeferredInfo->UpdateConstantBuffer(&deferred_info, sizeof(deferred_info));
        m_pLightInfoBuffer->Update(&light_light_data);

        m_pQuadMesh->SetRenderState(m_pLightingTech_HasShadow->GetRenderState());
        res = rc.Render(m_pLightingTech_HasShadow->GetProgram().get(), m_pQuadMesh);
        m_pQuadMesh->SetRenderState(nullptr);
        if (res != DVF_Success)
        {
            LOG_ERROR("DeferredShadingRenderer::LightingJob() Render() failed.");
            return RRV_NextJob;
        }
    }

    // Step4: Render no-shadow light
    if (light_num_no_shadow > 0)
    {
        for (uint32_t i = 0; i < light_num_no_shadow; i++)
        {
            LightComponent* pLight = light_no_shadow_list[i];
            this->FillLightInfoByLightIndex(s_LightInfos[i + light_num_has_shadow], cam, cached_light_index[pLight]);
        }
        deferred_info.light_index_start = light_num_has_shadow;
        deferred_info.light_num = (int)light_num_no_shadow;
        m_pParamDeferredInfo->UpdateConstantBuffer(&deferred_info, sizeof(deferred_info));
        m_pLightInfoBuffer->Update(&light_light_data);

        m_pQuadMesh->SetRenderState(m_pLightingTech_NoShadow->GetRenderState());
        res = rc.Render(m_pLightingTech_NoShadow->GetProgram().get(), m_pQuadMesh);
        m_pQuadMesh->SetRenderState(nullptr);
        if (res != DVF_Success)
        {
            LOG_ERROR("DeferredShadingRenderer::LightingJob() Render() failed.");
            return RRV_NextJob;
        }
    }
    return RRV_NextJob;    
}
RendererReturnValue DeferredShadingRenderer::PrintTimeQueryJob()
{
    //if (m_pTimeQueryGenShadowMap)
    //    LOG_INFO("Time: Gen Shadow Map %.2lf", m_pTimeQueryGenShadowMap->TimeElapsedInMS());

    //if (m_pTimeQueryGenGBuffer)
    //    LOG_INFO("Time: Gen GBuffer %.2lf", m_pTimeQueryGenGBuffer->TimeElapsedInMS());

    //if (m_pTimeQuerySSAO)
    //    LOG_INFO("Time: SSAO %.2lf", m_pTimeQuerySSAO->TimeElapsedInMS());

    //if (use_tile_culling && m_pTimeQueryLihgtCulling)
    //    LOG_INFO("Time: Lighting with Culling %.2lf", m_pTimeQueryLihgtCulling->TimeElapsedInMS());

    //if (!use_tile_culling && m_pTimeQueryLighting)
    //    LOG_INFO("Time: Lighting %.2lf", m_pTimeQueryLighting->TimeElapsedInMS());

    //if (m_pTimeQueryGI)
    //    LOG_INFO("Time: GI %.2lf", m_pTimeQueryGI->TimeElapsedInMS());

    //if (m_pTimeQuerySkybox)
    //    LOG_INFO("Time: SkyBox %.2lf", m_pTimeQuerySkybox->TimeElapsedInMS());

    //if (m_pTimeQueryHDR)
    //    LOG_INFO("Time: HDR %.2lf", m_pTimeQueryHDR->TimeElapsedInMS());

    //if (m_pTimeQueryLDR)
    //    LOG_INFO("Time: LDR %.2lf", m_pTimeQueryLDR->TimeElapsedInMS());

    return RRV_NextJob;
}

SEEK_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
