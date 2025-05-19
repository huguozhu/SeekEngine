#include "effect/deferred_shading_renderer.h"
#include "effect/technique.h"
#include "effect/postprocess.h"
#include "effect/effect.h"
#include "effect/hdr_postprocess.h"
#include "effect/ldr_postprocess.h"
#include "effect/shadow_layer.h"
#include "effect/blur.h"
#include "effect/gi.h"
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


#define SEEK_MACRO_FILE_UID 87     // this code is auto generated, don't touch it!!!

#define SSAO_KERNEL_COUNT 64

SEEK_NAMESPACE_BEGIN

#include "shader/shared/Ssao.h"

static bool use_tile_culling = 0;
static const std::string szTechName_PreZ = "PreZ";
static const std::string szTechName_GenerateShadowMap = "GenerateShadowMap";
static const std::string szTechName_GenerateCubeShadowMap = "GenerateCubeShadowMap";
static const std::string szTechName_GenerateGBuffer = "GenerateGBuffer";

static const std::string szTechName_DeferredLighting = "DeferredLighting";
static const std::string szTechName_SSAO = "SSAO";
static const std::string szTechName_LightCulling = "LightCullingCS";
static const std::string szTechName_GenerateReflectiveShadowMap = "GenerateReflectiveShadowMap";
static const std::string szTechName_GenerateCascadedShadowMap = "GenerateCascadedShadowMap";

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
	SEEK_RETIF_FAIL(SceneRenderer::Init());

    // Step1: RHIMesh & RHIRenderBuffers
	RHITexture::Desc target_desc = rc.GetScreenRHIFrameBuffer()->GetRenderTargetDesc(RHIFrameBuffer::Attachment::Color0);
    uint32_t w = m_iRenderWidth = target_desc.width;
    uint32_t h = m_iRenderHeight = target_desc.height;
    m_pQuadMesh = QuadMesh_GetMesh(rc);
    m_pCameraInfoCBuffer = rc.CreateConstantBuffer(sizeof(CameraInfo), RESOURCE_FLAG_CPU_WRITE);

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
    m_pSsaoSampleKernelCBuffer  = rc.CreateConstantBuffer(sizeof(ssao_kernels[0]) * ssao_kernels.size(), RESOURCE_FLAG_CPU_WRITE);
    m_pSsaoParamCBuffer         = rc.CreateConstantBuffer(sizeof(SsaoParam), RESOURCE_FLAG_CPU_WRITE);
    m_pLightInfoCBuffer         = rc.CreateByteAddressBuffer(sizeof(LightInfo) * MAX_DEFERRED_LIGHTS_NUM, RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_CPU_WRITE, nullptr);
    m_pLightCullingInfoCBuffer  = rc.CreateByteAddressBuffer(sizeof(LightCullingInfo) * MAX_DEFERRED_LIGHTS_NUM, RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_CPU_WRITE, nullptr);
    m_pDeferredLightingInfoCBuffer = rc.CreateConstantBuffer(sizeof(DeferredLightingInfo), RESOURCE_FLAG_CPU_WRITE);

    if (use_tile_culling)
    {
        uint32_t tile_width = (w + TILE_SIZE - 1) / TILE_SIZE;
        uint32_t tile_height = (h + TILE_SIZE - 1) / TILE_SIZE;
        m_pTileInfoBuffer = rc.CreateRWByteAddressBuffer(sizeof(TileInfo) * tile_width * tile_height, RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_SHADER_WRITE | RESOURCE_FLAG_COPY_BACK, nullptr);
    }

    // Step2: Textures
    RHITexture::Desc desc;
    desc.type = TextureType::Tex2D;
    desc.width = w;
    desc.height = h;
    desc.depth = 1;
    desc.num_mips = 1;
    desc.num_samples = m_pContext->GetNumSamples();
    desc.format = PixelFormat::D16;
    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
    m_pSceneDepthStencil = rc.CreateTexture2D(desc);

    if (m_pContext->GetAntiAliasingMode() == AntiAliasingMode::TAA)
    {
        desc.format = PixelFormat::R16G16_SNORM;
        m_pSceneVelocity = rc.CreateTexture2D(desc);
    }

    desc.num_samples = 1;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pLDRColor = rc.CreateTexture2D(desc);

    PixelFormat pf = PixelFormat::R16G16B16A16_FLOAT;
    if (!rc.GetCapabilitySet().IsTextureSupport(pf, TextureFormatSupportType::RenderTarget))
        pf = PixelFormat::R32G32B32A32_FLOAT;
    desc.num_samples = m_pContext->GetNumSamples();
    desc.format = pf;
    m_pHDRColor = rc.CreateTexture2D(desc);
    m_pSceneColor = m_pHDRColor;

    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pPreZColor = rc.CreateTexture2D(desc);

    desc.num_samples = 1;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
    m_pGBufferColor0 = rc.CreateTexture2D(desc);
    m_pGBufferColor1 = rc.CreateTexture2D(desc);
    m_pGBufferColor2 = rc.CreateTexture2D(desc);

    desc.width = w / 2;
    desc.height = h / 2;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pShadowTex = rc.CreateTexture2D(desc);

    desc.format = PixelFormat::R8_UNORM;
    m_pSsaoColor = rc.CreateTexture2D(desc);


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
    desc.format = PixelFormat::R32G32F;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE;
    m_pSsaoNoise = rc.CreateTexture2D(desc, ssao_noise_bitmap);


	//  Step3: RHIFrameBuffers
    RHIRenderViewPtr ds_view = rc.CreateDepthStencilView(m_pSceneDepthStencil);
    m_pPreZFb = rc.CreateEmptyRHIFrameBuffer();
    m_pPreZFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pPreZColor));
    m_pPreZFb->AttachDepthStencilView(ds_view);

    m_pGBufferFb = rc.CreateEmptyRHIFrameBuffer();
    m_pGBufferFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pGBufferColor0));
    m_pGBufferFb->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pGBufferColor1));
    m_pGBufferFb->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pGBufferColor2));
    m_pGBufferFb->AttachDepthStencilView(ds_view);

    desc.width = w;
    desc.height = h;
    desc.format = PixelFormat::D16;
    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
    m_pLightingDepthStencil = rc.CreateTexture2D(desc);
    m_pLightingFb = rc.CreateEmptyRHIFrameBuffer();
    m_pLightingFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pHDRColor));
    m_pLightingFb->AttachDepthStencilView(rc.CreateDepthStencilView(m_pLightingDepthStencil));

    m_pLDRFb = rc.CreateEmptyRHIFrameBuffer();
    m_pHDRFb = rc.CreateEmptyRHIFrameBuffer();    
    m_pSceneFb = m_pHDRFb;

    m_pShadowingFb = rc.CreateEmptyRHIFrameBuffer();
    m_pShadowingFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pShadowTex));

    m_pSsaoFb = rc.CreateEmptyRHIFrameBuffer();
    m_pSsaoFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pSsaoColor));


    // Step4: Techniques
    Effect& effect = m_pContext->EffectInstance();
    effect.LoadTechnique(szTechName_PreZ,               &RenderStateDesc::Default3D(),  "PreZMeshRenderingVS",  "EmptyPS");
    effect.LoadTechnique(szTechName_GenerateGBuffer,    &RenderStateDesc::GBuffer(),    "MeshRenderingVS",      "GenerateGBufferPS");
    effect.LoadTechnique(szTechName_DeferredLighting,   &RenderStateDesc::Lighting(),   "DeferredLightingVS",   "DeferredLightingPS");
    effect.LoadTechnique(szTechName_SSAO,               &RenderStateDesc::PostProcess(),"SsaoVS",               "SsaoPS");
    
    if (use_tile_culling)
    {
        effect.LoadTechnique(szTechName_LightCulling, nullptr, nullptr, nullptr, "LightCullingCS");
        m_pTileCullingTech = effect.GetTechnique(szTechName_LightCulling);
        m_pTileCullingTech->SetParam("cb_CameraInfo", m_pCameraInfoCBuffer);
        m_pTileCullingTech->SetParam("cb_LightCullingCSParam", m_pLightCullingInfoCBuffer);
		m_pTileCullingTech->SetParam("depth", m_pSceneDepthStencil);
        m_pTileCullingTech->SetParam("light_infos", m_pLightInfoCBuffer);
        m_pTileCullingTech->SetParam("tile_infos_rw", m_pTileInfoBuffer);

        m_pLightingTech_HasShadow_TileCulling = effect.GetTechnique(szTechName_DeferredLighting, { {"HAS_SHADOW" , "1"}, {"TILE_CULLING", "1"} });
        m_pLightingTech_HasShadow_TileCulling->SetParam("cb_DeferredLightingVSInfo", m_pDeferredLightingInfoCBuffer);
        m_pLightingTech_HasShadow_TileCulling->SetParam("cb_DeferredLightingPSInfo", m_pDeferredLightingInfoCBuffer);
        m_pLightingTech_HasShadow_TileCulling->SetParam("cb_CameraInfo", m_pCameraInfoCBuffer);
        m_pLightingTech_HasShadow_TileCulling->SetParam("tile_infos", m_pTileInfoBuffer);
        m_pLightingTech_HasShadow_TileCulling->SetParam("light_infos", m_pLightInfoCBuffer);

        m_pLightingTech_HasShadow_TileCulling->SetParam("gbuffer0", m_pGBufferColor0);
        m_pLightingTech_HasShadow_TileCulling->SetParam("gbuffer1", m_pGBufferColor1);
        m_pLightingTech_HasShadow_TileCulling->SetParam("depth_tex", m_pSceneDepthStencil);
        m_pLightingTech_HasShadow_TileCulling->SetParam("shadowing_tex", m_pShadowTex);
    }
    else
    {
        m_pLightingTech_HasShadow = effect.GetTechnique(szTechName_DeferredLighting, { {"HAS_SHADOW" , "1"}, {"TILE_CULLING", "0"} });
        m_pLightingTech_HasShadow->SetParam("cb_DeferredLightingVSInfo", m_pDeferredLightingInfoCBuffer);
        m_pLightingTech_HasShadow->SetParam("cb_DeferredLightingPSInfo", m_pDeferredLightingInfoCBuffer);
        m_pLightingTech_HasShadow->SetParam("cb_CameraInfo", m_pCameraInfoCBuffer);
        m_pLightingTech_HasShadow->SetParam("light_infos", m_pLightInfoCBuffer);
        m_pLightingTech_HasShadow->SetParam("gbuffer0", m_pGBufferColor0);
        m_pLightingTech_HasShadow->SetParam("gbuffer1", m_pGBufferColor1);
        m_pLightingTech_HasShadow->SetParam("depth_tex", m_pSceneDepthStencil);
        m_pLightingTech_HasShadow->SetParam("shadowing_tex", m_pShadowTex);

        m_pLightingTech_NoShadow = effect.GetTechnique(szTechName_DeferredLighting, { {"HAS_SHADOW" , "0"}, {"TILE_CULLING", "0"} });
        m_pLightingTech_NoShadow->SetParam("cb_DeferredLightingVSInfo", m_pDeferredLightingInfoCBuffer);
        m_pLightingTech_NoShadow->SetParam("cb_DeferredLightingPSInfo", m_pDeferredLightingInfoCBuffer);
        m_pLightingTech_NoShadow->SetParam("cb_CameraInfo", m_pCameraInfoCBuffer);
        m_pLightingTech_NoShadow->SetParam("light_infos", m_pLightInfoCBuffer);
        m_pLightingTech_NoShadow->SetParam("gbuffer0", m_pGBufferColor0);
        m_pLightingTech_NoShadow->SetParam("gbuffer1", m_pGBufferColor1);
        m_pLightingTech_NoShadow->SetParam("depth_tex", m_pSceneDepthStencil);
    }
    
	
    m_pSsaoTech = effect.GetTechnique(szTechName_SSAO);
    m_pSsaoTech->SetParam("gbuffer0",               m_pGBufferColor0);
    m_pSsaoTech->SetParam("depth_tex",              m_pSceneDepthStencil);
    m_pSsaoTech->SetParam("ssao_noise",             m_pSsaoNoise);
    m_pSsaoTech->SetParam("cb_CameraInfo",          m_pCameraInfoCBuffer);    
    m_pSsaoTech->SetParam("cb_SsaoSampleKernels",   m_pSsaoSampleKernelCBuffer);    
    m_pSsaoTech->SetParam("cb_SsaoVSParam",         m_pSsaoParamCBuffer);
    m_pSsaoTech->SetParam("cb_SsaoPSParam",         m_pSsaoParamCBuffer);


    // Step5: PostProcess
    m_pLDRPostProcess = MakeSharedPtr<LDRPostProcess>(m_pContext);
    m_pLDRPostProcess->SetLDRTexture(m_pLDRColor);
    if (m_pContext->GetAntiAliasingMode() == AntiAliasingMode::TAA)
        m_pLDRPostProcess->SetTaaSceneVelocityTexture(m_pSceneVelocity);
    
    m_pHDRPostProcess = MakeSharedPtr<HDRPostProcess>(m_pContext);
    m_pHDRPostProcess->SetSrcTexture(m_pHDRColor);

    // Shadow
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

    /************* Blur *****************************/
    m_pGaussianBlur = MakeSharedPtr<GaussianBlur>(m_pContext);
    m_pGaussianBlur->SetSrcTexture(m_pSsaoColor);
    m_pGaussianBlur->SetDstTexture(m_pSsaoColor);

    GlobalIlluminationMode mode = m_pContext->GetGlobalIlluminationMode();
    if (mode == GlobalIlluminationMode::RSM)
    {
        m_pGI = MakeSharedPtrMacro(RSM, m_pContext);
        ((RSM*)m_pGI.get())->Init(m_pGBufferColor0, m_pGBufferColor1, m_pSceneDepthStencil);
    }

    if (m_pContext->EnableProfile())
    {
        m_pTimeQueryGenShadowMap    = rc.CreateRHITimeQuery();
        m_pTimeQueryGenGBuffer      = rc.CreateRHITimeQuery();
        m_pTimeQuerySSAO            = rc.CreateRHITimeQuery();
        m_pTimeQueryLihgtCulling    = rc.CreateRHITimeQuery();
        m_pTimeQueryLighting        = rc.CreateRHITimeQuery();
        m_pTimeQueryGI              = rc.CreateRHITimeQuery();
        m_pTimeQuerySkybox          = rc.CreateRHITimeQuery();
        m_pTimeQueryHDR             = rc.CreateRHITimeQuery();
        m_pTimeQueryLDR             = rc.CreateRHITimeQuery();
    }
    
    return ret;
}
SResult DeferredShadingRenderer::BuildRenderJobList()
{
    m_vRenderingJobs.clear();
    SceneManager& sm = m_pContext->SceneManagerInstance();
    size_t light_count = sm.NumLightComponent();
    if (light_count == 0)
        return S_Success;

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

    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::RenderPrepareJob, this)));
    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::RenderPreZJob, this)));

    // Shadow Map job
    BEGIN_TIMEQUERY(m_pTimeQueryGenShadowMap);
    if (m_pContext->EnableShadow())
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
    if (m_pContext->GetGlobalIlluminationMode() != GlobalIlluminationMode::None)
        for (uint32_t i = 0; i < light_count; i++)
            this->AppendGIJobs(i);
    END_TIMEQUERY(m_pTimeQueryGI);
    
    
    BEGIN_TIMEQUERY(m_pTimeQuerySkybox);
    if (sm.GetSkyBoxComponent())
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::RenderSkyBoxJob, this)));
    END_TIMEQUERY(m_pTimeQuerySkybox);
    
    BEGIN_TIMEQUERY(m_pTimeQueryHDR);
    //m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::HDRJob, this)));
    END_TIMEQUERY(m_pTimeQueryHDR);

    //BEGIN_TIMEQUERY(m_pTimeQueryLDR);
    //m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::LDRJob, this)));
    //END_TIMEQUERY(m_pTimeQueryLDR);

    //m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::CalculateRenderRectJob, this)));

    if (m_pContext->EnableProfile())
        m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&DeferredShadingRenderer::PrintTimeQueryJob, this))); ;

    m_vRenderingJobs.push_back(MakeUniquePtr<RenderingJob>(std::bind(&SceneRenderer::FinishJob, this)));

    return S_Success;
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
SResult DeferredShadingRenderer::GetEffectTechniqueToRender(RHIMeshPtr mesh, Technique** tech)
{
    MorphInfo& morph_info = mesh->GetMorphInfo();
    MorphTargetType morph_target_type = morph_info.morph_target_type;
    uint32_t        morph_count = (uint32_t)morph_info.morph_target_weights.size();
    
    Effect& effect = m_pContext->EffectInstance();

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
    case RenderStage::PreZ:                         *tech = effect.GetTechnique(szTechName_PreZ, predefines);                       break;
    case RenderStage::GenerateShadowMap:            *tech = effect.GetTechnique(szTechName_GenerateShadowMap,       predefines);    break;
    case RenderStage::GenerateCubeShadowMap:        *tech = effect.GetTechnique(szTechName_GenerateCubeShadowMap,   predefines);    break;
    case RenderStage::GenerateGBuffer:
    case RenderStage::GenerateReflectiveShadowMap:
    case RenderStage::GenerateCascadedShadowMap:
    {
        uint32_t has_tex_normal = mesh->GetMaterial()->normal_tex ? 1 : 0;
        EffectPredefine hasNormalTexPredefine;
        hasNormalTexPredefine.name = "HAS_MATERIAL_NORMAL";
        hasNormalTexPredefine.value = std::to_string(has_tex_normal);

        EffectPredefine enableTAAPredefine;
        enableTAAPredefine.name = "ENABLE_TAA";
        enableTAAPredefine.value = m_pContext->GetAntiAliasingMode() == AntiAliasingMode::TAA ? "1" : "0";

        predefines.push_back(hasNormalTexPredefine);
        predefines.push_back(enableTAAPredefine);

        if (m_eCurRenderStage == RenderStage::GenerateGBuffer)
            *tech = effect.GetTechnique(szTechName_GenerateGBuffer, predefines);
        else if (m_eCurRenderStage == RenderStage::GenerateReflectiveShadowMap)
            *tech = effect.GetTechnique(szTechName_GenerateReflectiveShadowMap, predefines);
        else
            *tech = effect.GetTechnique(szTechName_GenerateCascadedShadowMap, predefines);
        break;
    }
    case RenderStage::None:
    case RenderStage::RenderScene:
    default:
        LOG_ERROR("DeferredShadingRenderer::GetEffectTechniqueToRender invalid RenderStage");
    }
    if (*tech == nullptr)
    {
        LOG_ERROR("DeferredShadingRenderer::GetEffectTechniqueToRender Invalid technique!");
        return ERR_INVALID_SHADER;
    }

    return S_Success;
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
    case RenderStage::PreZ:
    case RenderStage::GenerateGBuffer:
        return true;
        break;
    case RenderStage::None:
    case RenderStage::GenerateShadowMap:
    case RenderStage::GenerateCubeShadowMap:
    case RenderStage::GenerateCascadedShadowMap:
    case RenderStage::RenderScene:
    default:
        return false;
        break;
    }
}

