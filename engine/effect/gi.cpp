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
#include "math/matrix.h"
#define SEEK_MACRO_FILE_UID 189     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/GI.h"
#include "shader/shared/common.h"

static const uint32_t RSM_MIPMAP_LEVELS = 5;
static const uint32_t RSM_SIZE = 512;
static const uint32_t VXGI_VOLUME_SIZE = 256;

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
    m_pIndirectIlluminationFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.Create2DRenderTargetView(m_pIndirectIlluminationTex));
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
    m_pGenRsmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.Create2DRenderTargetView(m_pRsmTexs[0]));
    m_pGenRsmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.Create2DRenderTargetView(m_pRsmTexs[1]));
    m_pGenRsmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.Create2DRenderTargetView(m_pRsmTexs[2]));
    m_pGenRsmFb->AttachDepthStencilView(rc.Create2DDepthStencilView(m_pRsmDepthTex));

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
#if 0
    static int draw = 1;
    if (draw)
    {
        m_pRsmTexs[0]->DumpToFile("d:\\dump\\rsm0.rgba");
        m_pRsmTexs[1]->DumpToFile("d:\\dump\\rsm1.rgba");
        m_pRsmTexs[2]->DumpToFile("d:\\dump\\rsm2.rgba");
        m_pRsmDepthTex->DumpToFile("d:\\dump\\rsm_depth.g16l");
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
        static std::string path = "d:\\dump\\indirect_illumination.rgba";
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
LPV::LPV(Context* context)
    :RSM(context)
{
    m_eMode = GlobalIlluminationMode::LPV;
}
LPV::~LPV()
{
    m_pFbInject.clear();
    m_pFbPropagation.clear();
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
    m_pRTIndexCBuffer = rc.CreateConstantBuffer(sizeof(uint32_t), RESOURCE_FLAG_CPU_WRITE);
    pInjectTech->SetParam("cb_RTIndex", m_pRTIndexCBuffer);

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

    m_pFbInject.resize(LPV_SIZE);
    for (uint32_t i = 0; i < LPV_SIZE; i++)
    {
        m_pFbInject[i] = rc.CreateRHIFrameBuffer();
        m_pFbInject[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.Create3DRenderTargetView(m_pTexRedSh,   0, i, 1, 0));
        m_pFbInject[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.Create3DRenderTargetView(m_pTexGreenSh, 0, i, 1, 0));
        m_pFbInject[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.Create3DRenderTargetView(m_pTexBlueSh,  0, i, 1, 0));
        m_pFbInject[i]->SetViewport({ 0, 0, LPV_SIZE, LPV_SIZE });
    }

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

    m_pTexRedSh_Src = rc.CreateTexture3D(desc);
    m_pTexGreenSh_Src = rc.CreateTexture3D(desc);
    m_pTexBlueSh_Src = rc.CreateTexture3D(desc);
    m_pTexRedSh_RT = rc.CreateTexture3D(desc);
    m_pTexGreenSh_RT = rc.CreateTexture3D(desc);
    m_pTexBlueSh_RT = rc.CreateTexture3D(desc);
    m_pTexAccuRedSh = rc.CreateTexture3D(desc);
    m_pTexAccuGreenSh = rc.CreateTexture3D(desc);
    m_pTexAccuBlueSh = rc.CreateTexture3D(desc);

    // for Propagation
    m_pFbPropagation.resize(LPV_SIZE);
    for (uint32_t i = 0; i < LPV_SIZE; i++)
    {
        m_pFbPropagation[i] = rc.CreateRHIFrameBuffer();
        m_pFbPropagation[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.Create3DRenderTargetView(m_pTexRedSh_RT,      0, i, 1, 0));
        m_pFbPropagation[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.Create3DRenderTargetView(m_pTexGreenSh_RT,    0, i, 1, 0));
        m_pFbPropagation[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.Create3DRenderTargetView(m_pTexBlueSh_RT,     0, i, 1, 0));
        m_pFbPropagation[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color3, rc.Create3DRenderTargetView(m_pTexAccuRedSh,      0, i, 1, 0));
        m_pFbPropagation[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color4, rc.Create3DRenderTargetView(m_pTexAccuGreenSh,    0, i, 1, 0));
        m_pFbPropagation[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color5, rc.Create3DRenderTargetView(m_pTexAccuBlueSh,     0, i, 1, 0));
        m_pFbPropagation[i]->SetViewport({ 0, 0, LPV_SIZE, LPV_SIZE });
    }

    effect.LoadTechnique(TechName_LPVPropagation, &m_PropagationRsDesc, "LPVPropagationVS", "LPVPropagationPS", nullptr);
    Technique* pPropagationTech = effect.GetTechnique(TechName_LPVPropagation);
    if (!pPropagationTech)
    {
        LOG_ERROR("Load technique %s failed", TechName_LPVPropagation.c_str());
        return ERR_INVALID_INIT;
    }
    pPropagationTech->SetParam("redSH", m_pTexRedSh_Src);
    pPropagationTech->SetParam("greenSH", m_pTexGreenSh_Src);
    pPropagationTech->SetParam("blueSH", m_pTexBlueSh_Src);
    pPropagationTech->SetParam("cb_InjectVertics", m_pInjectVerticsCBuffer);
    pPropagationTech->SetParam("cb_RTIndex", m_pRTIndexCBuffer);

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
    m_pGiLpvPp->SetParam("redSH",   m_pTexAccuRedSh);
    m_pGiLpvPp->SetParam("greenSH", m_pTexAccuGreenSh);
    m_pGiLpvPp->SetParam("blueSH",  m_pTexAccuBlueSh);
    m_pGiLpvPp->SetOutput(0, m_pIndirectIlluminationTex);
    m_pGiLpvPp->SetPostProcessRenderStateDesc(RenderStateDesc::PostProcessAccumulate());
    return S_Success;
}
SResult LPV::LPVInject()
{
    Effect& effect = m_pContext->EffectInstance();
    Technique* pTech = effect.GetTechnique(TechName_LPVInject);
    for (uint32_t i = 0; i < LPV_SIZE; i++)
    {
        if (i == 0)
        {
            m_pFbInject[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, float4(0.0));
            m_pFbInject[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color1, float4(0.0));
            m_pFbInject[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color2, float4(0.0));
        }
        else
        {
            m_pFbInject[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, RHIFrameBuffer::LoadAction::Load);
            m_pFbInject[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color1, RHIFrameBuffer::LoadAction::Load);
            m_pFbInject[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color2, RHIFrameBuffer::LoadAction::Load);
        }

        SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "LPV::LPVInject", m_pFbInject[i].get() });
        if (res != S_Success)
        {
            LOG_ERROR_PRIERR(res, "DeferredShadingRenderer::GenerateGBufferJob() BindFrameBuffer failed.");
        }
        m_pRTIndexCBuffer->Update(&i, sizeof(uint32_t));
        pTech->DrawInstanced(MeshTopologyType::Points, RSM_SIZE * RSM_SIZE, 1, 0, 0);
        m_pContext->RHIContextInstance().EndRenderPass();
    }

