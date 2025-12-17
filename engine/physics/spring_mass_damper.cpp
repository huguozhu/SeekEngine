#include "physics/spring_mass_damper.h"
#include "math/math_utility.h"
#include <cmath>

SEEK_NAMESPACE_BEGIN
// 安全的指数计算，避免下溢
double SafeExp(double x) 
{
    if (x < Math::MIN_EXP_ARG) {  // exp(-700) far less double's min value 
        return 0.0;
    }
    return std::exp(x);
}

SpringMassDamper::SpringMassDamper(double mass, double damping, double stiffness, double3 center, double3 x0, double3 v0)
	:m_mass(mass), m_damping(damping), m_stiffness(stiffness), m_center(center), m_x0(x0), m_v0(v0)
{
	// 计算系统参数
	m_OmegaN = std::sqrt(m_stiffness / m_mass);
	m_Zeta = m_damping / (2.0 * std::sqrt(m_mass * m_stiffness));
}

void SpringMassDamper::Tick(double delta_time)
{
    m_x0 = m_x0 - m_center;
	double3 next_x0 = this->CalculatePosition(delta_time);
    double3 next_v0 = this->CalculateVelocity(delta_time);
    m_x0 = next_x0 + m_center;
    m_v0 = next_v0;
}

double3 SpringMassDamper::CalculatePosition(double t)
{
    if (IsUnderdamped()) {
        // 欠阻尼情况
        return UnderdampedPosition(t);
		//return UnderdampedPosition_Exact(t);
    }
    else if (IsCriticallyDamped()) {
        // 临界阻尼情况
        return CriticallyDampedPosition(t);
    }
    else {
        // 过阻尼情况
        return OverdampedPosition(t);
    }
}

// 计算在时间t时的速度
double3 SpringMassDamper::CalculateVelocity(double t)
{
    if (m_Zeta < 1.0) {
        return UnderdampedVelocity(t);
    }
    else if (std::abs(m_Zeta - 1.0) < 1e-10) {
        return CriticallyDampedVelocity(t);
    }
    else {
        return OverdampedVelocity(t);
    }
}

double3 SpringMassDamper::UnderdampedPosition(double t)
{
    double omega_d = m_OmegaN * std::sqrt(1 - m_Zeta * m_Zeta);
    double3 term1 = m_x0 * std::cos(omega_d * t);
    double3 term2 = (m_v0 + m_x0 * m_Zeta * m_OmegaN) / omega_d * std::sin(omega_d * t);
	double decay = std::exp(-m_Zeta * m_OmegaN * t);
    double3 v = (term1 + term2) * decay;
    return v;
}
double3 SpringMassDamper::UnderdampedPosition_Exact(double t)
{
    // 精确的无阻尼简谐运动公式:
    // x(t) = x0 * cos(ωt) + (v0/ω) * sin(ωt)

    const double angle = m_OmegaN * t;

    // 使用高精度的三角函数计算
    double cos_val, sin_val;
    PreciseCosSin(angle, cos_val, sin_val);

    const double3 term1 = m_x0 * cos_val;
    const double3 term2 = m_v0 * (sin_val / m_OmegaN);

    double3 result = term1 + term2;

    return result;
}
// 欠阻尼速度计算
double3 SpringMassDamper::UnderdampedVelocity(double t) {
    double omega_d = m_OmegaN * std::sqrt(1 - m_Zeta * m_Zeta);
    double exp_term = std::exp(-m_Zeta * m_OmegaN * t);

    double cos_term = std::cos(omega_d * t);
    double sin_term = std::sin(omega_d * t);

    double3 A = m_x0;
    double3 B = (m_v0 + m_x0* m_Zeta * m_OmegaN) / omega_d;

    double3 dxdt = (
        (A * -m_Zeta * m_OmegaN + B * omega_d) * cos_term +
        (B * -m_Zeta * m_OmegaN - A * omega_d) * sin_term
        ) * exp_term;

    return dxdt;
}

// 临界阻尼位移计算
double3 SpringMassDamper::CriticallyDampedPosition(double t) {
    double3 A = m_x0;
    double3 B = m_v0 + m_x0 * m_OmegaN;
    return (A + B * t) * std::exp(-m_OmegaN * t);
}

// 临界阻尼速度计算
double3 SpringMassDamper::CriticallyDampedVelocity(double t) {
    double3 A = m_x0;
    double3 B = m_v0 + m_x0 * m_OmegaN;
    double exp_term = std::exp(-m_OmegaN * t);
    return (B - (A + B * t) * m_OmegaN) * exp_term;
}

// 过阻尼位移计算
double3 SpringMassDamper::OverdampedPosition(double t) {
    double omega_p = m_OmegaN * std::sqrt(m_Zeta * m_Zeta - 1);
    double3 A = m_x0;
    double3 B = (m_v0 + m_x0 * m_Zeta * m_OmegaN) / omega_p;

    double cosh_term = std::cosh(omega_p * t);
    double sinh_term = std::sinh(omega_p * t);

    return (A * cosh_term + B * sinh_term) * std::exp(-m_Zeta * m_OmegaN * t);
}

// 过阻尼速度计算
double3 SpringMassDamper::OverdampedVelocity(double t) {
    double omega_p = m_OmegaN * std::sqrt(m_Zeta * m_Zeta - 1);
    double3 A = m_x0;
    double3 B = (m_v0 + m_x0 * m_Zeta * m_OmegaN) / omega_p;

    double cosh_term = std::cosh(omega_p * t);
    double sinh_term = std::sinh(omega_p * t);
    double exp_term = std::exp(-m_Zeta * m_OmegaN * t);

    double3 dxdt = (
        (A * -m_Zeta * m_OmegaN + B * omega_p) * cosh_term +
        (B * -m_Zeta * m_OmegaN + A * omega_p) * sinh_term
        ) * exp_term;

    return dxdt;
}
void SpringMassDamper::PreciseCosSin(double angle, double& cos_val, double& sin_val) const {
    // 将角度归一化到 [-2π, 2π] 范围内以提高精度
    angle = std::fmod(angle, 2.0 * Math::PI);

    //// 对于小角度，使用更精确的计算
    //if (std::abs(angle) < 1e-8) {
    //    cos_val = 1.0 - 0.5 * angle * angle;
    //    sin_val = angle;
    //}
    
    // 对于小角度，使用泰勒展开避免精度损失
    if (std::abs(angle) < 1e-8) {
        double angle2 = angle * angle;
        double angle4 = angle2 * angle2;
        cos_val = 1.0 - angle2 / 2.0 + angle4 / 24.0;
        sin_val = angle - (angle * angle2) / 6.0;
    }
    // 对于大角度，直接使用标准函数但进行范围缩减
    else if (std::abs(angle) > 1e6) {
        // 对于非常大的角度，进行额外的范围缩减
        angle = std::fmod(angle, 2.0 * Math::PI);
        cos_val = std::cos(angle);
        sin_val = std::sin(angle);
    }
    else {
        cos_val = std::cos(angle);
        sin_val = std::sin(angle);
    }
    // 确保结果在有效范围内
    cos_val = std::max(-1.0, std::min(1.0, cos_val));
    sin_val = std::max(-1.0, std::min(1.0, sin_val));
}

SEEK_NAMESPACE_END
