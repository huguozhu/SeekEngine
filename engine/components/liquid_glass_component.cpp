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

#include <cmath>

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

	float half_w = m_iWidth * 0.5f;
	float half_h = m_iHeight * 0.5f;
    m_iNumShapes = 5;

    m_Param.width = static_cast<float>(m_iWidth);
    m_Param.height = static_cast<float>(m_iHeight);
	m_Param.shape_count = m_iNumShapes;
    m_Param.sdf_smooth_value = 100.0f;
    
    m_InitShapeType[0]      = ShapeType::Circle;
    m_InitShapeSize[0]      = float2(std::min(half_w, half_h) * 0.5f, 0.0);
    m_InitShapeCenter[0]    = float2(half_w * 1.0, half_h * 0.5f);
    m_InitShapeParams[0]    = float3(1.0, 1.0, 0.0);

    m_InitShapeType[1]      = ShapeType::Ellipse;
    m_InitShapeSize[1]      = float2(half_w * 0.3f, half_h * 0.3f);
    m_InitShapeCenter[1]    = float2(half_w * 0.6f, half_h * 0.4f );
    m_InitShapeParams[1]    = float3(1.0, 1.0, 0.0);

    m_InitShapeType[2]      = ShapeType::Round_Rectangle;
    m_InitShapeSize[2]      = float2(half_w * 0.3, half_h * 0.3);
    m_InitShapeCenter[2]    = float2(half_w * 0.7, half_h);
    m_InitShapeParams[2]    = float3(1.0, 1.0, 50.0);

    m_InitShapeType[3]      = ShapeType::Super_Ellipse;
    m_InitShapeSize[3]      = float2(half_w * 0.3, half_h * 0.3);
    m_InitShapeCenter[3]    = float2(half_w * 1.1, half_h * 0.9);
    m_InitShapeParams[3]    = float3(1.0, 1.0, 1.0);

    m_InitShapeType[4]      = ShapeType::Circle;
    m_InitShapeSize[4]      = float2(std::min(half_w, half_h) * 0.65f);
    m_InitShapeCenter[4]    = float2(half_w * 1.8, half_h * 1.2);
    m_InitShapeParams[4]    = float3(1.0, 1.0, 0.0);

    m_States[0] = SpringMassDamperState::Playing;
    m_States[1] = SpringMassDamperState::Playing;
    m_States[2] = SpringMassDamperState::Playing;
    m_States[3] = SpringMassDamperState::Playing;
    m_States[4] = SpringMassDamperState::Playing;
    m_States[5] = SpringMassDamperState::Playing;
    damper_centers[0] = double3(half_w * 1.0, half_h * 1.0, 0.0);
    damper_centers[1] = double3(half_w * 0.5, half_h * 0.5, 0.0);
    damper_centers[2] = double3(half_w * 0.5, half_h * 1.5, 0.0);
    damper_centers[3] = double3(half_w * 1.5, half_h * 0.5, 0.0);
    damper_centers[4] = double3(half_w * 1.5, half_h * 1.5, 0.0);
    damper_centers[5] = double3(half_w * 0.0, half_h * 0.0, 0.0);

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
    //delta_time *= 0.2;
    for (int i = 0; i < m_iNumShapes; i++)
    {
        if (m_States[i] == SpringMassDamperState::Playing)
        {
            m_pSpringMassDamper[i]->Tick(delta_time);
            float3 v = m_pSpringMassDamper[i]->GetPosition();
            m_Param.shapes[i].center = float2(v.x(), v.y());
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
float sdRoundedRectangle(float2 p, float2 b, float r) {
    float2 d = float2(Math::Abs(p.x()), Math::Abs(p.y())) - b + r;
    return Math::Length(Math::Max(d, 0.0)) - r + std::min(std::max(d.x(), d.y()), 0.0f);
}
float sdSuperellipse(float2 p, float n, float r) {
    // Take the absolute value of the coordinates, as the formula uses |x| and |y|.
    // This makes the shape symmetrical in all quadrants.
    float2 p_abs = float2(Math::Abs(p.x()), Math::Abs(p.y()));

    // Numerator of the distance formula: |x|^n + |y|^n - r^n
    // This is the implicit equation of the superellipse. Its value is 0 on the
    // boundary, negative inside, and positive outside.
    float numerator = std::pow(p_abs.x(), n) + std::pow(p_abs.y(), n) - pow(r, n);

    // Denominator: n * sqrt(|x|^(2n-2) + |y|^(2n-2))
    // This is the magnitude of the gradient of the implicit function. Dividing by it
    // normalizes the result, making it a better approximation of true Euclidean distance.
    // Note: The pow() function can be computationally expensive. For a fixed, integer 'n'
    // (like n=4), you would get better performance by using direct multiplication.
    float den_x = std::pow(p_abs.x(), 2.0 * n - 2.0);
    float den_y = std::pow(p_abs.y(), 2.0 * n - 2.0);

    // Add a small epsilon to prevent division by zero at the origin (0,0).
    float denominator = n * sqrt(den_x + den_y) + 0.00001;

    // The final signed distance
    return numerator / denominator;
}
bool LiquidGlassComponent::HitShape(float2 hit_pos, int& hited_shape_index)
{
    for (int i = 0; i < m_iNumShapes; i++)
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
        else if (shape.shape_type == ShapeType::Round_Rectangle)
        {
            sdf = sdRoundedRectangle((hit_pos - shape.center), shape.size, shape.shape_params.z());
        }
        else if (shape.shape_type == ShapeType::Super_Ellipse)
        {
            sdf = sdSuperellipse((hit_pos - shape.center), shape.shape_params.z(), shape.size.x());
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
    if (spring_index < m_iNumShapes)
        m_States[spring_index] = state;
    return;
}
void LiquidGlassComponent::SetInitShapeType(uint32_t shape_index, ShapeType type)
{
    if (shape_index < m_iNumShapes)
        m_InitShapeType[shape_index] = type;
    return;
}
void LiquidGlassComponent::SetInitShapeSize(uint32_t shape_index, float2 size)
{
    if (shape_index < m_iNumShapes)
        m_InitShapeSize[shape_index] = size;
    return;
}
void LiquidGlassComponent::SetInitShapeCenter(uint32_t shape_index, float2 pos)
{
    if (shape_index < m_iNumShapes)
        m_InitShapeCenter[shape_index] = pos;
    return;
}
void LiquidGlassComponent::SetCurShapeCenter(uint32_t shape_index, float2 pos)
{
    if (shape_index < m_iNumShapes)
        m_Param.shapes[shape_index].center = pos;
    return;
}
void LiquidGlassComponent::Reset(uint32_t i)
{
    // init spring param
    {
        m_Param.shapes[i].shape_type    = m_InitShapeType[i];
        m_Param.shapes[i].shape_params  = m_InitShapeParams[i];        
        m_Param.shapes[i].center        = m_InitShapeCenter[i];
        m_Param.shapes[i].size          = m_InitShapeSize[i];
        m_pSpringMassDamper[i] = MakeSharedPtr<SpringMassDamper>(mass[i], damping[i], stiffness[i], damper_centers[i], float3(m_InitShapeCenter[i].x(), m_InitShapeCenter[i].y(), 0.0), 0.0f);
    }
}
void LiquidGlassComponent::ResetAll()
{
    for (uint32_t i = 0; i < m_iNumShapes; i++)
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
