#include "components/camera_component.h"
#include "math/math_utility.h"
#include "utils/exposure.h"
#include <cmath>

#define SEEK_MACRO_FILE_UID 34     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

CameraComponent::CameraComponent(Context* context, CameraProjectionType type, std::string const& name)
    : SceneComponent(context, name, ComponentType::Camera)
    , m_eCameraProjectionType(type)
{
}

void CameraComponent::SetLookAt(float3   position, float3   look_at, float3   up_vec)
{
    m_fPosition = position;
    m_fLookAt = look_at;
    m_fUpVec = Math::Normalize(up_vec);

    Matrix4 inv_mat = Math::LookAtLH(position, look_at, up_vec).Inverse();
    //m_pRootComponent->SetWorldTransform(inv_mat);
    this->SetWorldTransform(inv_mat);

    m_bLookAtDirty = false;
}

void CameraComponent::GetLookAt(float3 & position, float3 & look_at, float3 & up_vec)
{
    if (m_bLookAtDirty)
    {
        float3 _position, _look_at, _up_vec;
        Math::ToLookAtLH(m_mView, _position, _look_at, _up_vec);

        if (!std::isnan(_position[0]) && !std::isnan(_look_at[0]) && !std::isnan(_up_vec[0]))
        {
            m_bLookAtDirty = false;
            m_fPosition = _position;
            m_fLookAt = _look_at;
            m_fUpVec = _up_vec;
        }
    }
    position = m_fPosition;
    look_at = m_fLookAt;
    up_vec = m_fUpVec;
}

void CameraComponent::SetXMag(float v)
{
    if (m_fMagX != v)
    {
        m_fMagX = v;
        m_bProjDirty = true;
    }
}
void CameraComponent::SetYMag(float v)
{
    if (m_fMagY != v)
    {
        m_fMagY = v;
        m_bProjDirty = true;
    }
}
void CameraComponent::SetAspect(float v)
{
    if (m_fAspect != v)
    {
        m_fAspect = v;
        m_bProjDirty = true;
    }
}
void CameraComponent::SetYFov(float v)
{
    if (m_fYFov != v)
    {
        m_fYFov = v;
        m_bProjDirty = true;
    }
}
void CameraComponent::SetNearPlane(float v)
{
    if (m_fNearPlane != v)
    {
        m_fNearPlane = v;
        m_bProjDirty = true;
    }
}
void CameraComponent::SetFarPlane(float v)
{
    if (m_fFarPlane != v)
    {
        m_fFarPlane = v;
        m_bProjDirty = true;
    }
}

void CameraComponent::ProjPerspectiveParams(float yfov, float aspect, float near_plane, float far_plane)
{
    m_eCameraProjectionType = CameraProjectionType::Perspective;
    m_fYFov = yfov;
    m_fAspect = aspect;
    m_fNearPlane = near_plane;
    m_fFarPlane = far_plane;

    m_mProj = Math::PerspectiveLH(yfov, aspect, near_plane, far_plane);
    m_mInvProj = m_mProj.Inverse();
    m_bProjDirty = false;
    m_bViewProjDirty = true;
    m_bUseDirectProjMatrix = false;
}

void CameraComponent::ProjOrthographicParams(float width, float height, float near_plane, float far_plane)
{
    m_eCameraProjectionType = CameraProjectionType::Orthographic;
    m_fAspect = width / height;
    m_fMagX = width;
    m_fMagY = height;
    m_fNearPlane = near_plane;
    m_fFarPlane = far_plane;

    m_mProj = Math::OrthographicLH(width, height, near_plane, far_plane);
    m_mInvProj = m_mProj.Inverse();
    m_bProjDirty = false;
    m_bViewProjDirty = true;
    m_bUseDirectProjMatrix = false;
}

