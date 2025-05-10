#include "effect/shadow_layer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "effect/postprocess.h"
#include "effect/forward_shading_renderer.h"
//#include "effect/hdr_postprocess.h"

#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_mesh.h"

#include "math/matrix.h"
#include "math/transform.h"
#include "math/quad_mesh_process.h"

#include "components/light_component.h"
#include "components/camera_component.h"
#include "components/light_component.h"

#include "utils/buffer.h"
#include "utils/error.h"

SEEK_NAMESPACE_BEGIN


static int32_t const NO_SHADOW_INDEX = -1;
/******************************************************************************
 * ShadowLayer
 ******************************************************************************/
ShadowLayer::ShadowLayer(Context* context)
    :m_pContext(context)
{

}
SResult ShadowLayer::InitResource()
{
    // Shadow Map(SM)
    PixelFormat sm_pf = PixelFormat::R8G8B8A8_UNORM;
    PixelFormat depth_pf = PixelFormat::D16;

    RHIContext& rc = m_pContext->RHIContextInstance();
    RHITexture::Desc desc;
    desc.width = SM_SIZE;
    desc.height = SM_SIZE;
    desc.depth = 1;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.type = TextureType::Tex2D;
    desc.format = sm_pf;
    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
    m_pSmTex = rc.CreateTexture2D(desc);
    desc.format = depth_pf;
    m_pSmDepthTex = rc.CreateTexture2D(desc);
    m_pSmFb = rc.CreateEmptyRHIFrameBuffer();
    RHIRenderViewPtr sm_color_view = rc.CreateRenderTargetView(m_pSmTex);
    m_pSmFb->AttachTargetView(RHIFrameBuffer::Attachment::Color0, sm_color_view);
    m_pSmFb->AttachDepthStencilView(rc.CreateDepthStencilView(m_pSmDepthTex));

    PixelFormat filter_sm_fmt = PixelFormat::R8G8B8A8_UNORM;
    CapabilitySet const& cap_set = m_pContext->RHIContextInstance().GetCapabilitySet();
    if (cap_set.IsTextureSupport(PixelFormat::R16G16B16A16_UNORM, TextureFormatSupportType::RenderTarget))
        filter_sm_fmt = PixelFormat::R16G16B16A16_UNORM;
    desc.format = filter_sm_fmt;
    m_pFilteredSmTex = rc.CreateTexture2D(desc);

    // Cascaded Shadow Map(CSM)
    m_vCsmTex.resize(NUM_CSM_LEVELS);
    m_vCsmDepthTex.resize(NUM_CSM_LEVELS);
    m_vCsmFb.resize(NUM_CSM_LEVELS);    
    desc.format = PixelFormat::R32F;
    for (uint32_t i = 0; i < NUM_CSM_LEVELS; ++i)
    {
        desc.format = depth_pf;
        m_vCsmDepthTex[i] = rc.CreateTexture2D(desc);

        desc.format = PixelFormat::R32F;
        m_vCsmTex[i] = rc.CreateTexture2D(desc);

        m_vCsmFb[i] = rc.CreateEmptyRHIFrameBuffer();
        m_vCsmFb[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_vCsmTex[i]));
        m_vCsmFb[i]->AttachDepthStencilView(rc.CreateDepthStencilView(m_vCsmDepthTex[i]));
    }

    // Cube Shadow Map
    desc.format = PixelFormat::R32F;
    desc.type = TextureType::Cube;
    m_pCubeSmTex = rc.CreateTextureCube(desc);
    desc.format = PixelFormat::D16;
    desc.type = TextureType::Cube;
    m_pCubeSmDepthTex = rc.CreateTextureCube(desc);
    for (uint32_t i = (uint32_t)CubeFaceType::Positive_X; i < (uint32_t)CubeFaceType::Num; i++)
    {
        RHIRenderViewPtr cube_rtv = rc.CreateRenderTargetView(m_pCubeSmTex, (CubeFaceType)i);
        RHIRenderViewPtr cube_dsv = rc.CreateDepthStencilView(m_pCubeSmDepthTex, (CubeFaceType)i);
        m_pCubeSmFb[i] = rc.CreateEmptyRHIFrameBuffer();
        m_pCubeSmFb[i]->AttachTargetView(RHIFrameBuffer::Attachment::Color0, cube_rtv);
        m_pCubeSmFb[i]->AttachDepthStencilView(cube_dsv);
    }        
    return S_Success;
}
SResult ShadowLayer::AnalyzeLightShadow()
{
    uint32_t shadow_index = 0;
    uint32_t filtered_2d_map_index = 0;
    uint32_t cube_map_index = 0;
    m_vShadowIndex.clear();

    SceneManager& sm = m_pContext->SceneManagerInstance();
    size_t light_count = sm.NumLightComponent();
    for (uint32_t i = 0; i < light_count; i++)
    {
        LightComponent* pLight = sm.GetLightComponentByIndex(i);
        if (!pLight->IsEnable()                     || 
            !pLight->CastShadow()                   || 
            shadow_index >= MAX_SHADOW_LIGHT_NUM    )
        {
            m_vShadowIndex.push_back(std::make_pair(NO_SHADOW_INDEX, NO_SHADOW_INDEX));
            continue;
        }

        LightType type = pLight->GetLightType();
        if (type == LightType::Spot)
        {
            m_vShadowIndex.push_back(std::make_pair(shadow_index, filtered_2d_map_index));
            shadow_index++;
            filtered_2d_map_index++;
        }
        else if (type == LightType::Directional)
        {
            m_vShadowIndex.push_back(std::make_pair(shadow_index, filtered_2d_map_index));
            shadow_index++;
            filtered_2d_map_index++;
        }
        else if (type == LightType::Point)
        {
            if (cube_map_index >= MAX_CUBE_LIGHT_NUM)
            {
                m_vShadowIndex.push_back(std::make_pair(NO_SHADOW_INDEX, NO_SHADOW_INDEX));
                continue;
            }
            m_vShadowIndex.push_back(std::make_pair(shadow_index, cube_map_index));
            shadow_index++;
            cube_map_index++;
        }
    }
    return S_Success;
}
RendererReturnValue ShadowLayer::GenerateShadowMapJob(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();    

    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);
    LightType light_type = pLight->GetLightType();
    if (light_type == LightType::Spot || light_type == LightType::Directional)
    {
        sr.SetCurRenderStage(RenderStage::GenerateShadowMap);
        //rc.BindRHIFrameBuffer(m_pSmFb);
        //m_pSmFb->Clear();

        m_pSmFb->SetColorLoadOption(RHIFrameBuffer::Color0, float4(0.0f, 0.0f, 0.0f, 0.0f));
        m_pSmFb->SetDepthLoadOption(1.0f);
        m_pContext->RHIContextInstance().BeginRenderPass({ "GenerateShadowMap", m_pSmFb.get() });
        sc.SetActiveCamera(pLight->GetShadowMapCamera());
        SResult res = sr.RenderScene((uint32_t)RenderScope::Opacity);
        m_pContext->RHIContextInstance().EndRenderPass();
        if (res != S_Success)
        {
            LOG_ERROR_PRIERR(res, "GenerateShadowMapJob() failed.");
        }
        sc.SetActiveCamera(nullptr);
        sr.SetCurRenderStage(RenderStage::None);
    }
    else if (light_type == LightType::Point)
    {
        sr.SetCurRenderStage(RenderStage::GenerateCubeShadowMap);
        pLight->UpdateShadowMapCamera();
        uint32_t cube_shadow_map_index = m_vShadowIndex[light_index].second;
        for (uint32_t i = (uint32_t)CubeFaceType::Positive_X; i < (uint32_t)CubeFaceType::Num; i++)
        {
            // rc.BindFrameBuffer(m_pCubeSmFb[i]);            
            if (0 == cube_shadow_map_index)
            {
                //res = m_pCubeSmFb[i]->Clear(FrameBuffer::CBM_ALL, float4(1.0f), 1.0f);
                m_pCubeSmFb[i]->SetColorLoadOption(RHIFrameBuffer::Color0, float4(1.0f));
                m_pCubeSmFb[i]->SetDepthLoadOption(1.0f);
            }
            else
            {
                m_pCubeSmFb[i]->SetColorLoadOption(RHIFrameBuffer::Color0, {});
                m_pCubeSmFb[i]->SetDepthLoadOption({});
            }

            SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "GenerateCubeShadowMap", m_pCubeSmFb[i].get() });
            sc.SetActiveCamera(pLight->GetShadowMapCamera(i));
            res = sr.RenderScene((uint32_t)RenderScope::Opacity);
            if (res != S_Success)
            {
                LOG_ERROR_PRIERR(res, "GenerateShadowMapJob() Point Light failed.");
            }
            sc.SetActiveCamera(nullptr);
            m_pContext->RHIContextInstance().EndRenderPass();
        }
        sr.SetCurRenderStage(RenderStage::None);
    }

    return RRV_NextJob;
}
RendererReturnValue ShadowLayer::GenerateCascadedShadowMapJob(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
    SceneRenderer& sr = m_pContext->SceneRendererInstance();

    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);
    LightType light_type = pLight->GetLightType();
    if (light_type == LightType::Directional)
    {
        sr.SetCurRenderStage(RenderStage::GenerateCascadedShadowMap);
        for (uint32_t i = 0; i < NUM_CSM_LEVELS; ++i)
        {
            //rc.BindFrameBuffer(m_vCsmFb[i]);
            //m_vCsmFb[i]->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, float4(pLight->GetShadowMapCamera()->GetNearFarPlane().y(), 0.0, 0.0, 0.0), 1.0);
            m_vCsmFb[i]->SetColorLoadOption(RHIFrameBuffer::Color0, float4(pLight->GetShadowMapCamera()->GetFarPlane(), 0.0, 0.0, 0.0));
            m_vCsmFb[i]->SetDepthLoadOption(1.0f);
            SResult res = m_pContext->RHIContextInstance().BeginRenderPass({ "GenerateCascadedShadowMap", m_vCsmFb[i].get() });

            sc.SetActiveCamera(((DirectionalLightComponent*)pLight)->GetCSMCamera()->GetCameraByCSMIndex(i));
            res = sr.RenderScene((uint32_t)RenderScope::Opacity);
            if (res != S_Success)
            {
                LOG_ERROR_PRIERR(res, "ShadowLayer::GenerateCascadedShadowMapJob() failed.");
            }
            m_pContext->RHIContextInstance().EndRenderPass();
        }
        sc.SetActiveCamera(nullptr);
        sr.SetCurRenderStage(RenderStage::None);
    }
