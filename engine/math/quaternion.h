#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

class Quaternion
{
public:
    Quaternion() {}
    Quaternion(Quaternion const& rhs);
    Quaternion(float4 const& vec);
    Quaternion(float x, float y, float z, float w);

    Matrix4 ToMatrix() const;
    static Quaternion const& Identity();

    float& operator[](size_t index) { return m_vQuat[index]; }
    const float& operator[](size_t index) const { return m_vQuat[index]; }

    float& x() { return m_vQuat[0]; }
    float& y() { return m_vQuat[1]; }
    float& z() { return m_vQuat[2]; }
    float& w() { return m_vQuat[3]; }
    const float& x() const { return m_vQuat[0]; }
    const float& y() const { return m_vQuat[1]; }
    const float& z() const { return m_vQuat[2]; }
    const float& w() const { return m_vQuat[3]; }

    Quaternion const& operator*=(Quaternion const& rhs);
    Quaternion const operator-() const;

    Quaternion operator-(Quaternion const& rhs) const;
    Quaternion operator+(Quaternion const& rhs) const;
    Quaternion operator*(Quaternion const& rhs) const;
    Quaternion operator*(float rhs) const;
    float operator^(Quaternion const& rhs) const;

    float3 const v() const;
    void v(float3 const& rhs);

    bool operator==(Quaternion const& rhs) const;
    bool operator!=(Quaternion const& rhs) const;

    Quaternion Inverse() const;
    Quaternion Log() const;
    Quaternion Exp() const;

    std::string str() const { return m_vQuat.str(); }

private:
    float4 m_vQuat;
};

SEEK_NAMESPACE_END
