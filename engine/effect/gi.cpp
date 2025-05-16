#include "effect/gi.h"
#include "kernel/context.h"
#include "components/light_component.h"
#include "components/camera_component.h"
#include "effect/shadow_layer.h"
#include "effect/effect.h"
#include "effect/postprocess.h"
#include "effect/parameter.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_render_buffer.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 189     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#include "shader/shared/GI.h"
#include "shader/shared/common.h"

static const uint32_t RSM_MIPMAP_LEVELS = 5;

/******************************************************************************
 * GlobalIllumination
 ******************************************************************************/
GlobalIllumination::GlobalIllumination(Context* context)
    :m_pContext(context)
{
}
SResult GlobalIllumination::Init()
{
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

SResult RSM::Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    m_eMode = GlobalIlluminationMode::RSM;

    SEEK_RETIF_FAIL(GlobalIllumination::Init());

    // RSM
    RHITexture::Desc desc;
    desc.type = TextureType::Tex2D;
    desc.width = desc.height = ShadowLayer::SM_SIZE;
    desc.depth = 1;
    desc.num_mips = RSM_MIPMAP_LEVELS;
    desc.num_samples = 1;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_COPY_BACK;
    m_pRSMTexs[0] = rc.CreateTexture2D(desc);
    m_pRSMTexs[1] = rc.CreateTexture2D(desc);
    m_pRSMTexs[2] = rc.CreateTexture2D(desc);
    desc.format = PixelFormat::D32F;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET;
    m_pRSMDepthTex = rc.CreateTexture2D(desc);
    
    m_pRSMFb = rc.CreateEmptyRHIFrameBuffer();
    m_pRSMFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pRSMTexs[0]));
    m_pRSMFb->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pRSMTexs[1]));
    m_pRSMFb->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pRSMTexs[2]));
    m_pRSMFb->AttachDepthStencilView(rc.CreateDepthStencilView(m_pRSMDepthTex));

    // RSM Effect
    uint32_t w = 1280;
	uint32_t h = 720;
    desc.type = TextureType::Tex2D;
    desc.width = w;
    desc.height = h;
    desc.depth = 1;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_COPY_BACK;
    m_pIndirecIlluminationTex = rc.CreateTexture2D(desc);

    m_pIndirecIlluminationFb = rc.CreateEmptyRHIFrameBuffer();
    m_pIndirecIlluminationFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pIndirecIlluminationTex));

    Effect& effect = m_pContext->EffectInstance();

    static const std::string gi_name = "GiRsm";    
    effect.LoadTechnique(gi_name, &RenderStateDesc::Default2D(), "GiRsmVS", "GiRsmPS", nullptr);

    m_pGiRsmPp = MakeSharedPtr<PostProcess>(m_pContext, gi_name);
    m_pGiRsmPp->Init(gi_name, NULL_PREDEFINES);
    m_pGiRsmPp->SetClear(false);

    m_pGiRsmPp->SetParam("gbuffer0", gbuffer0);
    m_pGiRsmPp->SetParam("gbuffer1", gbuffer1);
    m_pGiRsmPp->SetParam("gbuffer_depth", gbuffer_depth);

    m_pGiRsmPp->SetParam("rsm_color0", m_pRSMTexs[0]);
    m_pGiRsmPp->SetParam("rsm_color1", m_pRSMTexs[1]);
    m_pGiRsmPp->SetParam("rsm_color2", m_pRSMTexs[2]);

    m_pRsmCameraInfoCBuffer = rc.CreateConstantBuffer(sizeof(CameraInfo), RESOURCE_FLAG_CPU_WRITE);
    m_pGiRsmPp->SetParam("cb_CameraInfo", m_pRsmCameraInfoCBuffer);

    m_pRsmParamCBuffer = rc.CreateConstantBuffer(sizeof(GiRsmParam), RESOURCE_FLAG_CPU_WRITE);
    m_pGiRsmPp->SetParam("cb_GiRsmParam", m_pRsmParamCBuffer);

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
    return S_Success;
}
SResult RSM::OnBegin()
{
    //if (m_pIndirecIlluminationFb)
    //    m_pIndirecIlluminationFb->Clear(RHIFrameBuffer::CBM_Color, float4(0, 0, 0, 1));
	//m_pIndirecIlluminationFb->SetColorLoadOption(RHIFrameBuffer::Color0, float4(0, 0, 0, 1));
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
		m_pRSMFb->SetColorLoadOption(RHIFrameBuffer::Color0, float4(0, 0, 0, 0));
		m_pRSMFb->SetDepthLoadOption(1.0f);
        SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "GenerateReflectiveShadowMap", m_pRSMFb.get() });
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
        m_pRSMTexs[0]->DumpToFile("d:\\rsm0.rgba");
        m_pRSMTexs[1]->DumpToFile("d:\\rsm1.rgba");
        m_pRSMTexs[2]->DumpToFile("d:\\rsm2.rgba");
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
        //rc.BindFrameBuffer(m_pIndirecIlluminationFb);
		m_pIndirecIlluminationFb->SetColorLoadOption(RHIFrameBuffer::Color0, float4(0, 0, 0, 0));
		m_pIndirecIlluminationFb->SetDepthLoadOption(1.0f);

        Matrix4 const& inv_proj = pCam->GetInvProjMatrix();
        Matrix4 const& inv_view = pCam->GetInvViewMatrix();
        Matrix4 const& light_vp = pLightCam->GetViewProjMatrix();
        GiRsmParam param;
        param.inv_proj_matrix = inv_proj.Transpose();
		param.inv_view_matrix = inv_view.Transpose();
		param.light_view_proj_matrix = light_vp.Transpose();
		param.radius_rsmsize = float2(m_fSampleRadius, ShadowLayer::SM_SIZE);
		m_pRsmParamCBuffer->Update(&param, sizeof(param));

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
        m_pIndirecIlluminationTex->DumpToFile(path);
        draw--;
    }
#endif
    return RRV_NextJob;
}
/******************************************************************************
 * LPV : Light Propagation Volumes
 ******************************************************************************/
LPV::LPV(Context* context)
    :RSM(context)
{
    RHIContext& rc = context->RHIContextInstance();
    //m_eMode = GlobalIlluminationMode::LPV;
}


/******************************************************************************
 * VXGI : Voxel Cone Tracing
 ******************************************************************************/
VXGI::VXGI(Context* context)
    :GlobalIllumination(context)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    //m_eMode = GlobalIlluminationMode::VXGI;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!

