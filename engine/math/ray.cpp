#include "math/ray.h"
#include "math/math_utility.h"
#include <cmath>

SEEK_NAMESPACE_BEGIN

Ray::Ray(float3 const& ori, float3 const& dir)
{
    m_fOrigin = ori;
    m_fDirection = dir;
}
SEEK_NAMESPACE_END
