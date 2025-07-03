#include "effect/gi.h"
#include "kernel/context.h"
#include "components/light_component.h"
#include "components/camera_component.h"
#include "effect/effect.h"
#include "effect/postprocess.h"
#include "effect/parameter.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_render_buffer.h"
#include "rhi/base/rhi_render_state.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 189     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/GI.h"
#include "shader/shared/common.h"

static const uint32_t RSM_MIPMAP_LEVELS = 5;
static const uint32_t RSM_SIZE = 512;
static const uint32_t LPV_SIZE = 32;
static const uint32_t VOLUME_SIZE = 256;

/******************************************************************************
 * GlobalIllumination
 ******************************************************************************/
GlobalIllumination::GlobalIllumination(Context* context)
    :m_pContext(context)
{
}
SResult GlobalIllumination::Init()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHITexture::Desc desc = m_pContext->RHIContextInstance().GetScreenRHIFrameBuffer()->GetRenderTargetDesc(RHIFrameBuffer::Attachment::Color0);
    uint32_t w = desc.width;
    uint32_t h = desc.height;
    desc.type = TextureType::Tex2D;
    desc.width = w;
    desc.height = h;
    desc.depth = 1;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.format = PixelFormat::R16G16B16A16_FLOAT;
    desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
    m_pIndirectIlluminationTex = rc.CreateTexture2D(desc);

    m_pIndirectIlluminationFb = rc.CreateRHIFrameBuffer();
    m_pIndirectIlluminationFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pIndirectIlluminationTex));
    return S_Success;
}
/******************************************************************************
 * RSM : Reflective Shadow Map
 ******************************************************************************/