RHIMeshPtr DeferredShadingRenderer::GetLightVolumeMesh(LightType type)
{
    RHIMeshPtr pMesh = nullptr;
    RHIContext& rc = m_pContext->RHIContextInstance();
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
RendererReturnValue DeferredShadingRenderer::RenderPrepareJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_pLightingFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, { float4(0.0) });
    m_pLightingFb->SetDepthLoadOption({ 1.0f });
    m_pSsaoFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, { float4(0.0) });
    m_pSsaoFb->SetDepthLoadOption({ 1.0f });
    if (m_pGI)
        m_pGI->OnBegin();
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::RenderPreZJob()
{
    m_eCurRenderStage = RenderStage::PreZ;
    m_pPreZFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, { float4(0.0) });
    m_pPreZFb->SetDepthLoadOption({ 1.0f });
    SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "RenderPreZJob", m_pPreZFb.get() });
    if (res != S_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::RenderPreZJob() BindFrameBuffer failed.");
    }

    res = m_pContext->SceneRendererInstance().RenderScene((uint32_t)RenderScope::Opacity);
    if (res != S_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::RenderPreZJob() RenderScene failed.");
    }
    m_eCurRenderStage = RenderStage::None;
    m_pContext->RHIContextInstance().EndRenderPass();
#if 1
    static int draw = 0;
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
    m_pGBufferFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, { float4(0.0) });
    m_pGBufferFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color1, { float4(0.0) });
    m_pGBufferFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color2, { float4(0.0) });
    m_pGBufferFb->SetDepthLoadOption(RHIFrameBuffer::LoadAction::Load);
    SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "GenerateGBufferJob", m_pGBufferFb.get() });
    if (res != S_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::GenerateGBufferJob() BindFrameBuffer failed.");
    }

    res = m_pContext->SceneRendererInstance().RenderScene((uint32_t)RenderScope::Opacity);
    if (res != S_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::GenerateGBufferJob() RenderScene failed.");
    }
    m_eCurRenderStage = RenderStage::None;

