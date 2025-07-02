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
    RHIContext& rc = m_pContext->RHIContextInstance();
    // Step1: Create Constant Buffer & Update Datas
    if (!m_pWaterMarkDescCBuffer)
        m_pWaterMarkDescCBuffer = rc.CreateConstantBuffer(sizeof(WaterMarkDesc), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_SRV);
    if (!m_pWaterMarkTargetSizeCBuffer)
        m_pWaterMarkTargetSizeCBuffer = rc.CreateConstantBuffer(sizeof(float2), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_SRV);
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

    // Step3: Create & Update the RepeatWaterMark Texture
    if (!m_pRepeatWaterMark ||
        (m_pRepeatWaterMark->Width() != w || m_pRepeatWaterMark->Height() != h))
    {
        if (m_pRepeatWaterMark)
            m_pRepeatWaterMark.reset();        
        RHITexture::Desc desc;
        desc.width = w;
        desc.height = h;
        desc.type = TextureType::Tex2D;
        desc.format = PixelFormat::R8G8B8A8_UNORM;
        desc.flags = RESOURCE_FLAG_SRV | RESOURCE_FLAG_UAV;
        m_pRepeatWaterMark = rc.CreateTexture2D(desc);
    }

    // Step4: Generate full Watermark Texture
    // Two methods: 1) using CS     2) using Texture Copy
    bool use_cs_to_generate_watermark = 0;
    if (m_sDesc.watermark_type == WaterMarkType_Repeat)
    {
        if (use_cs_to_generate_watermark)
        {
            if (!m_pTechWaterMarkGenerate)
            {
                std::string tech_name = "WaterMarkGenerate";
                Effect& effect = m_pContext->EffectInstance();
                SEEK_RETIF_FAIL(effect.LoadTechnique(tech_name, &RenderStateDesc::WaterMark(),
                    nullptr, nullptr, "WaterMarkGenerateCS"));
                m_pTechWaterMarkGenerate = effect.GetTechnique(tech_name);
            }

            if (m_bDirty)
            {
                m_pWaterMarkDescCBuffer->Update(&m_sDesc, sizeof(WaterMarkDesc));
                m_pTechWaterMarkGenerate->SetParam("waterMarkDesc", m_pWaterMarkDescCBuffer);
                m_pTechWaterMarkGenerate->SetParam("targetSize", m_pWaterMarkTargetSizeCBuffer);
                m_pTechWaterMarkGenerate->SetParam("watermark_tex", m_pWaterMarkTex);
                m_pTechWaterMarkGenerate->SetParam("repeat_watermark_target", m_pRepeatWaterMark);

                uint32_t dispatch_x = (w + Gen_WaterMark_CS_SIZE - 1) / Gen_WaterMark_CS_SIZE;
                uint32_t dispatch_y = (h + Gen_WaterMark_CS_SIZE - 1) / Gen_WaterMark_CS_SIZE;
                m_pTechWaterMarkGenerate->Dispatch(dispatch_x, dispatch_y, 1);
            }
        }
        else
        {
            if (m_bDirty)
            {
                int center_left_x = w * 0.5 - m_sDesc.src_width * 0.5;
                int center_top_y =  h * 0.5 - m_sDesc.src_height * 0.5;

                int offset_x_same_line = m_sDesc.offset_x + m_sDesc.src_width;
                int tile_x_count = w / offset_x_same_line + 6;
                int tile_y_count = h / m_sDesc.offset_y   + 6;

                for (    int tile_y = (int)(-tile_y_count / 2); tile_y <= (int)(tile_y_count / 2); tile_y++)
                {
                    for (int tile_x = (int)(-tile_x_count / 2); tile_x <= (int)(tile_x_count / 2); tile_x++)
                    {
                        int dst_x = center_left_x + tile_y * m_sDesc.offset_x + tile_x * offset_x_same_line;
                        int dst_y = center_top_y +  tile_y * m_sDesc.offset_y;
                        if (dst_x > w || dst_y > h ||
                            dst_x + m_sDesc.src_width < 0 ||
                            dst_y + m_sDesc.src_height < 0)
                            continue;                       
                        rc.CopyTextureRegion(m_pWaterMarkTex, m_pRepeatWaterMark, dst_x, dst_y);
                    }
                }
            }
        }
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
