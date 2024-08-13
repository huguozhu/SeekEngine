#pragma once

#include "components/scene_component.h"
#include "math/frustum.h"

SEEK_NAMESPACE_BEGIN

static constexpr const float MIN_APERTURE = 0.5f;
static constexpr const float MAX_APERTURE = 64.0f;
static constexpr const float MIN_SHUTTER_SPEED = 1.0f / 25000.0f;
static constexpr const float MAX_SHUTTER_SPEED = 60.0f;
static constexpr const float MIN_SENSITIVITY = 10.0f;
static constexpr const float MAX_SENSITIVITY = 204800.0f;

enum class CameraProjectionType
{
    Perspective,
    Orthographic,

    Num,
};

class CameraComponent : public SceneComponent
{
public:
    CameraComponent(Context* context, CameraProjectionType type = CameraProjectionType::Perspective, std::string const& name = "CameraComponet");
    virtual ~CameraComponent() {}

    CameraProjectionType GetCameraProjectionType() const { return m_eCameraProjectionType; }

    void                SetLookAt(float3   position, float3   look_at, float3   up_vec);
    void                GetLookAt(float3 & position, float3 & look_at, float3 & up_vec);

    float               GetXMag()        const { return m_fMagX; }
    float               GetYMag()        const { return m_fMagY; }
    float               GetAspect()      const { return m_fAspect; }
    float               GetYFov()        const { return m_fYFov; }
    float               GetNearPlane()   const { return m_fNearPlane; }
    float               GetFarPlane()    const { return m_fFarPlane; }

    void                SetXMag(float v);
    void                SetYMag(float v);
    void                SetAspect(float v);
    void                SetYFov(float v);
    void                SetNearPlane(float v);
    void                SetFarPlane(float v);

    void                SetViewDirty(bool v) { m_bViewDirty = v; }

    void                ProjPerspectiveParams(float yfov, float aspect, float near_plane, float far_plane);
    void                ProjOrthographicParams(float width, float height, float near_plane, float far_plane);

    void                SetViewMatrix(Matrix4 mat);
    void                SetProjMatrix(Matrix4 mat);

    Matrix4 const&      GetViewMatrix();
    Matrix4 const&      GetProjMatrix();
    Matrix4 const&      GetViewProjMatrix();
    Matrix4 const&      GetInvViewMatrix();
    Matrix4 const&      GetInvProjMatrix();
    Matrix4 const&      GetInvViewProjMatrix();

    Frustum const&      GetFrustum();

    float               GetAperture()       const { return m_fAperture; }
    float               GetShutterSpeed()   const { return m_fShutterSpeed; }
    float               GetSensitivity()    const { return m_fSensitivity; }
    void                SetAperture         (float v);
    void                SetShutterSpeed     (float v);
    void                SetSensitivity      (float v);

    void                SetExposure(float exposure);
    void                SetExposure(float aperture, float shutter_speed, float sensitivity);
    float               GetExposure();
    float               GetEV100();

protected:
    void                UpdateViewProjMatrix();
    virtual void        SetWorldDirty() override;

protected:
    //
    float3                  m_fPosition;
    float3                  m_fLookAt;
    float3                  m_fUpVec;
    bool                    m_bLookAtDirty = false;

    CameraProjectionType    m_eCameraProjectionType = CameraProjectionType::Orthographic;

    // Orthographic
    float                   m_fMagX = 0.0f;
    float                   m_fMagY = 0.0f;

    // Perspecctive
    float                   m_fAspect = 0.0f;
    float                   m_fYFov = 0.0f;

    // Both
    float                   m_fNearPlane = 0.0f;
    float                   m_fFarPlane = 0.0f;

    mutable Matrix4         m_mView         = Matrix4::Identity();
    mutable Matrix4         m_mProj         = Matrix4::Identity();
    mutable Matrix4         m_mViewProj     = Matrix4::Identity();
    mutable Matrix4         m_mInvView      = Matrix4::Identity();
    mutable Matrix4         m_mInvProj      = Matrix4::Identity();
    mutable Matrix4         m_mInvViewProj  = Matrix4::Identity();

    bool                    m_bViewDirty = false;
    bool                    m_bProjDirty = false;
    bool                    m_bViewProjDirty = false;

    bool                    m_bUseDirectViewMatrix = false;
    bool                    m_bUseDirectProjMatrix = false;

    Frustum                 m_cFrustum;
    bool                    m_bFrustumDirty = true;

    // exposure settings, Sunny 16
    float                   m_fAperture = 16.0f;
    float                   m_fShutterSpeed = float(1.0f / 125.0f);
    float                   m_fSensitivity = 100.0f;
};

SEEK_NAMESPACE_END
