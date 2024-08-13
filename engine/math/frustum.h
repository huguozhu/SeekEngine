#pragma once

#include "kernel/kernel.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/plane.h"
#include "math/aabbox.h"
#include "components/mesh_component.h"

SEEK_NAMESPACE_BEGIN

class Frustum
{
public:
    Frustum() {}

    void ClipMatrix(Matrix4 const& clip, Matrix4 const& inv_clip);
    VisibleMark Intersect(AABBox const& aabb) const;

    std::string Str() const;

private:
    std::array<Plane, 6> m_aPlanes;
    std::array<float3, 8> m_aCorners;

};

SEEK_NAMESPACE_END
