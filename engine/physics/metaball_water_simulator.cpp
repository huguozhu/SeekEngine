#include "physics/metaball_water_simulator.h"
#include <algorithm>
#include <random>

SEEK_NAMESPACE_BEGIN

MetaballWaterSimulator::MetaballWaterSimulator()
{
}

MetaballWaterSimulator::~MetaballWaterSimulator()
{
}

void MetaballWaterSimulator::Tick(float delta_time)
{
    if (delta_time > 0.03)
		delta_time = 0.03f; // 防止大时间步长导致的数值不稳定

    if (!m_pMetaballs)
        return;
    ApplyPhysics(delta_time);
    HandleCollisions();
    //HandleMetaballFusion();
}

void MetaballWaterSimulator::ApplyPhysics(float delta_time)
{
    for (auto& ball : *m_pMetaballs)
    {
        // 应用重力
        ball.velocity.y() += m_gravity * delta_time;

        // 应用阻尼
        ball.velocity.x() *= m_damping;
        ball.velocity.z() *= m_damping;

        // 更新位置
        ball.position.x() += ball.velocity.x() * delta_time;
        ball.position.y() += ball.velocity.y() * delta_time;
        ball.position.z() += ball.velocity.z() * delta_time;

        // Metaball 之间的排斥力（模拟表面张力）
        for (auto& other : *m_pMetaballs)
        {
            if (&ball != &other)
            {
                float3 repulsion = CalculateRepulsionForce(ball, other);
                ball.velocity.x() += repulsion.x() * delta_time;
                ball.velocity.y() += repulsion.y() * delta_time;
                ball.velocity.z() += repulsion.z() * delta_time;
            }
        }
    }
}

float3 MetaballWaterSimulator::CalculateRepulsionForce(const Metaball& a, const Metaball& b)
{
    float3 posA = a.position;
    float3 posB = b.position;

    float3 delta = posA - posB;
    float distance = Math::Distance(posA, posB);

    // 如果距离太小，计算排斥力
    float minDistance = a.radius + b.radius;
    if (distance < minDistance * 1.5f && distance > 0.001f)
    {
        float force = m_surfaceTension * (minDistance * 1.5f - distance) / distance;
        delta = delta * force;
        return delta;
    }
    return { 0, 0, 0 };
}

void MetaballWaterSimulator::HandleCollisions()
{
    for (auto& ball : *m_pMetaballs)
    {
        // 边界碰撞
        if (ball.position.x() - ball.radius < m_boundaryMin.x())
        {
            ball.position.x() = m_boundaryMin.x() + ball.radius;
            ball.velocity.x() = -ball.velocity.x() * m_CollisionDamping;
        }
        else if (ball.position.x() + ball.radius > m_boundaryMax.x())
        {
            ball.position.x() = m_boundaryMax.x() - ball.radius;
            ball.velocity.x() = -ball.velocity.x() * m_CollisionDamping;
        }

        if (ball.position.y() - ball.radius < m_boundaryMin.y())
        {
            ball.position.y() = m_boundaryMin.y() + ball.radius;
            ball.velocity.y() = -ball.velocity.y() * m_CollisionDamping;

            //ball.velocity.x() *= 0.9; // 地面摩擦
            //ball.velocity.z() *= 0.9;
        }
        else if (ball.position.y() + ball.radius > m_boundaryMax.y())
        {
            ball.position.y() = m_boundaryMax.y() - ball.radius;
            ball.velocity.y() = -ball.velocity.y() * m_CollisionDamping;
        }

        if (ball.position.z() - ball.radius < m_boundaryMin.z())
        {
            ball.position.z() = m_boundaryMin.z() + ball.radius;
            ball.velocity.z() = -ball.velocity.z() * m_CollisionDamping;
        }
        else if (ball.position.z() + ball.radius > m_boundaryMax.z())
        {
            ball.position.z() = m_boundaryMax.z() - ball.radius;
            ball.velocity.z() = -ball.velocity.z() * m_CollisionDamping;
        }
    }
}

void MetaballWaterSimulator::HandleMetaballFusion()
{
    for (size_t i = 0; i < (*m_pMetaballs).size(); ++i)
    {
        for (size_t j = i + 1; j < (*m_pMetaballs).size(); ++j)
        {
            auto& ball1 = (*m_pMetaballs)[i];
            auto& ball2 = (*m_pMetaballs)[j];

            float3 pos1 = ball1.position;
            float3 pos2 = ball2.position;

            float distance = Math::Distance(pos1, pos2);
            float fusionDistance = (ball1.radius + ball2.radius) * m_fusionThreshold;

            if (distance < fusionDistance)
            {
                // 融合两个 Metaball - 创建更大的球
                Metaball newBall;
                newBall.radius = sqrtf(ball1.radius * ball1.radius + ball2.radius * ball2.radius);

                // 质量加权平均位置
                float mass1 = ball1.radius * ball1.radius;
                float mass2 = ball2.radius * ball2.radius;
                float totalMass = mass1 + mass2;

                float3 newPos = (pos1 * mass1 / totalMass) + (pos2 * mass2 / totalMass);

                newBall.position = newPos;

                // 动量守恒
                float3 newVel = (ball1.velocity * mass1 / totalMass) +(ball2.velocity * mass2 / totalMass);
                newBall.velocity = newVel;

                newBall.intensity = 1.0f;

                // 移除旧球，添加新球

                (*m_pMetaballs).erase((*m_pMetaballs).begin() + j);
                (*m_pMetaballs).erase((*m_pMetaballs).begin() + i);
                (*m_pMetaballs).push_back(newBall);

                return; // 一次只处理一对融合
            }
        }
    }
}



SEEK_NAMESPACE_END