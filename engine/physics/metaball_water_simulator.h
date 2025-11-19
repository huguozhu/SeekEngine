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

    void Initialize(uint32_t initialCount = 5);
    void Tick(float delta_time);

    const std::vector<Metaball>& GetMetaballs() const { return m_metaballs; }
    uint32_t GetMetaballCount() const { return static_cast<uint32_t>(m_metaballs.size()); }
    void SetMetaballs(std::vector<Metaball> balls) { m_metaballs = balls; }

    // 物理参数
    void SetGravity(float gravity) { m_gravity = gravity; }
    void SetDamping(float damping) { m_damping = damping; }
    void SetSurfaceTension(float tension) { m_surfaceTension = tension; }

    void SetBoundary(float3 min, float3 max) { m_boundaryMin = min; m_boundaryMax = max; }

private:
    void ApplyPhysics(float delta_time);
    void HandleCollisions();
    void HandleMetaballFusion();
    float3 CalculateRepulsionForce(const Metaball& a, const Metaball& b);

private:
    std::vector<Metaball> m_metaballs;

    // 物理参数
    float m_gravity = -2.0f;
    float m_damping = 0.98f;
    float m_surfaceTension = 0.5f;
    float m_fusionThreshold = 0.5f; // 融合距离阈值

    float3 m_boundaryMin = { -3.0f, -3.0f, -3.0f };
    float3 m_boundaryMax = { 3.0f, 3.0f, 3.0f };
};
using MetaballWaterSimulatorPtr = std::shared_ptr<MetaballWaterSimulator>;

SEEK_NAMESPACE_END