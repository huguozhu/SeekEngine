#include "math/spring_mass_damper.h"
#include "math/math_utility.h"
#include <cmath>

SEEK_NAMESPACE_BEGIN

SpringMassDamper::SpringMassDamper(float mass, float damping, float stiffness, float3 x0, float3 v0)
	:m_m(mass), m_c(damping), m_k(stiffness), m_x0(x0), m_v0(v0)
{
	// 计算系统参数
	m_OmegaN = std::sqrt(m_k / m_m);
	m_Zeta = m_c / (2 * std::sqrt(m_m * m_k));
}

void SpringMassDamper::Tick(float delta_time)
{
	m_x0 = this->calculateDisplacement(delta_time);
    m_v0 = this->calculateVelocity(delta_time);
}

float3 SpringMassDamper::calculateDisplacement(float t)
{
    if (m_Zeta < 1.0) {
        // 欠阻尼情况
        return underdampedDisplacement(t);
    }
    else if (std::abs(m_Zeta - 1.0) < 1e-10) {
        // 临界阻尼情况
        return criticallyDampedDisplacement(t);
    }
    else {
        // 过阻尼情况
        return overdampedDisplacement(t);
    }
}

// 计算在时间t时的速度
float3 SpringMassDamper::calculateVelocity(float t)
{
    if (m_Zeta < 1.0) {
        return underdampedVelocity(t);
    }
    else if (std::abs(m_Zeta - 1.0) < 1e-10) {
        return criticallyDampedVelocity(t);
    }
    else {
        return overdampedVelocity(t);
    }
}



float3 SpringMassDamper::underdampedDisplacement(float t)
{
    float omega_d = m_OmegaN * std::sqrt(1 - m_Zeta * m_Zeta);
    float3 term1 = m_x0 * std::cos(omega_d * t);
    float3 term2 = (m_v0 + m_x0 * m_Zeta * m_OmegaN) / omega_d * std::sin(omega_d * t);
	float decay = std::exp(-m_Zeta * m_OmegaN * t);
    float3 v = (term1 + term2) * decay;
    return v;
}

// 欠阻尼速度计算
float3 SpringMassDamper::underdampedVelocity(float t) {
    float omega_d = m_OmegaN * std::sqrt(1 - m_Zeta * m_Zeta);
    float exp_term = std::exp(-m_Zeta * m_OmegaN * t);

    float cos_term = std::cos(omega_d * t);
    float sin_term = std::sin(omega_d * t);

    float3 A = m_x0;
    float3 B = (m_v0 + m_x0* m_Zeta * m_OmegaN) / omega_d;

    float3 dxdt = (
        (A * -m_Zeta * m_OmegaN + B * omega_d) * cos_term +
        (B * -m_Zeta * m_OmegaN - A * omega_d) * sin_term
        ) * exp_term;

    return dxdt;
}

// 临界阻尼位移计算
float3 SpringMassDamper::criticallyDampedDisplacement(float t) {
    float3 A = m_x0;
    float3 B = m_v0 + m_x0 * m_OmegaN;
    return (A + B * t) * std::exp(-m_OmegaN * t);
}

// 临界阻尼速度计算
float3 SpringMassDamper::criticallyDampedVelocity(float t) {
    float3 A = m_x0;
    float3 B = m_v0 + m_x0 * m_OmegaN;
    float exp_term = std::exp(-m_OmegaN * t);
    return (B - (A + B * t) * m_OmegaN) * exp_term;
}

// 过阻尼位移计算
float3 SpringMassDamper::overdampedDisplacement(float t) {
    float omega_p = m_OmegaN * std::sqrt(m_Zeta * m_Zeta - 1);
    float3 A = m_x0;
    float3 B = (m_v0 + m_x0 * m_Zeta * m_OmegaN) / omega_p;

    float cosh_term = std::cosh(omega_p * t);
    float sinh_term = std::sinh(omega_p * t);

    return (A * cosh_term + B * sinh_term) * std::exp(-m_Zeta * m_OmegaN * t);
}

// 过阻尼速度计算
float3 SpringMassDamper::overdampedVelocity(float t) {
    float omega_p = m_OmegaN * std::sqrt(m_Zeta * m_Zeta - 1);
    float3 A = m_x0;
    float3 B = (m_v0 + m_x0 * m_Zeta * m_OmegaN) / omega_p;

    float cosh_term = std::cosh(omega_p * t);
    float sinh_term = std::sinh(omega_p * t);
    float exp_term = std::exp(-m_Zeta * m_OmegaN * t);

    float3 dxdt = (
        (A * -m_Zeta * m_OmegaN + B * omega_p) * cosh_term +
        (B * -m_Zeta * m_OmegaN + A * omega_p) * sinh_term
        ) * exp_term;

    return dxdt;
}

SEEK_NAMESPACE_END