RSM::RSM(Context* context)
    :GlobalIllumination(context)
{
    m_eMode = GlobalIlluminationMode::RSM;
}
SResult RSM::OnBegin()
{
    m_pIndirectIlluminationFb->Clear(RHIFrameBuffer::CBM_Color, float4(0.0, 0.0, 0.0, 1.0));
    return S_Success;
}
SResult RSM::InitGenRsm()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHITexture::Desc desc;
    desc.type = TextureType::Tex2D;
    desc.width = desc.height = RSM_SIZE;
    desc.depth = 1;
    desc.num_mips = RSM_MIPMAP_LEVELS;
    desc.num_samples = 1;
    desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pRsmTexs[0] = rc.CreateTexture2D(desc);       // normal
    desc.format = PixelFormat::R16G16B16A16_FLOAT;
    m_pRsmTexs[1] = rc.CreateTexture2D(desc);       // position
    desc.format = PixelFormat::R16G16B16A16_FLOAT;
    m_pRsmTexs[2] = rc.CreateTexture2D(desc);       // Flux
    desc.format = PixelFormat::D16;
    desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
    m_pRsmDepthTex = rc.CreateTexture2D(desc);

    m_pGenRsmFb = rc.CreateRHIFrameBuffer();
    m_pGenRsmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pRsmTexs[0]));
    m_pGenRsmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pRsmTexs[1]));
    m_pGenRsmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pRsmTexs[2]));
    m_pGenRsmFb->AttachDepthStencilView(rc.CreateDepthStencilView(m_pRsmDepthTex));

    return S_Success;
}
SResult RSM::Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    SEEK_RETIF_FAIL(GlobalIllumination::Init());
    SEEK_RETIF_FAIL(this->InitGenRsm());
        
    Effect& effect = m_pContext->EffectInstance();
    static const std::string tech_name = "GiRsm";
    effect.LoadTechnique(tech_name, &RenderStateDesc::PostProcess(), "GiVS", "GiRsmPS", nullptr);
    m_pGiRsmPp = MakeSharedPtr<PostProcess>(m_pContext, tech_name);
    m_pGiRsmPp->Init(tech_name, NULL_PREDEFINES);
    m_pGiRsmPp->SetClear(false);

    m_pGiRsmPp->SetParam("gbuffer0", gbuffer0);
    m_pGiRsmPp->SetParam("gbuffer1", gbuffer1);
    m_pGiRsmPp->SetParam("gbuffer_depth", gbuffer_depth);

    m_pGiRsmPp->SetParam("rsm_color0", m_pRsmTexs[0]);
    m_pGiRsmPp->SetParam("rsm_color1", m_pRsmTexs[1]);
    m_pGiRsmPp->SetParam("rsm_color2", m_pRsmTexs[2]);

    m_pRsmCameraInfoCBuffer = rc.CreateConstantBuffer(sizeof(CameraInfo), RESOURCE_FLAG_CPU_WRITE);
    m_pGiRsmPp->SetParam("cb_CameraInfo", m_pRsmCameraInfoCBuffer);

    m_pRsmParamCBuffer = rc.CreateConstantBuffer(sizeof(GiRsmParam), RESOURCE_FLAG_CPU_WRITE);
    m_pRsmInvProjCBuffer = rc.CreateConstantBuffer(sizeof(float4x4), RESOURCE_FLAG_CPU_WRITE);
    m_pGiRsmPp->SetParam("cb_InvProjMatrix", m_pRsmInvProjCBuffer);
    m_pGiRsmPp->SetParam("cb_GiRsmPSParam", m_pRsmParamCBuffer);

	m_pVplCoordAndWeightsCBuffer = rc.CreateConstantBuffer(sizeof(float4) * VPL_NUM, RESOURCE_FLAG_CPU_WRITE);
    std::vector<float4> vplCoordAndWeights;
    for (int i = 0; i < VPL_NUM; ++i)
    {
        float2 f = Math::GenerateRandom(float2(0.0), float2(1.0));
        vplCoordAndWeights.push_back(float4(
            f.x() * sin(2 * Math::PI * f.y()),
            f.x() * cos(2 * Math::PI * f.y()),
            f.x() * f.x(), 0)
        );
    }
	m_pVplCoordAndWeightsCBuffer->Update(vplCoordAndWeights.data(), sizeof(float4) * VPL_NUM);
    m_pGiRsmPp->SetParam("cb_VplCoordAndWeights", m_pVplCoordAndWeightsCBuffer);

    m_pGiRsmPp->SetOutput(0, m_pIndirectIlluminationTex);
    m_pGiRsmPp->SetPostProcessRenderStateDesc(RenderStateDesc::PostProcessAccumulate());
    return S_Success;
}
RendererReturnValue RSM::GenerateReflectiveShadowMapJob(uint32_t light_index)
{
    SceneManager& sm = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();

    LightComponent* pLight = sm.GetLightComponentByIndex(light_index);
    LightType light_type = pLight->GetLightType();
    if (light_type == LightType::Spot || light_type == LightType::Directional)
    {
        sr.SetCurRenderStage(RenderStage::GenerateReflectiveShadowMap);
		m_pGenRsmFb->SetColorLoadOption(RHIFrameBuffer::Color0, float4(0, 0, 0, 1));
		m_pGenRsmFb->SetDepthLoadOption(1.0f);
        SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "GenerateReflectiveShadowMap", m_pGenRsmFb.get() });
        sm.SetActiveCamera(pLight->GetShadowMapCamera());
        sm.SetActiveLightIndex(light_index);
        res = sr.RenderScene((uint32_t)RenderScope::Opacity);
        if (res != S_Success)
        {
            LOG_ERROR_PRIERR(res, "RSM::GenerateReflectiveShadowMapJob failed.");
        }
        sm.SetActiveLightIndex(-1);
        sm.SetActiveCamera(nullptr);
        m_pContext->RHIContextInstance().EndRenderPass();
        sr.SetCurRenderStage(RenderStage::None);
    }
#if 1
    static int draw = 1;
    if (draw)
    {
        m_pRsmTexs[0]->DumpToFile("d:\\rsm0.rgba");
        m_pRsmTexs[1]->DumpToFile("d:\\rsm1.rgba");
        m_pRsmTexs[2]->DumpToFile("d:\\rsm2.rgba");
        m_pRsmDepthTex->DumpToFile("d:\\rsm_depth.g16l");
        draw--;
    }
