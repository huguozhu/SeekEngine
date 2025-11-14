#include "components/liquid_glass_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"
#include "utils/shape_mesh.h"


#define SEEK_MACRO_FILE_UID 75     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static const int TECH_INDEX = 0;

LiquidGlassComponent::LiquidGlassComponent(Context* context)
    :MeshComponent(context)
{
    m_eComponentType = ComponentType::LiquidGlass;
	m_szName = "LiquidGlassComponent";

    RHIContext& rc = m_pContext->RHIContextInstance();
    m_pBgMesh = QuadMesh_GetMesh(rc);

    MeshData mesh_data;
    CreateSphere(mesh_data, 0.5f);
    m_pShpereMesh = CreateMeshFromMeshData(m_pContext, mesh_data);
}
LiquidGlassComponent::~LiquidGlassComponent()
{
    m_pLiquildTech = nullptr;
    m_pBgTex = nullptr;
}
SResult LiquidGlassComponent::OnRenderBegin(Technique* tech, RHIMeshPtr mesh)
{
    if (m_pDrawBgTech)
        m_pDrawBgTech->SetParam("src_tex", m_pBgTex);

    if (m_pLiquildTech)
    {
		m_pParamCbBuffer->Update(&m_Param, sizeof(LiquidGlassParam));
        m_pLiquildTech->SetParam("src_tex", m_pBgTex);
    }
    return S_Success;
}
SResult LiquidGlassComponent::Render()
{
    if (!m_pLiquildTech || !m_pDrawBgTech)
    {
        this->InitShaders();
        if (!m_pLiquildTech)
            return ERR_INVALID_INIT;
    }

    SEEK_RETIF_FAIL(this->OnRenderBegin(nullptr, nullptr));
    SEEK_RETIF_FAIL(m_pDrawBgTech->Render(m_pBgMesh));
    SEEK_RETIF_FAIL(m_pLiquildTech->Render(m_pBgMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());

    return S_Success;
}
SResult LiquidGlassComponent::Tick(float delta_time)
{
    
    return S_Success;
}
SResult LiquidGlassComponent::InitShaders()
{
    Effect& effect = m_pContext->EffectInstance();

    effect.LoadTechnique("DrawLiquidGlassBg", &RenderStateDesc::Default2D(),
		"PostProcessVS", "CopyPS", nullptr);
	m_pDrawBgTech = effect.GetTechnique("DrawLiquidGlassBg");

    struct LiquidGlassTechInfo
    {
        const std::string tech_name;
        const char* vertex_shader_name;
        const char* pixel_shader_name;
        const char* compute_shader_name;
    } lg_techs[] = {
       { "LiquildGlass",  "PostProcessVS",    "LiquidGlassPS",    nullptr},
    };
    
    int tech_count = sizeof(lg_techs) / sizeof(LiquidGlassTechInfo);
    for (size_t i = 0; i < tech_count; ++i)
    {
        const std::string tech_name = lg_techs[i].tech_name;
        if (!effect.GetTechnique(tech_name))
        {
            SEEK_RETIF_FAIL(effect.LoadTechnique(tech_name, &RenderStateDesc::Default2D(),
                lg_techs[i].vertex_shader_name,
                lg_techs[i].pixel_shader_name,
                lg_techs[i].compute_shader_name));
        }
    }
    m_pLiquildTech = effect.GetTechnique(lg_techs[TECH_INDEX].tech_name);

	m_pParamCbBuffer = m_pContext->RHIContextInstance().CreateConstantBuffer(sizeof(LiquidGlassParam), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
	m_pLiquildTech->SetParam("cb_LiquidGlassParam", m_pParamCbBuffer);
    return S_Success;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
