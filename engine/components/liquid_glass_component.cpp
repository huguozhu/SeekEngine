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
#include "math/math_utility.h"

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

    m_InitShapeType[0] = ShapeType::Circle;
    m_InitShapeSize[0] = float2(std::min(m_iWidth, m_iHeight) / 4.0f);
    m_InitShapeCenter[0] = float2(m_iWidth / 4.0f, m_iHeight / 4.0f);

    m_InitShapeType[1] = ShapeType::Ellipse;
    m_InitShapeSize[1] = float2(m_iWidth / 6.0f, m_iHeight / 6.0f);
    m_InitShapeCenter[1] = float2(m_iWidth / 4.0f * 3, m_iHeight / 2.0f);

	// init spring param
    this->ResetAll();
}
LiquidGlassComponent::~LiquidGlassComponent()
{
    m_pLiquildTech = nullptr;
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
        m_pLiquildTech->SetParam("src_tex", m_pImage);
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
    delta_time = 0.003;
    for (int i = 0; i < Num_Shapes; i++)
    {
        if (m_States[i] == SpringMassDamperState::Playing)
        {
            m_pSpringMassDamper[i]->Tick(delta_time);
            float3 v = m_pSpringMassDamper[i]->GetPosition();
            m_Param.shapes[i].center = float2(v.x(), v.y()) + float2(m_iWidth / 2.0f, m_iHeight / 2.0f);
        }
    }

    m_fDuration += delta_time;
    if (m_fDuration > 3.0f)
    {
        m_fDuration = 0.0;
		this->Reset(0);
    }
    
    return S_Success;
}

float sdEllipse(float2 p, float2 r) {
    // This implementation is an approximation but good enough for most cases
    float k0 = Math::Length(p / r);
    float k1 = Math::Length(p / (r * r));
    return k0 * (k0 - 1.0) / k1;
}
bool LiquidGlassComponent::HitShape(float2 hit_pos, int& hited_shape_index)
{
    for (int i = 0; i < Num_Shapes; i++)
    {
        SdfShape shape = m_Param.shapes[i];
        float sdf = 10000.0f;
        if (shape.shape_type == ShapeType::Circle)
        {
            sdf = Math::Distance(hit_pos, shape.center) - shape.size.x();
        }
        else if (shape.shape_type == ShapeType::Ellipse)
        {
            sdf = sdEllipse( (hit_pos - shape.center), shape.size);
        }
        if (sdf <= 0)
        {
            hited_shape_index = i;
            return true;
        }
    }
    return false;
}
void LiquidGlassComponent::SetSpringState(uint32_t spring_index, SpringMassDamperState state)
{
    if (spring_index < Num_Shapes)
        m_States[spring_index] = state;
    return;
}
void LiquidGlassComponent::SetInitShapeType(uint32_t shape_index, ShapeType type)
{
    if (shape_index < Num_Shapes)
        m_InitShapeType[shape_index] = type;
    return;
}
void LiquidGlassComponent::SetInitShapeSize(uint32_t shape_index, float2 size)
{
    if (shape_index < Num_Shapes)
        m_InitShapeSize[shape_index] = size;
    return;
}
void LiquidGlassComponent::SetInitShapeCenter(uint32_t shape_index, float2 pos)
{
    if (shape_index < Num_Shapes)
        m_InitShapeCenter[shape_index] = pos;
    return;
}
void LiquidGlassComponent::SetCurShapeCenter(uint32_t shape_index, float2 pos)
{
    if (shape_index < Num_Shapes)
        m_Param.shapes[shape_index].center = pos;
    return;
}
void LiquidGlassComponent::Reset(uint32_t i)
{
    // init spring param
    {
        m_Param.shapes[i].shape_type = m_InitShapeType[i];
        m_Param.shapes[i].size = m_InitShapeSize[i];
        m_Param.shapes[i].center = m_InitShapeCenter[i];

        float2 center = m_InitShapeCenter[i] - float2(m_iWidth / 2.0f, m_iHeight / 2.0f);
        m_pSpringMassDamper[i] = MakeSharedPtr<SpringMassDamper>(0.005f, 0.01f, 2.0f, float3(center.x(), center.y(), 0.0), 0.0f);
    }
}
void LiquidGlassComponent::ResetAll()
{
    for (uint32_t i = 0; i < Num_Shapes; i++)
        this->Reset(i);
}
SResult LiquidGlassComponent::InitShaders()
{
    Effect& effect = m_pContext->EffectInstance();
	RHIContext& rc = m_pContext->RHIContextInstance();
    SEEK_RETIF_FAIL(effect.LoadTechnique("LiquildGlass", &RenderStateDesc::Default2D(),
        "Sprite2DVS", "LiquidGlassPS", nullptr));

    m_pLiquildTech = effect.GetTechnique("LiquildGlass");
	m_pParamCbBuffer = rc.CreateConstantBuffer(sizeof(LiquidGlassParam), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
	m_pLiquildTech->SetParam("cb_LiquidGlassParam", m_pParamCbBuffer);

    m_pMvpCbBuffer = rc.CreateConstantBuffer(sizeof(Matrix4), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
    m_pLiquildTech->SetParam("cb_Sprite2DInfo", m_pMvpCbBuffer);
    return S_Success;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
