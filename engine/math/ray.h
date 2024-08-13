#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "aabbox.h"

SEEK_NAMESPACE_BEGIN

class Ray
{
public:
    Ray(float3 const& ori, float3 const& dir);

    float3 const& GetOrigin() const { return m_fOrigin; }
    float3 const& GetDirection() const { return m_fDirection; }

private:
    float3 m_fOrigin;
    float3 m_fDirection;
};

SEEK_NAMESPACE_END