#if 0
    static int draw = 1;
    if (draw)
    {
        BitmapBufferPtr bitmap_data[LPV_SIZE] = {};
        for (uint32_t i = 0; i < LPV_SIZE; i++)
        {
            std::string path = "d:\\dump\\sh_red\\sh_red_" + std::to_string(i) + ".rgba";
            bitmap_data[i] = MakeSharedPtr<BitmapBuffer>();
            Box<uint32_t> box = { 0, 0, i, m_pTexRedSh->Width(), m_pTexRedSh->Height(), 1 };
            m_pTexRedSh->DumpSubResource3D(bitmap_data[i], 0, 0, &box);
            m_pTexRedSh->DumpToFile(path, bitmap_data[i]);
        }
        draw--;
    }
#endif
    return S_Success;
}
SResult LPV::LPVPropagation()
{
    for (uint32_t i = 0; i < LPV_SIZE; i++)
    {
        m_pFbPropagation[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color0, RHIFrameBuffer::LoadAction::DontCare);
        m_pFbPropagation[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color1, RHIFrameBuffer::LoadAction::DontCare);
        m_pFbPropagation[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color2, RHIFrameBuffer::LoadAction::DontCare);
        m_pFbPropagation[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color3, RHIFrameBuffer::LoadAction::Load);
        m_pFbPropagation[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color4, RHIFrameBuffer::LoadAction::Load);
        m_pFbPropagation[i]->SetColorLoadOption(RHIFrameBuffer::Attachment::Color5, RHIFrameBuffer::LoadAction::Load);
    }
    
    RHIContext& rc = m_pContext->RHIContextInstance();
    Effect& effect = m_pContext->EffectInstance();
    Technique* pTech = effect.GetTechnique(TechName_LPVPropagation);

    m_pFbPropagation[0]->Clear();
    for (uint32_t j = 0; j < LPV_SIZE; j++)
    {        
        rc.CopyTexture(m_pTexRedSh, m_pTexRedSh_Src);
        rc.CopyTexture(m_pTexGreenSh, m_pTexGreenSh_Src);
        rc.CopyTexture(m_pTexBlueSh, m_pTexBlueSh_Src);

        m_pRTIndexCBuffer->Update(&j, sizeof(uint32_t));
        rc.BeginRenderPass({ "LPVPropagation", m_pFbPropagation[j].get() });
        for (uint32_t i = 0; i < m_iPropagationSteps; ++i)
        {
            pTech->DrawInstanced(MeshTopologyType::Triangles, 3, 1, 0, 0);
            rc.CopyTexture(m_pTexRedSh_RT, m_pTexRedSh_Src);
            rc.CopyTexture(m_pTexGreenSh_RT, m_pTexGreenSh_Src);
            rc.CopyTexture(m_pTexBlueSh_RT, m_pTexBlueSh_Src);
        }
        rc.EndRenderPass();
    }

#if 0
    static int draw = 1;
    if (draw)
    {
        BitmapBufferPtr bitmap_data[LPV_SIZE] = {};
        for (uint32_t i = 0; i < LPV_SIZE; i++)
        {
            std::string path = "d:\\dump\\accu_sh_red\\accu_sh_red_step" + std::to_string(i) + ".rgba";
            bitmap_data[i] = MakeSharedPtr<BitmapBuffer>();
            Box<uint32_t> box = { 0, 0, i, m_pTexAccuRedSh->Width(), m_pTexAccuRedSh->Height(), 1 };
            m_pTexAccuRedSh->DumpSubResource3D(bitmap_data[i], 0, 0, &box);
            bitmap_data[i]->DumpToFile(path);
        }
        draw--;
    }
#endif
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
        static std::string path = "d:\\dump\\indirect_illumination.rgba";
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
    desc.width = desc.height = desc.depth = VXGI_VOLUME_SIZE;
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

