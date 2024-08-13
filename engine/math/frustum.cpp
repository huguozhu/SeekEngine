#include "math/frustum.h"
#include "math/math_utility.h"
#include "utils/log.h"
#include "components/mesh_component.h"

SEEK_NAMESPACE_BEGIN

void Frustum::ClipMatrix(Matrix4 const& clip, Matrix4 const& inv_clip)
{
    m_aCorners[0] = Math::TransformVectorWithScale(float3(-1, -1, 0), inv_clip);        // left bottom near
    m_aCorners[1] = Math::TransformVectorWithScale(float3(+1, -1, 0), inv_clip);        // right bottom near
    m_aCorners[2] = Math::TransformVectorWithScale(float3(-1, +1, 0), inv_clip);        // left top near
    m_aCorners[3] = Math::TransformVectorWithScale(float3(+1, +1, 0), inv_clip);        // right top near
    m_aCorners[4] = Math::TransformVectorWithScale(float3(-1, -1, +1), inv_clip);        // left bottom far
    m_aCorners[5] = Math::TransformVectorWithScale(float3(+1, -1, +1), inv_clip);        // right bottom far
    m_aCorners[6] = Math::TransformVectorWithScale(float3(-1, +1, +1), inv_clip);        // left top far
    m_aCorners[7] = Math::TransformVectorWithScale(float3(+1, +1, +1), inv_clip);        // right top far

    float4 const& column1(clip.Col(0));
    float4 const& column2(clip.Col(1));
    float4 const& column3(clip.Col(2));
    float4 const& column4(clip.Col(3));

    m_aPlanes[0] = column4 - column1;       // right
    m_aPlanes[1] = column4 + column1;       // left
    m_aPlanes[2] = column4 - column2;       // top
    m_aPlanes[3] = column4 + column2;       // bottom
    m_aPlanes[4] = column4 - column3;       // far
    m_aPlanes[5] = column3;                 // near;
    for (auto& plane : m_aPlanes)
        plane = plane.Normalize();
}
VisibleMark Frustum::Intersect(AABBox const& aabb) const
{
    float3 min_pt = aabb.Min();
    float3 max_pt = aabb.Max();
    bool intersect = false;
    for (int i = 0; i < 6; i++)
    {
        Plane const& plane = m_aPlanes[i];
        float3 v0((plane.a() < 0) ? min_pt[0] : max_pt[0], (plane.b() < 0) ? min_pt[1] : max_pt[1], (plane.c() < 0) ? min_pt[2] : max_pt[2]);
        float3 v1((plane.a() < 0) ? max_pt[0] : min_pt[0], (plane.b() < 0) ? max_pt[1] : min_pt[1], (plane.c() < 0) ? max_pt[2] : min_pt[2]);

        if (plane.GetDistance(v0) < 0)
        {
            return VisibleMark::No;
        }
        if (plane.GetDistance(v1) < 0)
        {
            intersect = true;
        }
    }
    return intersect ? VisibleMark::Partial : VisibleMark::Yes;
}

std::string Frustum::Str() const
{
    std::string s;
    for (size_t i=0; i<m_aCorners.size(); i++)
    {
        s += "Corner" + m_aCorners[i].str() + "\n";
    }
    for (size_t i=0; i<m_aPlanes.size(); i++)
    {
        s += "surface" + m_aPlanes[i].abcd().str() + "\n";
    }
    return s;
}

SEEK_NAMESPACE_END