#if 0
    static int draw = 1;
    if (draw)
    {
        for (uint32_t i = 0; i < NUM_CSM_LEVELS; ++i)
        {
            m_vCsmTex[i]->DumpToFile("d:\\csm_" + std::to_string(i) + ".rgba");
            m_vCsmDepthTex[i]->DumpToFile("d:\\csm_depth_" + std::to_string(i) + ".g16l");
        }
        draw--;
    }
#endif
    return RRV_NextJob;
}
int32_t ShadowLayer::GetShadowMapIndexByLightIndex(size_t light_index)
{
    return m_vShadowIndex[light_index].first;
}
Technique* ShadowLayer::GetShadowTechnique(uint32_t morph_count)
{
 //   Effect& effect = m_pContext->EffectInstance();
 //   Technique* pTech = nullptr;

	//std::string tech_name = "shadow_morph_count" + std::to_string(morph_count);
 //   //effect.LoadTechnique(tech_name, )

 //   if (m_ShadowEffects.find(morph_count) != m_ShadowEffects.end())
 //       pEffect = m_ShadowEffects[morph_count].get();
 //   else
 //   {
 //       static std::string morhp_size_macro_name = "MORPH_SIZE";
 //       std::string morph_size_value = std::to_string(morph_count);
 //       std::vector<std::pair<std::string, std::string>> v;
 //       v.push_back(std::make_pair(morhp_size_macro_name, morph_size_value));

 //       // FIXME: BAD! we load same effect files many times
 //       EffectPtr effect_new = MakeSharedPtr<Effect>(m_pContext);
 //       SResult ret = effect_new->Load("shadow.effect", &v);
 //       if (ret != S_Success)
 //       {
 //           LOG_ERROR("ShadowLayer::GetShadowEffect() Load failed.");
 //           return nullptr;
 //       }
 //       m_ShadowEffects[morph_count] = effect_new;
 //       pEffect = effect_new.get();
 //   }
 //   return pEffect;
    return nullptr;
}


