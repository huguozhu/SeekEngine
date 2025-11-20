#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/aabbox.h"
#include "math/math_utility.h"
#include <span>

SEEK_NAMESPACE_BEGIN

#include "shader/shared/MetaBall.h"

class MetaballWaterSimulator
{
public:
    MetaballWaterSimulator();
    ~MetaballWaterSimulator();

    void Tick(float delta_time);

    // 物理参数
    void SetMetaballs(std::vector<Metaball>* balls) { m_pMetaballs = balls; }
    void SetGravity(float gravity) { m_gravity = gravity; }
    void SetDamping(float damping) { m_damping = damping; }
    void SetSurfaceTension(float tension) { m_surfaceTension = tension; }
	void SetFusionTHreshold(float threshold) { m_fusionThreshold = threshold; }
    void SetBoundary(float3 min, float3 max) { m_boundaryMin = min; m_boundaryMax = max; }

private:
    void ApplyPhysics(float delta_time);
    void HandleCollisions();
    void HandleMetaballFusion();
    float3 CalculateRepulsionForce(const Metaball& a, const Metaball& b);

private:
    std::vector<Metaball>* m_pMetaballs = nullptr;

    // 物理参数
    float m_gravity = -1.0f;
    float m_damping = 1.0;
    float m_surfaceTension = 1.5f;
    float m_fusionThreshold = 1.5f; // 融合距离阈值
	float m_CollisionDamping = 1.0f; // 碰撞反弹阻尼系数(=1.0:无阻尼)

    float3 m_boundaryMin = { -3.0f, -3.0f, -3.0f };
    float3 m_boundaryMax = { 3.0f, 3.0f, 3.0f };
};
using MetaballWaterSimulatorPtr = std::shared_ptr<MetaballWaterSimulator>;

SEEK_NAMESPACE_END