#endif
    return RRV_NextJob;
}
RendererReturnValue RSM::PostProcessReflectiveShadowMapJob(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();
    CameraComponent* pCam = sc.GetActiveCamera();    
    if (!pCam)
    {
        LOG_ERROR("RSM::PostProcessReflectiveShadowMapJob Invalid Camera");
        return RRV_NextJob;
    }

    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);
    CameraComponent* pLightCam = pLight->GetShadowMapCamera();
    LightType light_type = pLight->GetLightType();
    if (light_type == LightType::Spot || light_type == LightType::Directional)
    {
		m_pIndirectIlluminationFb->SetColorLoadOption(RHIFrameBuffer::Color0, RHIFrameBuffer::LoadAction::Load);

        Matrix4 const& inv_proj = pCam->GetInvProjMatrix();
        Matrix4 const& inv_view = pCam->GetInvViewMatrix();
        Matrix4 const& light_vp = pLightCam->GetViewProjMatrix();
        GiRsmParam param;
		param.inv_view_matrix = inv_view.Transpose();
		param.light_view_proj_matrix = light_vp.Transpose();
		param.radius_rsmsize = float2(m_fSampleRadius, RSM_SIZE);
		m_pRsmParamCBuffer->Update(&param, sizeof(param));

        float4x4 inv_proj_matrix = inv_proj.Transpose();
        m_pRsmInvProjCBuffer->Update(&inv_proj_matrix, sizeof(float4x4));

        CameraInfo cameraInfo;
        if (pCam)
        {
            cameraInfo.posWorld = pCam->GetWorldTransform().GetTranslation();
			cameraInfo.nearPlane = pCam->GetNearPlane();
			cameraInfo.farPlane = pCam->GetFarPlane();
        }
		m_pRsmCameraInfoCBuffer->Update(&cameraInfo, sizeof(cameraInfo));

        m_pGiRsmPp->Run();
    }

#if 0
    static int draw = 1;
    if (draw)
    {
        static std::string path = "d:\\indirect_illumination.rgba";
        m_pIndirectIlluminationTex->DumpToFile(path);
        draw--;
    }
#endif
    return RRV_NextJob;
}
/******************************************************************************
 * LPV : Light Propagation Volumes
 ******************************************************************************/
