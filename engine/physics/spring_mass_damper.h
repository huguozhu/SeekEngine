#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/aabbox.h"
#include "math/math_utility.h"

SEEK_NAMESPACE_BEGIN

class SpringMassDamper
{
public:
    SpringMassDamper(double mass, double damping, double stiffness, double3 center, double3 x0, double3 v0);

    // 计算在时间 t 时的位移
    double3 CalculatePosition(double t);
    // 计算在时间 t 时的速度
    double3 CalculateVelocity(double t);

    void Tick(double delta_time);

	double3 GetPosition() const { return m_x0; }
	double3 GetVelocity() const { return m_v0; }

    double GetDampingRatio() const { return m_Zeta; }
    double GetNaturalFrequency() const { return m_OmegaN; }

    double CalculateAdaptiveTimeStep() {
        // 基于系统动力学估计时间尺度
        double characteristic_time = 1.0 / this->GetNaturalFrequency();

        // 时间步长应为特征时间的 1/10 到 1/100
        return characteristic_time / 50.0;
    }


private:
    // 欠阻尼位移计算
    double3 UnderdampedPosition(double t);
	// 欠阻尼位移计算(精确方法)
    double3 UnderdampedPosition_Exact(double t);
    // 欠阻尼速度计算
    double3 UnderdampedVelocity(double t);

    // 临界阻尼位移计算
    double3 CriticallyDampedPosition(double t);
    // 临界阻尼速度计算
    double3 CriticallyDampedVelocity(double t);

    // 过阻尼位移计算
    double3 OverdampedPosition(double t);
    // 过阻尼速度计算
    double3 OverdampedVelocity(double t);

private:
	// 物理参数
		double m_mass;      // 质量, 单位: kg 千克
		double m_damping;   // 阻尼系数, 单位: N·s/m 牛顿·秒/米
		double m_stiffness; // 弹簧刚度, 单位: N/m 牛顿/米
    double3 m_center;   // 弹簧中心位置
    double3 m_x0;   // 初始位置
    double3 m_v0;   // 初始速度


    // 派生得到的参数
    double m_OmegaN; // 无阻尼自然频率, 单位: rad/s 弧度/秒
    /*
     * 阻尼比:
		 *      [0, 1): 欠阻尼, 系统会发生振荡，振幅逐渐减小直至停止
		 *       = 1:   临界阻尼, 系统以最快时间返回平衡位置，不振荡
		 *       > 1:   过阻尼, 系统返回平衡位置的速度较慢，不振荡
     */
	double m_Zeta;

    // 精确三角函数计算（避免角度累积误差）
    void PreciseCosSin(double angle, double& cos_val, double& sin_val) const;

    // 判断阻尼类型
    bool IsUnderdamped() const { return m_Zeta < 1.0 - Math::FOLAT_EPSILON; }
    bool IsCriticallyDamped() const { return std::abs(m_Zeta - 1.0) < Math::FOLAT_EPSILON; }
    bool IsOverdamped() const { return m_Zeta > 1.0 + Math::FOLAT_EPSILON; }

};
using SpringMassDamperPtr = std::shared_ptr<SpringMassDamper>;

SEEK_NAMESPACE_END