/******************************************************************************
 * ForwardShadowLayer
 ******************************************************************************/
ForwardShadowLayer::ForwardShadowLayer(Context* context)
    :ShadowLayer(context)
{

}
SResult ForwardShadowLayer::InitResource()
{
    ShadowLayer::InitResource();
	Effect& effect = m_pContext->EffectInstance();
    // Shadow Map PostProcess
    for (uint32_t i = 0; i < MAX_SHADOW_LIGHT_NUM; i++)
    {
        m_pShadowCopy[i] = MakeSharedPtr<PostProcess>(m_pContext, "ShadowCopy");
        static std::vector<std::string> tech_names = {
            "ShadowCopyR",
            "ShadowCopyG",
            "ShadowCopyB",
            "ShadowCopyA"
        };
		static std::vector<RenderStateDesc> render_states = {
			RenderStateDesc::ShadowCopyR(),
			RenderStateDesc::ShadowCopyG(),
			RenderStateDesc::ShadowCopyB(),
			RenderStateDesc::ShadowCopyA()
		};
        SEEK_RETIF_FAIL(effect.LoadTechnique(tech_names[i], &render_states[i], "PostProcessVS", "ShadowCopyPS", nullptr));
        m_pShadowCopy[i]->Init(tech_names[i]);        
        m_pShadowCopy[i]->SetClear(false);
    }

    return S_Success;
}
RendererReturnValue ForwardShadowLayer::PostProcessShadowMapJob(uint32_t light_index)
{
    SceneManager& sc = m_pContext->SceneManagerInstance();
    LightComponent* pLight = sc.GetLightComponentByIndex(light_index);

    CameraComponent* pCam = pLight->GetShadowMapCamera();
    if (!pCam)
    {
        LOG_ERROR("ShadowLayer::PostProcessShadowMapJob(): No Shadow Map Camera.");
        return RRV_NextJob;
    }
    if (light_index >= m_vShadowIndex.size() ||
        m_vShadowIndex[light_index].first >= MAX_SHADOW_LIGHT_NUM ||
        m_vShadowIndex[light_index].second >= MAX_SHADOW_LIGHT_NUM)
    {
        LOG_ERROR("ShadowLayer::PostProcessShadowMapJob(): Invalid ShadowIndex.");
        return RRV_NextJob;
    }

    uint32_t filtered_shadow_map_index = m_vShadowIndex[light_index].second;
    LightType light_type = pLight->GetLightType();
    if (light_type == LightType::Spot ||
        light_type == LightType::Directional)
    {
        m_pShadowCopy[filtered_shadow_map_index]->SetParam("shadow_map_tex", m_pSmDepthTex);
        m_pShadowCopy[filtered_shadow_map_index]->SetOutput(0, m_pFilteredSmTex);
        m_pShadowCopy[filtered_shadow_map_index]->Run();
    }
    return RRV_NextJob;
}
/******************************************************************************
 * DeferredShadowLayer
 ******************************************************************************/