const std::string LPV::TechName_LPVInject = "LPVInject";
const std::string LPV::TechName_LPVPropagation = "LPVPropagation";
const std::string LPV::TechName_LPVPropagation_Bak = "LPVPropagation_BAK";
LPV::LPV(Context* context)
    :RSM(context)
{
    m_eMode = GlobalIlluminationMode::LPV;
}
SResult LPV::OnBegin()
{
    RSM::OnBegin();
    return S_Success;
}
SResult LPV::Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    SEEK_RETIF_FAIL(GlobalIllumination::Init());
    SEEK_RETIF_FAIL(this->InitGenRsm());

    Effect& effect = m_pContext->EffectInstance();
    // for Inject
    effect.LoadTechnique(TechName_LPVInject, &RenderStateDesc::DepthDisable(), "LPVInjectVS", "LPVInjectPS", nullptr);
    Technique* pInjectTech = effect.GetTechnique(TechName_LPVInject);
    if (!pInjectTech)
    {
        LOG_ERROR("Load technique %s failed", TechName_LPVInject.c_str());
        return ERR_INVALID_INIT;
    }
    pInjectTech->SetParam("rsm_color0", m_pRsmTexs[0]);
    pInjectTech->SetParam("rsm_color1", m_pRsmTexs[1]);
    pInjectTech->SetParam("rsm_color2", m_pRsmTexs[2]);

    RHITexture::Desc desc;
    desc.type = TextureType::Tex3D;
    desc.width = desc.height = LPV_SIZE;
    desc.depth = LPV_SIZE;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pTexRedSh = rc.CreateTexture3D(desc);
    m_pTexGreenSh = rc.CreateTexture3D(desc);
    m_pTexBlueSh = rc.CreateTexture3D(desc);

    m_pFbInject = rc.CreateRHIFrameBuffer();
    m_pFbInject->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pTexRedSh));
    m_pFbInject->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pTexGreenSh));
    m_pFbInject->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pTexBlueSh));
    m_pFbInject->SetViewport({ 0, 0, LPV_SIZE, LPV_SIZE });

    // RenderState
    {
        m_PropagationRsDesc.depthStencil.bDepthEnable = false;
        m_PropagationRsDesc.blend.bIndependentBlendEnable = true;
        for (uint32_t i = 0; i < 6; ++i)
        {
            m_PropagationRsDesc.blend.stTargetBlend[i].bBlendEnable = (i < 3) ? false : true;
            m_PropagationRsDesc.blend.stTargetBlend[i].eSrcBlendColor = BlendFactor::One;
            m_PropagationRsDesc.blend.stTargetBlend[i].eDstBlendColor = BlendFactor::One;
            m_PropagationRsDesc.blend.stTargetBlend[i].eBlendOpColor = BlendOperation::Add;

            m_PropagationRsDesc.blend.stTargetBlend[i].eSrcBlendAlpha = BlendFactor::One;
            m_PropagationRsDesc.blend.stTargetBlend[i].eDstBlendAlpha = BlendFactor::One;
            m_PropagationRsDesc.blend.stTargetBlend[i].eBlendOpAlpha = BlendOperation::Add;
            m_PropagationRsDesc.blend.stTargetBlend[i].bColorWriteMask = CWM_RGBA;
        }
    }

    // for Propagation_Bak
    float4 vertices[3] = {
        float4(-1.0,  1.0, 0.0, 1.0),
        float4(3.0,  1.0, 0.0, 1.0),
        float4(-1.0, -3.0, 0.0, 1.0)
    };
    m_pInjectVerticsCBuffer = rc.CreateConstantBuffer(sizeof(float4) * 3, RESOURCE_FLAG_CPU_WRITE);
    m_pInjectVerticsCBuffer->Update(vertices, sizeof(float4) * 3);

    m_pTexRedSh_Bak = rc.CreateTexture3D(desc);
    m_pTexGreenSh_Bak = rc.CreateTexture3D(desc);
    m_pTexBlueSh_Bak = rc.CreateTexture3D(desc);
    m_pTexAccuRedSh = rc.CreateTexture3D(desc);
    m_pTexAccuGreenSh = rc.CreateTexture3D(desc);
    m_pTexAccuBlueSh = rc.CreateTexture3D(desc);

    m_pFbPropagation_Bak = rc.CreateRHIFrameBuffer();
    m_pFbPropagation_Bak->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pTexRedSh_Bak));
    m_pFbPropagation_Bak->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pTexGreenSh_Bak));
    m_pFbPropagation_Bak->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pTexBlueSh_Bak));
    m_pFbPropagation_Bak->AttachTargetView(RHIFrameBuffer::Attachment::Color3, rc.CreateRenderTargetView(m_pTexAccuRedSh));
    m_pFbPropagation_Bak->AttachTargetView(RHIFrameBuffer::Attachment::Color4, rc.CreateRenderTargetView(m_pTexAccuGreenSh));
    m_pFbPropagation_Bak->AttachTargetView(RHIFrameBuffer::Attachment::Color5, rc.CreateRenderTargetView(m_pTexAccuBlueSh));
    m_pFbPropagation_Bak->SetViewport({ 0, 0, LPV_SIZE, LPV_SIZE });
    effect.LoadTechnique(TechName_LPVPropagation_Bak, &m_PropagationRsDesc, "LPVPropagationVS", "LPVPropagationPS", nullptr);
    Technique* pPropagationTech_Bak = effect.GetTechnique(TechName_LPVPropagation_Bak);
    if (!pPropagationTech_Bak)
    {
        LOG_ERROR("Load technique %s failed", TechName_LPVPropagation_Bak.c_str());
        return ERR_INVALID_INIT;
    }
    pPropagationTech_Bak->SetParam("redSH", m_pTexRedSh);
    pPropagationTech_Bak->SetParam("greenSH", m_pTexGreenSh);
    pPropagationTech_Bak->SetParam("blueSH", m_pTexBlueSh);
    pPropagationTech_Bak->SetParam("cb_InjectVertics", m_pInjectVerticsCBuffer);

    // for Propagation
    m_pFbPropagation = rc.CreateRHIFrameBuffer();
    m_pFbPropagation->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pTexRedSh));
    m_pFbPropagation->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pTexGreenSh));
    m_pFbPropagation->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pTexBlueSh));
    m_pFbPropagation->AttachTargetView(RHIFrameBuffer::Attachment::Color3, rc.CreateRenderTargetView(m_pTexAccuRedSh));
    m_pFbPropagation->AttachTargetView(RHIFrameBuffer::Attachment::Color4, rc.CreateRenderTargetView(m_pTexAccuGreenSh));
    m_pFbPropagation->AttachTargetView(RHIFrameBuffer::Attachment::Color5, rc.CreateRenderTargetView(m_pTexAccuBlueSh));
    m_pFbPropagation->SetViewport({ 0, 0, LPV_SIZE, LPV_SIZE });

    effect.LoadTechnique(TechName_LPVPropagation, &m_PropagationRsDesc, "LPVPropagationVS", "LPVPropagationPS", nullptr);
    Technique* pPropagationTech = effect.GetTechnique(TechName_LPVPropagation);
    if (!pPropagationTech)
    {
        LOG_ERROR("Load technique %s failed", TechName_LPVPropagation.c_str());
        return ERR_INVALID_INIT;
    }
    pPropagationTech->SetParam("redSH", m_pTexRedSh_Bak);
    pPropagationTech->SetParam("greenSH", m_pTexGreenSh_Bak);
    pPropagationTech->SetParam("blueSH", m_pTexBlueSh_Bak);
    pPropagationTech->SetParam("cb_InjectVertics", m_pInjectVerticsCBuffer);

    // for Calc LPV's Indirect 
    static const std::string gilpv_tech_name = "GiLpv";
    effect.LoadTechnique(gilpv_tech_name, &RenderStateDesc::PostProcess(), "GiVS", "GiLpvPS", nullptr);
    m_pGiLpvPp = MakeSharedPtr<PostProcess>(m_pContext, gilpv_tech_name);
    m_pGiLpvPp->Init(gilpv_tech_name, NULL_PREDEFINES);
    m_pGiLpvPp->SetClear(false);

    m_pLpvInvProjCBuffer = rc.CreateConstantBuffer(sizeof(float4x4), RESOURCE_FLAG_CPU_WRITE);
    m_pLpvCameraInfoCBuffer = rc.CreateConstantBuffer(sizeof(CameraInfo), RESOURCE_FLAG_CPU_WRITE);
    m_pLpvParamCBuffer = rc.CreateConstantBuffer(sizeof(GiLpvParam), RESOURCE_FLAG_CPU_WRITE);

    m_pGiLpvPp->SetParam("gbuffer0", gbuffer0);
    m_pGiLpvPp->SetParam("gbuffer1", gbuffer1);
    m_pGiLpvPp->SetParam("gbuffer_depth", gbuffer_depth);
    m_pGiLpvPp->SetParam("cb_InvProjMatrix", m_pLpvInvProjCBuffer);
    m_pGiLpvPp->SetParam("cb_CameraInfo", m_pLpvCameraInfoCBuffer);
    m_pGiLpvPp->SetParam("cb_GiLpvPSParam", m_pLpvParamCBuffer);
    m_pGiLpvPp->SetParam("redSH",   m_pTexRedSh);
    m_pGiLpvPp->SetParam("greenSH", m_pTexGreenSh);
    m_pGiLpvPp->SetParam("blueSH",  m_pTexBlueSh);
    m_pGiLpvPp->SetOutput(0, m_pIndirectIlluminationTex);
    m_pGiLpvPp->SetPostProcessRenderStateDesc(RenderStateDesc::PostProcessAccumulate());
    return S_Success;
}
SResult LPV::LPVInject()
{
    m_pFbInject->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, float4(0.0));
    m_pFbInject->SetColorLoadOption(RHIFrameBuffer::Attachment::Color1, float4(0.0));
    m_pFbInject->SetColorLoadOption(RHIFrameBuffer::Attachment::Color2, float4(0.0));
    SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "LPV::LPVInject", m_pFbInject.get() });
    if (res != S_Success)
    {
        LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::GenerateGBufferJob() BindFrameBuffer failed.");
    }
    Effect& effect = m_pContext->EffectInstance();
    Technique* pTech = effect.GetTechnique(TechName_LPVInject);
    pTech->DrawInstanced(MeshTopologyType::Points, RSM_SIZE * RSM_SIZE, 1, 0, 0);
    m_pContext->RHIContextInstance().EndRenderPass();