#if 1
    static int draw = 1;
    if (draw)
    {
        m_pGBufferColor0->DumpToFile("d:\\GBuffer_RT0.rgba");
        m_pGBufferColor1->DumpToFile("d:\\GBuffer_RT1.rgba");
        m_pGBufferColor2->DumpToFile("d:\\GBuffer_RT2.rgba");
        m_pSceneDepthStencil->DumpToFile("d:\\GBuffer_DS.g16l");
        draw--;
    }
#endif
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::SSAOJob()
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();    
    SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "SSAOJob", m_pSsaoFb.get() });
    if (res != S_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::SSAOJob() BindFrameBuffer failed.");
        return RRV_NextJob;
    }
    CameraComponent* cam = sm.GetActiveCamera();
    CameraInfo cameraInfo = {  };
    if (cam)
    {
        cameraInfo.posWorld = cam->GetWorldTransform().GetTranslation();
        cameraInfo.farPlane = cam->GetFarPlane();
    }
    m_pCameraInfoCBuffer->Update(&cameraInfo, sizeof(cameraInfo));

    Matrix4 view_matrix = cam->GetViewMatrix().Transpose();
    Matrix4 proj_matrix = cam->GetProjMatrix().Transpose();
    Matrix4 inv_proj_matrix = cam->GetInvProjMatrix().Transpose();
    SsaoParam param = {  };
    param.view_matrix = view_matrix;
	param.proj_matrix = proj_matrix;
    param.inv_proj_matrix = inv_proj_matrix;
    param.ssao_scale = float2((float)m_iRenderWidth / 4.0, (float)m_iRenderHeight / 4.0);
	m_pSsaoParamCBuffer->Update(&param, sizeof(param));
	

    m_pQuadMesh->SetRenderState(m_pSsaoTech->GetRenderState());
    res = rc.Render(m_pSsaoTech->GetProgram(), m_pQuadMesh);
    if (res != S_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::SSAOJob() Render() failed.");
        return RRV_NextJob;
    }

