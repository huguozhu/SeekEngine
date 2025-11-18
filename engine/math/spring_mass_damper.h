#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "aabbox.h"

SEEK_NAMESPACE_BEGIN

class SpringMassDamper
{
public:
    SpringMassDamper(float mass, float damping, float stiffness, float3 x0, float3 v0);

    // 计算在时间t时的位移
    float3 calculateDisplacement(float t);
    // 计算在时间t时的速度
    float3 calculateVelocity(float t);

    void Tick(float delta_time);

	float3 GetPosition() const { return m_x0; }
	float3 GetVelocity() const { return m_v0; }

private:
    // 欠阻尼位移计算
    float3 underdampedDisplacement(float t);
    // 欠阻尼速度计算
    float3 underdampedVelocity(float t);

    // 临界阻尼位移计算
    float3 criticallyDampedDisplacement(float t);
    // 临界阻尼速度计算
    float3 criticallyDampedVelocity(float t);

    // 过阻尼位移计算
    float3 overdampedDisplacement(float t);
    // 过阻尼速度计算
    float3 overdampedVelocity(float t);

private:
	// 输入参数
    float m_m; // 质量
    float m_c; // 阻尼系数
    float m_k; // 弹簧刚度
    float3 m_x0; // 初始位移
    float3 m_v0; // 初始速度


    // 计算得到的参数
    float m_OmegaN; // 无阻尼自然频率
    /*
     * 阻尼比:
	 *      [0, 1): 欠阻尼, 系统会发生振荡，振幅逐渐减小直至停止
	 *       = 1:   临界阻尼, 系统在最短时间内返回平衡位置，不发生振荡
	 *       > 1:   过阻尼, 系统返回平衡位置的速度较慢，不发生振荡
     */
    float m_Zeta;

};
using SpringMassDamperPtr = std::shared_ptr<SpringMassDamper>;

SEEK_NAMESPACE_END
