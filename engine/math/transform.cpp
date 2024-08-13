#include "math/transform.h"
#include "math/math_utility.h"
#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

Transform::Transform()
{
    SetIdentity();
}
Transform::Transform(float3 const& translation, Quaternion const& quat, float3 const& scale)
    : m_fTranslation(translation), m_qRotation(quat), m_fScale(scale), m_bDirty(true)
{}
/*
inline bool Transform::operator==(Transform const& v) const
{
    return m_fTranslation == v.m_fTranslation
        && m_qRotation == v.m_qRotation
        && m_fScale == v.m_fScale;
}
inline bool Transform::operator!=(Transform const& v) const
{
    return !(*this == v);
}
*/
void Transform::SetTranslation(float3 const& translation)
{
    m_bDirty = true;
    m_fTranslation = translation;
}
void Transform::SetTranslation(float x, float y, float z)
{
    m_bDirty = true;
    m_fTranslation = float3(x, y, z);
}
void Transform::SetScale(float3 const& scale)
{
    m_bDirty = true;
    m_fScale = scale;
}
void Transform::SetScale(float x, float y, float z)
{
    m_bDirty = true;
    m_fScale = float3(x, y, z);
}
void Transform::SetRotation(Quaternion const& rotation)
{
    m_bDirty = true;
    m_qRotation = rotation;
}
void Transform::Translate(float3 const& translation)
{
    m_bDirty = true;
    m_fTranslation += translation;
}
void Transform::Translate(float x, float y, float z)
{
    m_bDirty = true;
    m_fTranslation += float3(x, y, z);
}
void Transform::Scale(float3 const& scale)
{
    m_bDirty = true;
    m_fScale *= scale;
}
void Transform::Scale(float scale)
{
    m_bDirty = true;
    m_fScale *= scale;
}
void Transform::Scale(float x, float y, float z)
{
    m_bDirty = true;
    m_fScale *= float3(x, y, z);
}
void Transform::Rotate(Quaternion const& rotation)
{
    m_bDirty = true;
    m_qRotation *= rotation;
}
void Transform::Update()
{
    if (!m_bDirty)
        return;
    if (m_eMatDecomposeStatus == MatDecomposeStatus::No)
    {
        LOG_ERROR("Matrix Can't been Decompose, But Use the Trans/Rotate/Scale. It will lead wrong result. %s", m_mMatrix.Str().c_str());
        return;
    }
    m_bDirty = false;
    m_mMatrix = Math::Transform(m_fTranslation, m_qRotation, m_fScale);
    m_mInvMatrix = m_mMatrix.Inverse();
}
bool Transform::CheckMatCanbeDecompose(Matrix4 const& m, float3 const& t, Quaternion const& r, float3 const& s) const
{
    Matrix4 m1 = Math::Transform(t, r, s);

    float const* data1 = m.begin();
    float const* data2 = m1.begin();

    for (int i=0; i<16; i++)
    {
        float diff = Math::Abs(data1[i] - data2[i]);
        if (diff > 0.01f)
            return false;
    }
    return true;
}
Matrix4 const& Transform::Matrix()
{
    Update();
    return m_mMatrix;
}
Matrix4 const& Transform::InvMatrix()
{
    Update();
    return m_mInvMatrix;
}
void Transform::Set(float3 const& translation, Quaternion const& rot, float3 const& scale)
{
    m_bDirty = true;
    m_fTranslation = translation;
    m_qRotation = rot;
    m_fScale = scale;
}
void Transform::Set(Matrix4 const& matrix)
{
    Math::TransformDecompose(m_fScale, m_qRotation, m_fTranslation, matrix);

    if (m_eMatDecomposeStatus == MatDecomposeStatus::Unknown)
    {
        if (CheckMatCanbeDecompose(matrix, m_fTranslation, m_qRotation, m_fScale))
            m_eMatDecomposeStatus = MatDecomposeStatus::Yes;
        else
        {
            Matrix4 m1 = Math::Transform(m_fTranslation, m_qRotation, m_fScale);
            LOG_WARNING("Matrix Can't been Decompose %s %s", matrix.Str().c_str(), m1.Str().c_str());
            m_eMatDecomposeStatus = MatDecomposeStatus::No;
        }
    }

    m_mMatrix = matrix;
    m_mInvMatrix = matrix.Inverse();
    m_bDirty = false;
}
void Transform::SetIdentity()
{
    m_fTranslation      = float3::Zero();
    m_qRotation         = Quaternion::Identity();
    m_fScale            = float3::One();
    m_mMatrix           = Matrix4::Identity();
    m_mInvMatrix        = Matrix4::Identity();
    m_bDirty            = false;
}
void Transform::CombineWithParent(Transform& parent)
{
    if ((m_eMatDecomposeStatus == MatDecomposeStatus::No || parent.m_eMatDecomposeStatus == MatDecomposeStatus::No) ||
        !IsUniformScale(parent.m_fScale))
    {
        if (!(m_eMatDecomposeStatus == MatDecomposeStatus::No))
            Update();
        m_mMatrix = m_mMatrix * parent.Matrix();
        m_mInvMatrix = m_mMatrix.Inverse();
        m_eMatDecomposeStatus = MatDecomposeStatus::No;
    }
    else
    {
        m_bDirty = true;
        m_fScale *= parent.m_fScale;
        m_qRotation = m_qRotation * parent.m_qRotation;

        m_fTranslation *= parent.m_fScale;
        m_fTranslation = Math::TransformQuaternion(m_fTranslation, parent.m_qRotation);
        m_fTranslation += parent.m_fTranslation;
    }
}

bool Transform::IsUniformScale(float3 const& scale) const
{
    static float threshold = 0.01;
    if (Math::Abs(scale[0] - scale[1]) > threshold || Math::Abs(scale[1] - scale[2]) > threshold)
        return false;
    else
        return true;
}

SEEK_NAMESPACE_END
