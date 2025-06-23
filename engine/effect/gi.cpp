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
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_COPY_BACK;
    m_pIndirectIlluminationTex = rc.CreateTexture2D(desc);

    m_pIndirectIlluminationFb = rc.CreateEmptyRHIFrameBuffer();
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
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_COPY_BACK;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pRsmTexs[0] = rc.CreateTexture2D(desc);       // normal
    desc.format = PixelFormat::R16G16B16A16_FLOAT;
    m_pRsmTexs[1] = rc.CreateTexture2D(desc);       // position
    desc.format = PixelFormat::R16G16B16A16_FLOAT;
    m_pRsmTexs[2] = rc.CreateTexture2D(desc);       // Flux
    desc.format = PixelFormat::D16;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_COPY_BACK;;
    m_pRsmDepthTex = rc.CreateTexture2D(desc);

    m_pGenRsmFb = rc.CreateEmptyRHIFrameBuffer();
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
    effect.LoadTechnique(tech_name, &RenderStateDesc::Default2D(), "GiRsmVS", "GiRsmPS", nullptr);
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
    m_pGiRsmPp->SetParam("cb_GiRsmVSParam", m_pRsmParamCBuffer);
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
    static int draw = 0;
    if (draw)
    {
        m_pRsmTexs[0]->DumpToFile("d:\\rsm0.rgba");
        //m_pRsmTexs[1]->DumpToFile("d:\\rsm1.rgba");
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
        param.inv_proj_matrix = inv_proj.Transpose();
		param.inv_view_matrix = inv_view.Transpose();
		param.light_view_proj_matrix = light_vp.Transpose();
		param.radius_rsmsize = float2(m_fSampleRadius, RSM_SIZE);
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
        m_pIndirectIlluminationTex->DumpToFile(path);
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
    //m_eMode = GlobalIlluminationMode::LPV;
}
SResult LPV::OnBegin()
{
    RSM::OnBegin();
    m_pFbSH->Clear();
    return S_Success;
}
SResult LPV::Init(RHITexturePtr const& gbuffer0, RHITexturePtr const& gbuffer1, RHITexturePtr const& gbuffer_depth)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    SEEK_RETIF_FAIL(GlobalIllumination::Init());
    SEEK_RETIF_FAIL(this->InitGenRsm());

    RHITexture::Desc desc;
    desc.type = TextureType::Tex3D;
    desc.width = desc.height = LPV_SIZE;
    desc.depth = LPV_SIZE;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_COPY_BACK;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pTexRedSh = rc.CreateTexture3D(desc);
    m_pTexGreenSh = rc.CreateTexture3D(desc);
    m_pTexBlueSh = rc.CreateTexture3D(desc);

    m_pFbSH = rc.CreateEmptyRHIFrameBuffer();
    m_pFbSH->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pTexRedSh));
    m_pFbSH->AttachTargetView(RHIFrameBuffer::Attachment::Color1, rc.CreateRenderTargetView(m_pTexGreenSh));
    m_pFbSH->AttachTargetView(RHIFrameBuffer::Attachment::Color2, rc.CreateRenderTargetView(m_pTexBlueSh));
    return S_Success;
}
SResult LPV::LPVInject()
{
    return S_Success;
}
SResult LPV::LPVPropagation()
{
    return S_Success;
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
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_RENDER_TARGET;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    m_pTexVoxel3D = rc.CreateTexture3D(desc);
    m_pTexVoxel3D = rc.CreateTexture3D(desc);


    return S_Success;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!

