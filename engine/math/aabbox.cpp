#include "math/aabbox.h"
#include "math/math_utility.h"
#include <float.h>

SEEK_NAMESPACE_BEGIN

AABBox::AABBox()
{
    AABBox(true);
}
AABBox::AABBox(bool valid)
{
    if (valid)
    {
        m_fMin = float3(-FLT_MAX);
        m_fMax = float3(FLT_MAX);
    }
    else
    {
        m_fMin = float3(FLT_MAX);
        m_fMax = float3(-FLT_MAX);
    }
}
AABBox::AABBox(float3 const& min, float3 const& max)
{
    m_fMin = min;
    m_fMax = max;
}
AABBox& AABBox::operator&=(AABBox const& rhs)
{
    m_fMin = Math::Max(this->m_fMin, rhs.m_fMin);
    m_fMax = Math::Min(this->m_fMax, rhs.m_fMax);
    return *this;
}
AABBox& AABBox::operator|=(AABBox const& rhs)
{
    m_fMin = Math::Min(this->m_fMin, rhs.m_fMin);
    m_fMax = Math::Max(this->m_fMax, rhs.m_fMax);
    return *this;
}
AABBox& AABBox::operator=(AABBox const& rhs)
{
    if (this != &rhs)
    {
        this->m_fMin = rhs.m_fMin;
        this->m_fMax = rhs.m_fMax;
    }
    return *this;
}
bool AABBox::operator==(AABBox const& rhs)
{
    return this->m_fMin == rhs.m_fMin && this->m_fMax == rhs.m_fMax;
}
AABBox& AABBox::operator+(AABBox const& rhs)
{
    m_fMin = this->m_fMin + rhs.m_fMin;
    m_fMax = this->m_fMax + rhs.m_fMax;
    return *this;
}
AABBox const AABBox::operator-() const
{
    return AABBox(-this->m_fMin, -this->m_fMax);
}
float3 AABBox::HalfSize() const
{
    return (m_fMax - m_fMin) * 0.5f;
}
float3 AABBox::Center() const
{
    return (m_fMax + m_fMin) * 0.5f;
}
float3 AABBox::Corner(size_t index) const
{
    if (index > 8)
        return float3(0.0f);
    return float3(
        (index & 1UL) ? m_fMax[0] : m_fMin[0],
        (index & 2UL) ? m_fMax[1] : m_fMin[1],
        (index & 4UL) ? m_fMax[2] : m_fMin[2]);
}
std::string AABBox::Str() const
{
    return "min:" + m_fMin.str() + " max:" + m_fMax.str();
}

SEEK_NAMESPACE_END
