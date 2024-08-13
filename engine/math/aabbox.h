#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

class AABBox
{
public:
    AABBox();
    AABBox(bool valid);
    AABBox(float3 const& min, float3 const& max);

    AABBox& operator&=(AABBox const& rhs);
    AABBox& operator|=(AABBox const& rhs);
    AABBox& operator= (AABBox const& rhs);
    bool    operator==(AABBox const& rhs);

    AABBox& operator+(AABBox const& rhs);
    AABBox const operator-() const;

    float3&         Min()       { return m_fMin; }
    float3 const&   Min() const { return m_fMin; }
    void            Min(float3 v) { m_fMin = v; }

    float3&         Max()       { return m_fMax; }
    float3 const&   Max() const { return m_fMax; }
    void            Max(float3 v) { m_fMax = v; }
    
    float3          HalfSize() const;
    float3          Center() const;
    float3          Corner(size_t index) const;

    std::string     Str() const;

private:
    float3          m_fMin;
    float3          m_fMax;
};

SEEK_NAMESPACE_END