#if 1
    static int draw = 1;
    if (draw)
    {
        m_pSsaoColor->DumpToFile("d:\\ssao.gray");
        draw--;
    }
#endif
    m_pContext->RHIContextInstance().EndRenderPass();
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::LightingTileCullingJob()
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();

    // Step1: Set Params
    CameraComponent* cam = sm.GetActiveCamera();
    CameraInfo cameraInfo;
    if (cam)
    {
        cameraInfo.posWorld = cam->GetWorldTransform().GetTranslation();
        cameraInfo.farPlane = cam->GetFarPlane();
    }
    m_pCameraInfoCBuffer->Update(&cameraInfo, sizeof(cameraInfo));

    LightCullingInfo culling_info;
	culling_info.view_matrix = cam->GetViewMatrix().Transpose();
	culling_info.proj_matrix = cam->GetProjMatrix().Transpose();
	culling_info.frame_size = float2((float)m_iRenderWidth, (float)m_iRenderHeight);
    culling_info.light_index_start = 0;
    culling_info.light_num = (int)sm.NumLightComponent();
	m_pLightCullingInfoCBuffer->Update(&culling_info, sizeof(culling_info));

    for (uint32_t i = 0; i < sm.NumLightComponent(); i++)
    {
        LightComponent* pLight = sm.GetLightComponentByIndex(i);
        this->FillLightInfoByLightIndex(s_LightInfos[i], cam, i);
    }
    m_pLightInfoCBuffer->Update(&s_LightInfos[0], sizeof(LightInfo) * MAX_DEFERRED_LIGHTS_NUM);

    
    /**************    Step1: Light culling    ****************************/
    uint32_t x = (m_pGBufferColor0->Width()  + TILE_SIZE - 1) / TILE_SIZE;
    uint32_t y = (m_pGBufferColor0->Height() + TILE_SIZE - 1) / TILE_SIZE;
    SResult res = rc.Dispatch(m_pTileCullingTech->GetProgram(), x, y, 1);
    if (res != S_Success)
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

    /**************    Step2: Rendering after light culling    ****************************/
    res = m_pContext->RHIContextInstance().BeginRenderPass({ "Lighting_LightCulling", m_pLightingFb.get() });
    if (res != S_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::LightingTileCullingJob() BindFrameBuffer failed.");
        return RRV_NextJob;
    }

    m_pQuadMesh->SetRenderState(m_pLightingTech_HasShadow_TileCulling->GetRenderState());
    res = rc.Render(m_pLightingTech_HasShadow_TileCulling->GetProgram(), m_pQuadMesh);
    if (res != S_Success)
    {
        LOG_ERROR("DeferredShadingRenderer::LightingTileCullingJob() Render() failed.");
        return RRV_NextJob;
    }
    m_pContext->RHIContextInstance().EndRenderPass();
    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::LightingJob()
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
        
    // Step1: Set Params
    CameraComponent* cam = sm.GetActiveCamera();
    CameraInfo cameraInfo;
    if (cam)
    {
        cameraInfo.posWorld = cam->GetWorldTransform().GetTranslation();
        cameraInfo.nearPlane = cam->GetNearPlane();
        cameraInfo.farPlane = cam->GetFarPlane();
    }
    m_pCameraInfoCBuffer->Update(&cameraInfo, sizeof(cameraInfo));


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
    DeferredLightingInfo deferred_info;
    deferred_info.lightVolumeMV = cam ? cam->GetInvProjMatrix().Transpose() : Matrix4::Identity();
    deferred_info.lightVolumeInvView = cam ? cam->GetInvViewMatrix().Transpose() : Matrix4::Identity();
    
    SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "Lighting", m_pLightingFb.get()});
    if (light_num_has_shadow > 0)
    {
        for (uint32_t i = 0; i < light_num_has_shadow; i++)
        {
            LightComponent* pLight = light_has_shadow_list[i];
            this->FillLightInfoByLightIndex(s_LightInfos[i], cam, cached_light_index[pLight]);
        }
        m_pLightInfoCBuffer->Update(&s_LightInfos[0], light_num_has_shadow * sizeof(LightInfo));

        deferred_info.light_index_start = 0;
        deferred_info.light_num = (int)light_num_has_shadow;
        m_pDeferredLightingInfoCBuffer->Update(&deferred_info, sizeof(deferred_info));
        

        m_pQuadMesh->SetRenderState(m_pLightingTech_HasShadow->GetRenderState());
        res = rc.Render(m_pLightingTech_HasShadow->GetProgram(), m_pQuadMesh);
        m_pQuadMesh->SetRenderState(nullptr);
        if (res != S_Success)
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
        m_pLightInfoCBuffer->Update(&s_LightInfos[0], MAX_DEFERRED_LIGHTS_NUM * sizeof(LightInfo));

        deferred_info.light_index_start = light_num_has_shadow;
        deferred_info.light_num = (int)light_num_no_shadow;
        m_pDeferredLightingInfoCBuffer->Update(&deferred_info, sizeof(deferred_info));

        m_pQuadMesh->SetRenderState(m_pLightingTech_NoShadow->GetRenderState());
        res = rc.Render(m_pLightingTech_NoShadow->GetProgram(), m_pQuadMesh);
        m_pQuadMesh->SetRenderState(nullptr);
        if (res != S_Success)
        {
            LOG_ERROR("DeferredShadingRenderer::LightingJob() Render() failed.");
            return RRV_NextJob;
        }
    }
    m_pContext->RHIContextInstance().EndRenderPass();

