#include "components/watermark_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_render_buffer.h"
#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"



#define SEEK_MACRO_FILE_UID 36     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

WaterMarkComponent::WaterMarkComponent(Context* context)
    :MeshComponent(context)
{
    m_eComponentType = ComponentType::WaterMark;
    m_szName = "WaterMarkComponent";

    RHIContext& rc = m_pContext->RHIContextInstance();
    RHIMeshPtr mesh = QuadMesh_GetMesh(rc);
    m_vMeshes.push_back(mesh);
}

WaterMarkComponent::~WaterMarkComponent()
{
}

SResult WaterMarkComponent::SetWaterMarkTex(RHITexturePtr watermark_tex)
{
    m_bDirty = true;
    m_pWaterMarkTex = watermark_tex;
    return S_Success;
}
SResult WaterMarkComponent::SetWaterMarkDesc(WaterMarkDesc desc)
{
    m_bDirty = true;
    m_sDesc = desc;
    return S_Success;
}

SResult WaterMarkComponent::OnRenderBegin(Technique* tech, RHIMeshPtr mesh)
{
    if (m_sDesc.watermark_type == WaterMarkType_Single)
    {        
        m_pTechWaterMarkRender->SetParam("watermark_tex", m_pWaterMarkTex);
    }
    else if (m_sDesc.watermark_type == WaterMarkType_Repeat)
    {
        m_pTechWaterMarkRender->SetParam("watermark_tex", m_pRepeatWaterMark);
    }
    return S_Success;
}
SResult WaterMarkComponent::Render()
{
    // Step1: Create Constant Buffer & Update Datas
    if (!m_pWaterMarkDescCBuffer)
        m_pWaterMarkDescCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(WaterMarkDesc), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_SHADER_RESOURCE);
    if (!m_pWaterMarkTargetSizeCBuffer)
        m_pWaterMarkTargetSizeCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(float2), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_SHADER_RESOURCE);
    Viewport vp = m_pContext->GetViewport();
    float w = vp.width;
    float h = vp.height;
    float2 target_size(w, h);
    m_pWaterMarkTargetSizeCBuffer->Update(&target_size, sizeof(float2));
    m_pWaterMarkDescCBuffer->Update(&m_sDesc, sizeof(WaterMarkDesc));
 
    // Step2: Create Techniques
    if (!m_pTechWaterMarkRender)
    {
        std::string tech_name = "WaterMark";
        Effect& effect = m_pContext->EffectInstance();
        SEEK_RETIF_FAIL(effect.LoadTechnique(tech_name, &RenderStateDesc::WaterMark(),
            "WaterMarkVS", "WaterMarkPS", nullptr));
        m_pTechWaterMarkRender = effect.GetTechnique(tech_name);
        m_pTechWaterMarkRender->SetParam("waterMarkDesc", m_pWaterMarkDescCBuffer);
        m_pTechWaterMarkRender->SetParam("targetSize", m_pWaterMarkTargetSizeCBuffer);
    }
    if (m_sDesc.watermark_type == WaterMarkType_Repeat && !m_pTechWaterMarkGenerate)
    {
        std::string tech_name = "WaterMarkGenerate";
        Effect& effect = m_pContext->EffectInstance();
        SEEK_RETIF_FAIL(effect.LoadTechnique(tech_name, &RenderStateDesc::WaterMark(),
            nullptr, nullptr, "WaterMarkGenerateCS"));
        m_pTechWaterMarkGenerate = effect.GetTechnique(tech_name);
    }

    // Step3: Create & Execute WaterMarkGenerate CS
    if (m_sDesc.watermark_type == WaterMarkType_Repeat && m_bDirty)
    {
        if (!m_pRepeatWaterMark ||
            (m_pRepeatWaterMark->Width() != w || m_pRepeatWaterMark->Height() != h) )
        {
            if (m_pRepeatWaterMark)
                m_pRepeatWaterMark.reset();
            RHIContext& rc = m_pContext->RHIContextInstance();
            RHITexture::Desc desc;
            desc.width = w;
            desc.height = h;
            desc.type = TextureType::Tex2D;
            desc.format = PixelFormat::R8G8B8A8_UNORM;
            desc.flags = RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_SHADER_WRITE;
            m_pRepeatWaterMark = rc.CreateTexture2D(desc);
        }
        m_pWaterMarkDescCBuffer->Update(&m_sDesc, sizeof(WaterMarkDesc));
        m_pTechWaterMarkGenerate->SetParam("waterMarkDesc", m_pWaterMarkDescCBuffer);
        m_pTechWaterMarkGenerate->SetParam("targetSize", m_pWaterMarkTargetSizeCBuffer);
        m_pTechWaterMarkGenerate->SetParam("watermark_tex", m_pWaterMarkTex);
        m_pTechWaterMarkGenerate->SetParam("repeat_watermark_target", m_pRepeatWaterMark);

        uint32_t dispatch_x = (w + Gen_WaterMark_CS_SIZE - 1) / Gen_WaterMark_CS_SIZE;
        uint32_t dispatch_y = (h + Gen_WaterMark_CS_SIZE - 1) / Gen_WaterMark_CS_SIZE;
        m_pTechWaterMarkGenerate->Dispatch(dispatch_x, dispatch_y, 1);
    }
    m_bDirty = false;

    // Step4: Render WaterMark
    RHIMeshPtr pMesh = m_vMeshes[0];
    SEEK_RETIF_FAIL(this->OnRenderBegin(m_pTechWaterMarkRender, pMesh));
    SEEK_RETIF_FAIL(m_pTechWaterMarkRender->Render(pMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
