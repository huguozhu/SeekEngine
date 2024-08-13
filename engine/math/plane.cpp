#include "math/plane.h"
#include "math/math_utility.h"
#include <cmath>

SEEK_NAMESPACE_BEGIN

Plane::Plane(float4 const& rhs)
    : m_fPlane(rhs)
{}
Plane& Plane::operator=(float4 const& rhs)
{
    m_fPlane = rhs;
    return *this;
}
float3 Plane::Normal() const
{
    return float3(m_fPlane[0], m_fPlane[1], m_fPlane[2]);
}
void Plane::Normal(float3 const& rhs)
{
    m_fPlane[0] = rhs[0];
    m_fPlane[1] = rhs[1];
    m_fPlane[2] = rhs[2];
}
Plane Plane::Normalize() const
{
    float3 normal = this->Normal();
    float length = (float)sqrt(Math::Dot(normal, normal));
    float const inv((float)(1.0 / length));
    return Plane(m_fPlane * inv);
}
float Plane::GetDistance(float3 const& point) const
{
    return m_fPlane[0] * point[0] + m_fPlane[1] * point[1] + m_fPlane[2] * point[2] + m_fPlane[3];
}
SEEK_NAMESPACE_END
