#include "components/metaball_component.h"
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

#include <algorithm>
#include <random>

#define SEEK_MACRO_FILE_UID 75     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * Metaball2DComponent
 ******************************************************************************/
Metaball2DComponent::Metaball2DComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index)
	:Sprite2DComponent(context, width, height, draw_index)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    MeshData bg_mesh_data;
    CreateSprite2D(bg_mesh_data, width, height);
    m_pMetaballMesh = CreateMeshFromMeshData(m_pContext, bg_mesh_data);
   
    
    this->InitRandomBall(MAX_METABALL_NUM);
    m_Param.width = static_cast<float>(m_iWidth);
    m_Param.height = static_cast<float>(m_iHeight);
    m_Param.surfaceThreshold = 0.001f;
    m_Param.metaballCount = MAX_METABALL_NUM;
    for (uint32_t i =0 ; i < MAX_METABALL_NUM; ++i)
        m_Param.balls[i] = m_vMetaballs[i];	
}
Metaball2DComponent::~Metaball2DComponent()
{

}
void Metaball2DComponent::InitRandomBall(uint32_t init_count)
{
    m_vMetaballs.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0, std::min(m_iWidth, m_iHeight));
    std::uniform_real_distribution<float> radiusDist(2, 3);
    std::uniform_real_distribution<float> velecity(-600.0, 600.1);

    for (uint32_t i = 0; i < init_count; ++i)
    {
        Metaball ball;
        ball.position = {
            posDist(gen)+100,
            posDist(gen), // 从上方开始
            0
        };
        ball.radius = radiusDist(gen);
        ball.velocity = { velecity(gen), velecity(gen), 0};

        m_vMetaballs.push_back(ball);
    }
    m_pSimulator = MakeSharedPtr<MetaballWaterSimulator>();
    m_pSimulator->SetMetaballs(&m_vMetaballs);
    //m_pSimulator->SetGravity(0.0);
	m_pSimulator->SetBoundary(float3(0, 0, 0), float3((float)m_iWidth, (float)m_iHeight, 0));
}
void Metaball2DComponent::AddMetaball(Metaball ball)
{
    m_vMetaballs.push_back(ball);
}
void Metaball2DComponent::RemoveMetaball(uint32_t index)
{
    if (index < m_vMetaballs.size())
    {
        m_vMetaballs.erase(m_vMetaballs.begin() + index);
    }
}
SResult Metaball2DComponent::OnRenderBegin()
{
    if (!m_pMetaballTech)
    {
        this->InitShaders();
        if (!m_pMetaballTech)
			return ERR_INVALID_INIT;
    }
	CameraComponent* cam_2d = m_pContext->SceneManagerInstance().GetActiveCamera();
    Matrix4 vp = cam_2d->GetViewProjMatrix();
    {
		Matrix4 mvp = GetWorldMatrix() * vp;
		m_pMvpCbBuffer->Update(&mvp, sizeof(Matrix4));

        for (uint32_t i = 0; i < MAX_METABALL_NUM; ++i)
            m_Param.balls[i] = m_vMetaballs[i];
        m_pParamCbBuffer->Update(&m_Param, sizeof(MetaballParam));
		m_pMetaballTech->SetParam("src_tex", m_pImage);
	}

    return S_Success;
}
SResult Metaball2DComponent::Render()
{
    SEEK_RETIF_FAIL(this->OnRenderBegin());
    SEEK_RETIF_FAIL(m_pMetaballTech->Render(m_pMetaballMesh));
    SEEK_RETIF_FAIL(this->OnRenderEnd());
    return S_Success;
}
SResult Metaball2DComponent::Tick(float delta_time)
{
    m_pSimulator->Tick(delta_time);
    return S_Success;
}
void Metaball2DComponent::Reset()
{
}
SResult Metaball2DComponent::InitShaders()
{
    Effect& effect = m_pContext->EffectInstance();
    RHIContext& rc = m_pContext->RHIContextInstance();
    SEEK_RETIF_FAIL(effect.LoadTechnique("Metaball", &RenderStateDesc::Default2D(),
        "Sprite2DVS", "Sprite2DMetaballPS", nullptr));

    m_pMetaballTech = effect.GetTechnique("Metaball");
    m_pParamCbBuffer = rc.CreateConstantBuffer(sizeof(MetaballParam), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
    m_pMetaballTech->SetParam("cb_MetaBallParam", m_pParamCbBuffer);

    m_pMvpCbBuffer = rc.CreateConstantBuffer(sizeof(Matrix4), RESOURCE_FLAG_CPU_WRITE | RESOURCE_FLAG_GPU_READ);
    m_pMetaballTech->SetParam("cb_Sprite2DInfo", m_pMvpCbBuffer);
    return S_Success;
}

/******************************************************************************
 * Metaball3DComponent
 ******************************************************************************/

SEEK_NAMESPACE_END

