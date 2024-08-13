#include "math/quaternion.h"
#include "math/math_utility.h"
#include <cmath>

SEEK_NAMESPACE_BEGIN

Quaternion::Quaternion(Quaternion const& rhs)
    :m_vQuat(rhs.m_vQuat)
{
}
Quaternion::Quaternion(float4 const& vec)
{
    m_vQuat = vec;
}
Quaternion::Quaternion(float x, float y, float z, float w)
{
    m_vQuat[0] = x;
    m_vQuat[1] = y;
    m_vQuat[2] = z;
    m_vQuat[3] = w;
}
Matrix4 Quaternion::ToMatrix() const
{
    float x = m_vQuat[0];
    float y = m_vQuat[1];
    float z = m_vQuat[2];
    float w = m_vQuat[3];
    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;
    float xx2 = x*x2, xy2 = x*y2, xz2 = x*z2;
    float yy2 = y*y2, yz2 = y*z2, zz2 = z*z2;
    float wx2 = w*x2, wy2 = w*y2, wz2 = w*z2;
    return Matrix4(
        1 - yy2 - zz2,  xy2 + wz2,      xz2 - wy2,      0,
        xy2 - wz2,      1 - xx2 - zz2,  yz2 + wx2,      0,
        xz2 + wy2,      yz2 - wx2,      1 - xx2 - yy2,  0,
        0,              0,              0,              1);
}
Quaternion const& Quaternion::Identity()
{
    static Quaternion const out(0, 0, 0, 1);
    return out;
}
Quaternion const& Quaternion::operator*=(Quaternion const& rhs)
{
    *this = *this * rhs;
    return *this;
}
Quaternion const Quaternion::operator-() const
{
    return Quaternion(-m_vQuat[0], -m_vQuat[1], -m_vQuat[2], -m_vQuat[3]);
}


Quaternion Quaternion::operator-(Quaternion const& rhs) const
{
    return Quaternion(m_vQuat - rhs.m_vQuat);
}
Quaternion Quaternion::operator+(Quaternion const& rhs) const
{
    Quaternion out;
    out.m_vQuat = this->m_vQuat + rhs.m_vQuat;
    return out;
}
Quaternion Quaternion::operator*(Quaternion const& rhs) const
{
    Quaternion out = Quaternion(
        this->x() * rhs.w() - this->y() * rhs.z() + this->z() * rhs.y() + this->w() * rhs.x(),
        this->x() * rhs.z() + this->y() * rhs.w() - this->z() * rhs.x() + this->w() * rhs.y(),
        this->y() * rhs.x() - this->x() * rhs.y() + this->z() * rhs.w() + this->w() * rhs.z(),
        this->w() * rhs.w() - this->x() * rhs.x() - this->y() * rhs.y() - this->z() * rhs.z()
    );
    return out;
}
Quaternion Quaternion::operator*(float rhs) const
{
    Quaternion out;
    out.m_vQuat = this->m_vQuat * rhs;
    return out;
}
float Quaternion::operator^(Quaternion const& rhs) const
{
    float v0 = m_vQuat[3] * rhs.m_vQuat[3];
    float v1 = Math::Dot(this->v(), rhs.v());
    return v0 + v1;
}
float3 const Quaternion::v() const
{
    return float3(m_vQuat[0], m_vQuat[1], m_vQuat[2]);
}
void Quaternion::v(float3 const& rhs)
{
    m_vQuat[0] = rhs.x();
    m_vQuat[1] = rhs.y();
    m_vQuat[2] = rhs.z();
}
bool Quaternion::operator==(Quaternion const& rhs) const
{
    return m_vQuat == rhs.m_vQuat;
}
bool Quaternion::operator!=(Quaternion const& rhs) const
{
    return !(this->operator==(rhs));
}

Quaternion Quaternion::Inverse() const
{
    float d = *this ^ *this;
    return Quaternion(
        -m_vQuat[0] / d,
        -m_vQuat[1] / d,
        -m_vQuat[2] / d,
        m_vQuat[3] / d);
}
Quaternion Quaternion::Log() const
{
    //
    // For unit quaternion, from Advanced Animation and
    // Rendering Techniques by Watt and Watt, Page 366:
    //

    float theta =std::acos(Math::Min(m_vQuat[3], 1.0f));

    if (theta == 0)
        return Quaternion(m_vQuat[0], m_vQuat[1], m_vQuat[2], 0);

    float sintheta = std::sin(theta);

    float k;
    if (abs(sintheta) < 1 && abs(theta) >= Math::FLOAT_MAX * abs(sintheta))
        k = 1;
    else
        k = theta / sintheta;

    return Quaternion(m_vQuat[0] * k, m_vQuat[1] * k, m_vQuat[2] * k, 0.0f);
}

Quaternion Quaternion::Exp() const
{
    //
    // For pure quaternion (zero scalar part):
    // from Advanced Animation and Rendering
    // Techniques by Watt and Watt, Page 366:
    //

    float3 v = this->v();
    float theta = Math::Length(v);
    float sintheta = std::sin(theta);

    float k;
    if (abs(theta) < 1 && abs(sintheta) >= Math::FLOAT_MAX * abs(theta))
        k = 1;
    else
        k = sintheta / theta;

    float costheta = std::cos(theta);

    return Quaternion(v[0]* k, v[1] * k, v[2] * k, costheta);
}
SEEK_NAMESPACE_END
