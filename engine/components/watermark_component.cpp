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
    m_pWaterMarkTex = watermark_tex;
    return S_Success;
}
SResult WaterMarkComponent::SetWaterMarkDesc(WaterMarkDesc desc)
{
    m_sDesc = desc;
    return S_Success;
}

SResult WaterMarkComponent::OnRenderBegin(Technique* tech, RHIMeshPtr mesh)
{
    if (!m_pWaterMarkDescCBuffer)
    {
        m_pWaterMarkDescCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(WaterMarkDesc), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_SHADER_RESOURCE);
        tech->SetParam("waterMarkDesc", m_pWaterMarkDescCBuffer);
    }
    m_pWaterMarkDescCBuffer->Update(&m_sDesc, sizeof(WaterMarkDesc));

    if (!m_pWaterMarkTargetSizeCBuffer)
    {
        m_pWaterMarkTargetSizeCBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(float2), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_SHADER_RESOURCE);
        tech->SetParam("targetSize", m_pWaterMarkTargetSizeCBuffer);
    }
    Viewport vp = m_pContext->GetViewport();
    float2 target_size(vp.Width(), vp.Height());
    m_pWaterMarkTargetSizeCBuffer->Update(&target_size, sizeof(float2));

    tech->SetParam("watermark_tex", m_pWaterMarkTex);

    return S_Success;
}
SResult WaterMarkComponent::Render()
{
    if (!m_pTechWaterMark)
    {
        std::string tech_name = "WaterMark";
        Effect& effect = m_pContext->EffectInstance();
        SEEK_RETIF_FAIL(effect.LoadTechnique(tech_name, &RenderStateDesc::WaterMark(),
           "WaterMarkVS", "WaterMarkPS", nullptr));
        m_pTechWaterMark = effect.GetTechnique(tech_name);
    }

    RHIMeshPtr pMesh = m_vMeshes[0];
    SEEK_RETIF_FAIL(this->OnRenderBegin(m_pTechWaterMark, pMesh));
    SEEK_RETIF_FAIL(m_pTechWaterMark->Render(pMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
