#include "components/metaball_component.h"
#include "kernel/context.h"

#include <algorithm>
#include <random>

SEEK_NAMESPACE_BEGIN

/******************************************************************************
 * Metaball2DComponent
 ******************************************************************************/
Metaball2DComponent::Metaball2DComponent(Context* context, uint32_t width, uint32_t height, uint32_t draw_index)
	:Sprite2DComponent(context, width, height, draw_index)
{
    m_eComponentType = ComponentType::Metaball;
    this->InitRandomBall(5);
}
Metaball2DComponent::~Metaball2DComponent()
{

}
void Metaball2DComponent::InitRandomBall(uint32_t init_count)
{
    m_vMetaballs.clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-2.0f, 2.0f);
    std::uniform_real_distribution<float> radiusDist(0.2f, 0.4f);

    for (uint32_t i = 0; i < init_count; ++i)
    {
        Metaball ball;
        ball.position = {
            posDist(gen),
            posDist(gen) + 1.0f, // 从上方开始
            posDist(gen)
        };
        ball.radius = radiusDist(gen);
        ball.velocity = { 0, 0, 0 };

        m_vMetaballs.push_back(ball);
    }
    m_pSimulator = MakeSharedPtr<MetaballWaterSimulator>();
    m_pSimulator->SetMetaballs(m_vMetaballs);
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
SResult Metaball2DComponent::Render()
{
    return S_Success;
}
SResult Metaball2DComponent::Tick(float delta_time)
{
    m_pSimulator->Tick(delta_time);
    return S_Success;
}


/******************************************************************************
 * Metaball3DComponent
 ******************************************************************************/

SEEK_NAMESPACE_END