#if 1
    static int draw = 1;
    if (draw)
    {
        m_pLightingDepthStencil->DumpToFile("d:\\light_depth.g16l");
        draw--;
    }
#endif

    return RRV_NextJob;    
}
RendererReturnValue DeferredShadingRenderer::RenderSkyBoxJob()
{
    m_eCurRenderStage = RenderStage::None;
    m_pLightingFb->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, RHIFrameBuffer::LoadAction::Load);
    m_pLightingFb->SetDepthLoadOption(RHIFrameBuffer::LoadAction::Load);
    m_pContext->RHIContextInstance().BeginRenderPass({ "RenderSkybox", m_pLightingFb.get() });
    SkyBoxComponent* skybox = m_pContext->SceneManagerInstance().GetSkyBoxComponent();
    if (skybox)
        skybox->Render();
    m_pContext->RHIContextInstance().EndRenderPass();

    return RRV_NextJob;
}
RendererReturnValue DeferredShadingRenderer::PrintTimeQueryJob()
{
    if (m_pTimeQueryGenShadowMap)
        LOG_INFO("Time: Gen Shadow Map %.2lf", m_pTimeQueryGenShadowMap->TimeElapsedInMS());

    if (m_pTimeQueryGenGBuffer)
        LOG_INFO("Time: Gen GBuffer %.2lf", m_pTimeQueryGenGBuffer->TimeElapsedInMS());

    if (m_pTimeQuerySSAO)
        LOG_INFO("Time: SSAO %.2lf", m_pTimeQuerySSAO->TimeElapsedInMS());

    if (use_tile_culling && m_pTimeQueryLihgtCulling)
        LOG_INFO("Time: Lighting with Culling %.2lf", m_pTimeQueryLihgtCulling->TimeElapsedInMS());

    if (!use_tile_culling && m_pTimeQueryLighting)
        LOG_INFO("Time: Lighting %.2lf", m_pTimeQueryLighting->TimeElapsedInMS());

    if (m_pTimeQueryGI)
        LOG_INFO("Time: GI %.2lf", m_pTimeQueryGI->TimeElapsedInMS());

    if (m_pTimeQuerySkybox)
        LOG_INFO("Time: SkyBox %.2lf", m_pTimeQuerySkybox->TimeElapsedInMS());

    if (m_pTimeQueryHDR)
        LOG_INFO("Time: HDR %.2lf", m_pTimeQueryHDR->TimeElapsedInMS());

    if (m_pTimeQueryLDR)
        LOG_INFO("Time: LDR %.2lf", m_pTimeQueryLDR->TimeElapsedInMS());

    return RRV_NextJob;
}

SEEK_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