#if 0
    static int draw = 1;
    if (draw)
    {
        static std::string path = "d:\\sh_red.rgba";
        m_pTexRedSh->DumpToFile(path);
        draw--;
    }
#endif
    return S_Success;
}
SResult LPV::LPVPropagation()
{
    m_pFbPropagation->ClearRenderTarget(RHIFrameBuffer::Attachment::Color3, float4(0.0));
    m_pFbPropagation->ClearRenderTarget(RHIFrameBuffer::Attachment::Color4, float4(0.0));
    m_pFbPropagation->ClearRenderTarget(RHIFrameBuffer::Attachment::Color5, float4(0.0));
    
    RHIContext& rc = m_pContext->RHIContextInstance();
    Effect& effect = m_pContext->EffectInstance();
    Technique* pTech_Bak = effect.GetTechnique(TechName_LPVPropagation_Bak);
    Technique* pTech = effect.GetTechnique(TechName_LPVPropagation);
   
    for (uint32_t i = 0; i < m_iPropagationSteps; ++i)
    {
        rc.BeginRenderPass({ "LPVPropagation_Bak", m_pFbPropagation_Bak.get() });
        pTech_Bak->DrawInstanced(MeshTopologyType::Triangles, 3, LPV_DIM, 0, 0);
        rc.EndRenderPass();

        rc.BeginRenderPass({ "LPVPropagation", m_pFbPropagation.get() });
        pTech->DrawInstanced(MeshTopologyType::Triangles, 3, LPV_DIM, 0, 0);
        rc.EndRenderPass();
    }
   
    return S_Success;
}
SResult LPV::LPVCalcIndirect(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();
    CameraComponent* pCam = sc.GetActiveCamera();
    if (!pCam)
    {
        LOG_ERROR("RSM::PostProcessReflectiveShadowMapJob Invalid Camera");
        return RRV_NextJob;
    }

    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);
    CameraComponent* pLightCam = pLight->GetShadowMapCamera();
    LightType light_type = pLight->GetLightType();
    if (light_type == LightType::Spot || light_type == LightType::Directional)
    {
        m_pIndirectIlluminationFb->SetColorLoadOption(RHIFrameBuffer::Color0, RHIFrameBuffer::LoadAction::Load);

        Matrix4 const& inv_proj = pCam->GetInvProjMatrix();
        m_pLpvInvProjCBuffer->Update(&inv_proj, sizeof(Matrix4));

        Matrix4 const& inv_view = pCam->GetInvViewMatrix().Transpose();
        GiLpvParam param = { inv_view, m_fLpvAttenuation, m_fLpvPower, m_fLpvCutoff };
        m_pLpvParamCBuffer->Update(&param, sizeof(GiLpvParam));

        CameraInfo cameraInfo = {  };
        if (pCam)
        {
            cameraInfo.posWorld = pCam->GetWorldTransform().GetTranslation();
            cameraInfo.nearPlane = pCam->GetNearPlane();
            cameraInfo.farPlane = pCam->GetFarPlane();
        }
        m_pLpvCameraInfoCBuffer->Update(&cameraInfo, sizeof(CameraInfo));

        m_pGiLpvPp->Run();
    }

#if 0
    static int draw = 1;
    if (draw)
    {
        static std::string path = "d:\\indirect_illumination.rgba";
        m_pIndirectIlluminationTex->DumpToFile(path);
        draw--;
    }
#endif
    return S_Success;
}
RendererReturnValue LPV::PostProcessReflectiveShadowMapJob(uint32_t light_index)
{
    this->LPVInject();
    this->LPVPropagation();
    this->LPVCalcIndirect(light_index);
    return RRV_NextJob;
}

/******************************************************************************
 * VXGI : Voxel Cone Tracing 
 ******************************************************************************/
VXGI::VXGI(Context* context)
    :GlobalIllumination(context)
{
    m_eMode = GlobalIlluminationMode::VXGI;
}

SResult VXGI::OnBegin()
{
    SEEK_RETIF_FAIL(GlobalIllumination::OnBegin());
    return S_Success;
}
SResult VXGI::Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHITexture::Desc desc;
    desc.type = TextureType::Tex3D;
    desc.width = desc.height = desc.depth = VOLUME_SIZE;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pTexVoxel3D = rc.CreateTexture3D(desc);
    m_pTexVoxel3D_Copy = rc.CreateTexture3D(desc);
    return S_Success;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!