//DeferredShadowLayer::DeferredShadowLayer(Context* context)
//    :ShadowLayer(context)
//{
//
//}
//SResult DeferredShadowLayer::InitResource()
//{
//    ShadowLayer::InitResource();
//
//    RenderContext& rc = m_pContext->RenderContextInstance();
//
//    Texture::Desc desc;
//    desc.width = SM_SIZE;
//    desc.height = SM_SIZE;
//    desc.depth = 1;
//    desc.num_mips = 1;
//    desc.num_samples = 1;
//    desc.type = TextureType::Tex2D;
//    desc.format = PixelFormat::R8G8B8A8_UNORM;
//    desc.flags = RESOURCE_FLAG_RENDER_TARGET | RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK;
//    m_pShadowingTex = rc.CreateTexture2D(desc);
//    m_pShadowingFb = rc.CreateFrameBuffer();
//    m_pShadowingFb->AttachTargetView(FrameBuffer::Attachment::Color0, rc.CreateRenderTargetView(m_pShadowingTex));
//
//
//    std::vector<EffectPredefine> no_csm_predefine;
//    std::vector<EffectPredefine> use_csm_predefine;
//
//    EffectPredefine predefine;
//    predefine.name = "USE_CSM";
//    predefine.value = "0";
//    no_csm_predefine.push_back(predefine);
//    
//    predefine.value = "1";
//    use_csm_predefine.push_back(predefine);
//
//    m_pShadow0Effect = this->GetShadowEffect(0);
//    m_pQuadMesh = QuadMesh_GetMesh(rc);
//    m_pShadowingTechs[(uint32_t)LightType::Ambient][0] = nullptr;
//    m_pShadowingTechs[(uint32_t)LightType::Ambient][1] = nullptr;
//    m_pShadowingTechs[(uint32_t)LightType::Directional][0] = m_pShadow0Effect->GetTechnique("ShadowingDirectional", no_csm_predefine);
//    m_pShadowingTechs[(uint32_t)LightType::Directional][1] = m_pShadow0Effect->GetTechnique("ShadowingDirectional", use_csm_predefine);
//    m_pShadowingTechs[(uint32_t)LightType::Spot][0] = m_pShadow0Effect->GetTechnique("ShadowingSpot");
//    m_pShadowingTechs[(uint32_t)LightType::Spot][1] = nullptr;
//    m_pShadowingTechs[(uint32_t)LightType::Point][0] = m_pShadow0Effect->GetTechnique("ShadowingPoint");
//    m_pShadowingTechs[(uint32_t)LightType::Point][1] = nullptr;
//
//    m_pShadowingRenderStates[0] = rc.GetRenderState(RenderStateDesc::ShadowCopyR());
//    m_pShadowingRenderStates[1] = rc.GetRenderState(RenderStateDesc::ShadowCopyG());
//    m_pShadowingRenderStates[2] = rc.GetRenderState(RenderStateDesc::ShadowCopyB());
//    m_pShadowingRenderStates[3] = rc.GetRenderState(RenderStateDesc::ShadowCopyA());
//
//    m_pParamDepthTex            = m_pShadow0Effect->GetParamByName("depth_tex");
//    m_pParamShadowTex           = m_pShadow0Effect->GetParamByName("shadowTex");
//    m_pParamCubeShadowTex       = m_pShadow0Effect->GetParamByName("cubeShadowTex");
//    m_pParamCsmTex[0]           = m_pShadow0Effect->GetParamByName("cascadedShadowMap0");
//    m_pParamCsmTex[1]           = m_pShadow0Effect->GetParamByName("cascadedShadowMap1");
//    m_pParamCsmTex[2]           = m_pShadow0Effect->GetParamByName("cascadedShadowMap2");
//    m_pParamCsmTex[3]           = m_pShadow0Effect->GetParamByName("cascadedShadowMap3");
//    m_pParamCsmDistance         = m_pShadow0Effect->GetParamByName("csmDistance");
//    m_pParamCsmLightVPMatrices  = m_pShadow0Effect->GetParamByName("csmLightVPMatrices");
//    m_pParamLightViewMatrices   = m_pShadow0Effect->GetParamByName("lightViewMatrix");
//
//    m_pParamCameraInfo = m_pShadow0Effect->GetParamByName("cameraInfo");
//    m_pParamLightInfo = m_pShadow0Effect->GetParamByName("lightInfo");
//    m_pParamDeferredLightingInfo = m_pShadow0Effect->GetParamByName("deferredLightingInfo");
//
//    return DVF_Success;
//}
//RendererReturnValue DeferredShadowLayer::PostProcessShadowMapJob(uint32_t light_index)
//{
//    SceneManager& sm = m_pContext->SceneManagerInstance();
//    SceneRenderer& sr = m_pContext->SceneRendererInstance();
//    RenderContext& rc = m_pContext->RenderContextInstance();
//    SResult res = DVF_Success;
//
//    if (light_index >= m_vShadowIndex.size() ||
//        m_vShadowIndex[light_index].first >= MAX_SHADOW_LIGHT_NUM ||
//        m_vShadowIndex[light_index].second >= MAX_SHADOW_LIGHT_NUM)
//    {
//        LOG_ERROR("ShadowLayer::PostProcessShadowMapJob(): Invalid ShadowIndex.");
//        return RRV_NextJob;
//    }
//
//    CameraComponent* pCam = sm.GetActiveCamera();
//    LightInfo lightInfo;
//    LightComponent* pLight = sm.GetLightComponentByIndex(light_index);
//    LightType light_type = pLight->GetLightType();
//    sr.FillLightInfoByLightIndex(lightInfo, pCam, light_index);
//    m_pParamLightInfo->UpdateConstantBuffer(&lightInfo, sizeof(lightInfo));
//
//    CameraInfo cameraInfo;
//    if (pCam)
//    {
//        cameraInfo.posWorld = pCam->GetWorldTransform().GetTranslation();
//        cameraInfo.nearFarPlane = pCam->GetNearFarPlane();
//    }
//    m_pParamCameraInfo->UpdateConstantBuffer(&cameraInfo, sizeof(cameraInfo));
//
//    DeferredLightingInfo deferred_info;
//    deferred_info.lightVolumeMV = pCam ? pCam->GetInvProjMatrix().Transpose() : Matrix4::Identity();
//    deferred_info.lightVolumeInvView = pCam ? pCam->GetInvViewMatrix().Transpose() : Matrix4::Identity();
//    m_pParamDeferredLightingInfo->UpdateConstantBuffer(&deferred_info, sizeof(deferred_info));
//
//    uint32_t use_csm = pLight->CascadedShadow() ? 1 : 0;
//
//    *m_pParamDepthTex = sr.GetSceneDepthStencilTex();
//    *m_pParamShadowTex = m_pSmDepthTex;
//    if (light_type == LightType::Point)
//        *m_pParamCubeShadowTex = m_pCubeSmTex;
//    else if (light_type == LightType::Directional && use_csm)
//    {
//        for (uint32_t i = 0; i < NUM_CSM_LEVELS; ++i)
//        {
//            if (m_pParamCsmTex[i])
//                *m_pParamCsmTex[i] = m_vCsmDepthTex[i];
//        }
//        if (m_pParamCsmDistance)
//        {
//            float4& far_distance = ((DirectionalLightComponent*)pLight)->GetCSMCamera()->GetCsmFarDistance();
//            m_pParamCsmDistance->UpdateConstantBuffer(&far_distance[0], sizeof(float4));
//        }
//        if (m_pParamCsmLightVPMatrices)
//        {
//            std::vector<Matrix4> vp_matrices;
//            for (uint32_t i = 0; i < NUM_CSM_LEVELS; ++i)
//            {
//                Matrix4 vp = ((DirectionalLightComponent*)pLight)->GetCSMCamera()->GetCameraByCSMIndex(i)->GetViewProjMatrix();
//                vp_matrices.push_back(vp.Transpose());
//            }
//            m_pParamCsmLightVPMatrices->UpdateConstantBuffer(&vp_matrices[0], sizeof(Matrix4) * vp_matrices.size());
//        }
//        if (m_pParamLightViewMatrices)
//        {
//            Matrix4 view = pLight->GetShadowMapCamera()->GetViewMatrix().Transpose();
//            float4 pos = float4(0, 0, 0, 1) * view.Transpose();
//            pos = pos;
//            m_pParamLightViewMatrices->UpdateConstantBuffer(&view, sizeof(Matrix4));
//        }
//    }
//
//    rc.BindFrameBuffer(m_pShadowingFb);
//    uint32_t shadow_map_index = m_vShadowIndex[light_index].first;
//    if (shadow_map_index == 0)
//        m_pShadowingFb->Clear(FrameBuffer::CBM_Color, float4(1, 1, 1, 1));
//    
//    Technique* pTech = m_pShadowingTechs[(uint32_t)light_type][use_csm];
//    m_pQuadMesh->SetRenderState(m_pShadowingRenderStates[shadow_map_index]);
//    res = rc.Render(pTech->GetProgram().get(), m_pQuadMesh);
//    m_pQuadMesh->SetRenderState(nullptr);
//
//#if 0
//    static int draw = 1;
//    if (draw)
//    {
//        m_pShadowingTex->DumpToFile("d:\\shadowing.rgba");
//        --draw;
//    }
//#endif
//    return RRV_NextJob;
//}
SEEK_NAMESPACE_END
