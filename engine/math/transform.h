#pragma once

#include "kernel/kernel.h"
#include "math/matrix.h"
#include "math/quaternion.h"

SEEK_NAMESPACE_BEGIN

enum class MatDecomposeStatus
{
    Unknown,
    Yes,
    No,
};

class Transform
{
public:
    Transform();
    Transform(float3 const& translation, Quaternion const& quat, float3 const& scale);

    //inline bool         operator==(Transform const& rhs) const;
    //inline bool         operator!=(Transform const& rhs) const;
    float3      const&  GetTranslation()    const { return m_fTranslation; }
    Quaternion  const&  GetRotation()       const { return m_qRotation; }
    float3      const&  GetScale()          const { return m_fScale; }
    MatDecomposeStatus  GetMatDecomposeStatus() const { return m_eMatDecomposeStatus; }

    void SetTranslation(float3 const& translation);
    void SetTranslation(float x, float y, float z);
    void SetScale(float3 const& scale);
    void SetScale(float x, float y, float z);
    void SetRotation(Quaternion const& rotation);

    void Translate(float3 const& translation);
    void Translate(float x, float y, float z);
    void Scale(float3 const& scale);
    void Scale(float scale);
    void Scale(float x, float y, float z);
    void Rotate(Quaternion const& rotation);

    Matrix4 const& Matrix();
    Matrix4 const& InvMatrix();

    void Set(float3 const& translation, Quaternion const& rot, float3 const& scale);
    void Set(Matrix4 const& matrix);
    void SetIdentity();

    void CombineWithParent(Transform& parent);

private:
    void Update();
    bool CheckMatCanbeDecompose(Matrix4 const& m, float3 const& t, Quaternion const& r, float3 const& s) const;
    bool IsUniformScale(float3 const& scale) const;

private:
    float3              m_fTranslation          = float3::Zero();
    float3              m_fScale                = float3::One();
    Quaternion          m_qRotation             = Quaternion::Identity();
    bool                m_bDirty                = false;

    Matrix4             m_mMatrix               = Matrix4::Identity();
    Matrix4             m_mInvMatrix            = Matrix4::Identity();
    MatDecomposeStatus  m_eMatDecomposeStatus   = MatDecomposeStatus::Unknown;
};

SEEK_NAMESPACE_END
