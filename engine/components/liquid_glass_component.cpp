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
#include "math/matrix.h"

#define SEEK_MACRO_FILE_UID 75     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN



LiquidGlassComponent::LiquidGlassComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index)
    :Sprite2DComponent(context, width, height, draw_index)
{
	m_szName = "LiquidGlassComponent";

    RHIContext& rc = m_pContext->RHIContextInstance();
    MeshData bg_mesh_data;
    CreateSprite2D(bg_mesh_data, width, height);
    m_pLiquidMesh = CreateMeshFromMeshData(m_pContext, bg_mesh_data);

    m_Param.width = static_cast<float>(m_iWidth);
    m_Param.height = static_cast<float>(m_iHeight);

	// init spring param
	this->Reset();
}
LiquidGlassComponent::~LiquidGlassComponent()
{
    m_pLiquildTech = nullptr;
    m_pBgTex = nullptr;
}
SResult LiquidGlassComponent::OnRenderBegin()
{
    if (!m_pLiquildTech)
    {
        this->InitShaders();
        if (!m_pLiquildTech)
            return ERR_INVALID_INIT;
    }

    CameraComponent* cam_2d = m_pContext->SceneManagerInstance().GetActiveCamera();
    Matrix4 vp = cam_2d->GetViewProjMatrix();

    {
        Matrix4 mvp = GetWorldMatrix() * vp;
		m_pMvpCbBuffer->Update(&mvp, sizeof(Matrix4));
		m_pParamCbBuffer->Update(&m_Param, sizeof(LiquidGlassParam));
        m_pLiquildTech->SetParam("src_tex", m_pBgTex);
    }
    return S_Success;
}
SResult LiquidGlassComponent::Render()
{
    SEEK_RETIF_FAIL(this->OnRenderBegin());
    SEEK_RETIF_FAIL(m_pLiquildTech->Render(m_pLiquidMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());

    return S_Success;
}
SResult LiquidGlassComponent::Tick(float delta_time)
{
    //delta_time = m_pSpringMassDamper_1->CalculateAdaptiveTimeStep();

	m_pSpringMassDamper_1->Tick(delta_time);
    float3 v3_1 = m_pSpringMassDamper_1->GetPosition();
    m_Param.circle_center = float2(v3_1.x(), v3_1.y()) + float2(m_iWidth / 2.0f, m_iHeight / 2.0f);

    m_pSpringMassDamper_2->Tick(delta_time);    
    float3 v3_2 = m_pSpringMassDamper_2->GetPosition();
	m_Param.ellipse_center = float2(v3_2.x(), v3_2.y()) + float2(m_iWidth / 2.0f, m_iHeight / 2.0f);
    
    m_fDuration += delta_time;
    if (m_fDuration > 3.0f)
    {
        m_fDuration = 0.0;
		this->Reset();
    }
    
    return S_Success;
}

void LiquidGlassComponent::Reset()
{
    m_Param.circle_radius = std::min(m_iWidth, m_iHeight) / 4.0f;    
    m_Param.circle_center = float2(m_iWidth / 4.0f, m_iHeight / 4.0f);
    m_Param.ellipse_radius = float2(m_iWidth / 6.0f, m_iHeight / 6.0f);
    m_Param.ellipse_center = float2(m_iWidth / 4.0f * 3, m_iHeight / 2.0f);

    // init spring param
    float2 circle_x0 = m_Param.circle_center - float2(m_iWidth / 2.0f, m_iHeight / 2.0f);
    float2 ellipse_x0 = m_Param.ellipse_center - float2(m_iWidth / 2.0f, m_iHeight / 2.0f);
    m_pSpringMassDamper_1 = MakeSharedPtr<SpringMassDamper>(0.005f, 0.01f, 2.0f, float3(circle_x0.x(), circle_x0.y(), 0.0), 0.0f);
    m_pSpringMassDamper_2 = MakeSharedPtr<SpringMassDamper>(0.005f, 0.01f, 2.0f, float3(ellipse_x0.x(), ellipse_x0.y(), 0.0), 0.0f);
}
SResult LiquidGlassComponent::InitShaders()
{
    Effect& effect = m_pContext->EffectInstance();
	RHIContext& rc = m_pContext->RHIContextInstance();

    struct LiquidGlassTechInfo
    {
        const std::string tech_name;
        const char* vertex_shader_name;
        const char* pixel_shader_name;
        const char* compute_shader_name;
    } lg_techs[] = {
       { "LiquildGlass",  "Sprite2DVS",    "LiquidGlassPS",    nullptr},
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
    m_pLiquildTech = effect.GetTechnique(lg_techs[0].tech_name);
	m_pParamCbBuffer = rc.CreateConstantBuffer(sizeof(LiquidGlassParam), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
	m_pLiquildTech->SetParam("cb_LiquidGlassParam", m_pParamCbBuffer);

    m_pMvpCbBuffer = rc.CreateConstantBuffer(sizeof(Matrix4), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
    m_pLiquildTech->SetParam("cb_Sprite2DInfo", m_pMvpCbBuffer);
    return S_Success;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