void CameraComponent::SetViewMatrix(Matrix4 mat)
{
    m_mView = mat;
    m_mInvView = mat.Inverse();
    m_bViewDirty = false;
    m_bViewProjDirty = true;
    m_bFrustumDirty = true;
    m_bLookAtDirty = true;
    m_bUseDirectViewMatrix = true;
}
void CameraComponent::SetProjMatrix(Matrix4 mat)
{
    m_mProj = mat;
    m_mInvProj = mat.Inverse();
    m_bProjDirty = false;
    m_bViewProjDirty = true;
    m_bFrustumDirty = true;
    m_bUseDirectProjMatrix = true;
}
Matrix4 const& CameraComponent::GetViewMatrix()
{
    UpdateViewProjMatrix();
    return m_mView;
}
Matrix4 const& CameraComponent::GetProjMatrix()
{
    UpdateViewProjMatrix();
    return m_mProj;
}
Matrix4 const& CameraComponent::GetViewProjMatrix()
{
    UpdateViewProjMatrix();
    return m_mViewProj;
}
Matrix4 const& CameraComponent::GetInvViewMatrix()
{
    UpdateViewProjMatrix();
    return m_mInvView;
}
Matrix4 const& CameraComponent::GetInvProjMatrix()
{
    UpdateViewProjMatrix();
    return m_mInvProj;
}
Matrix4 const& CameraComponent::GetInvViewProjMatrix()
{
    UpdateViewProjMatrix();
    return m_mInvViewProj;
}
Frustum const& CameraComponent::GetFrustum()
{
    if (m_bFrustumDirty)
    {
        m_cFrustum.ClipMatrix(GetViewProjMatrix(), GetInvViewProjMatrix());
        m_bFrustumDirty = false;
    }
    return m_cFrustum;
}
void CameraComponent::SetExposure(float exposure)
{
    this->SetExposure(1.0f, 1.2f, 100.0f * (1.0 / exposure));
}
void CameraComponent::SetExposure(float aperture, float shutter_speed, float sensitivity)
{
    m_fAperture     = Math::Clamp(aperture, MIN_APERTURE, MAX_APERTURE);
    m_fShutterSpeed = Math::Clamp(shutter_speed, MIN_SHUTTER_SPEED, MAX_SHUTTER_SPEED);
    m_fSensitivity  = Math::Clamp(sensitivity, MIN_SENSITIVITY, MAX_SENSITIVITY);
}
void CameraComponent::SetAperture(float v)
{
    m_fAperture     = Math::Clamp(v, MIN_APERTURE, MAX_APERTURE);
}
void CameraComponent::SetShutterSpeed(float v)
{
    m_fShutterSpeed = Math::Clamp(v, MIN_SHUTTER_SPEED, MAX_SHUTTER_SPEED);
}
void CameraComponent::SetSensitivity(float v)
{
    m_fSensitivity  = Math::Clamp(v, MIN_SENSITIVITY, MAX_SENSITIVITY);
}
float CameraComponent::GetExposure()
{
    // float exposure = Exposure::Exposure(m_fAperture, m_fShutterSpeed, m_fSensitivity);
    // float ev100 = Exposure::EV100(m_fAperture, m_fShutterSpeed, m_fSensitivity);
    // float lumiance = Exposure::Luminance(m_fAperture, m_fShutterSpeed, m_fSensitivity);
    // float illumiance = Exposure::Illuminance(m_fAperture, m_fShutterSpeed, m_fSensitivity);
    return Exposure::Exposure(m_fAperture, m_fShutterSpeed, m_fSensitivity);
}
float CameraComponent::GetEV100()
{
    return Exposure::EV100(m_fAperture, m_fShutterSpeed, m_fSensitivity);
}
void CameraComponent::UpdateViewProjMatrix()
{
    // update view matrix
    if ((m_bViewDirty || m_bWorldDirty) && !m_bUseDirectViewMatrix)
    {
        Matrix4 mat = GetWorldMatrix();
        m_mView = mat.Inverse();
        m_mInvView = mat;
        m_bViewDirty = false;
        m_bViewProjDirty = true;
        m_bFrustumDirty = true;
        m_bLookAtDirty = true;
    }
    // update proj matrix
    if (m_bProjDirty && !m_bUseDirectProjMatrix)
    {
        if (m_eCameraProjectionType == CameraProjectionType::Perspective)
            this->ProjPerspectiveParams(m_fYFov, m_fAspect, m_fNearPlane, m_fFarPlane);
        else
            this->ProjOrthographicParams(m_fMagX, m_fMagY, m_fNearPlane, m_fFarPlane);
        m_bProjDirty = false;
        m_bViewProjDirty = true;
        m_bFrustumDirty = true;
    }
    // update view*proj matrix
    if (m_bViewProjDirty)
    {
        m_mViewProj = m_mView * m_mProj;
        m_mInvViewProj = m_mViewProj.Inverse();
        m_bViewProjDirty = false;
        m_bFrustumDirty = true;
    }
}
void CameraComponent::SetWorldDirty()
{
    SceneComponent::SetWorldDirty();
    m_bFrustumDirty = true;
    m_bViewDirty = true;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
