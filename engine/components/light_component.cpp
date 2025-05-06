#include "components/light_component.h"
#include "components/camera_component.h"
#include "rhi/base/rhi_texture.h"
#include "math/math_utility.h"
#include "utils/log.h"
#include <cmath>

#define SEEK_MACRO_FILE_UID 32     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

#define DEFAULT_SM_CAMERA_NEAR 0.1
#define DEFAULT_SM_CAMERA_FAR  100

/******************************************************************************
 * Light
 ******************************************************************************/
LightComponent::LightComponent(Context* context, LightType type, std::string const& name /* = "LightComponent" */)
    : SceneComponent(context, name, ComponentType::Light), m_eLightType(type)
{
    switch (type)
    {
        case LightType::Num:
        case LightType::Unknown:        m_sLightTypeStr = "unknown";        break;
        case LightType::Ambient:        m_sLightTypeStr = "ambient";        break;
        case LightType::Directional:    m_sLightTypeStr = "directional";    break;
        case LightType::Point:          m_sLightTypeStr = "point";          break;
        case LightType::Spot:           m_sLightTypeStr = "spot";           break;
    }
}
LightComponent::~LightComponent()
{

}
LightComponentPtr LightComponent::CreateLightComponent(Context* context, LightType lightType, std::string const& name)
{
    if (context == nullptr || uint32_t(lightType) >= uint32_t(LightType::Num) || lightType == LightType::Unknown)
    {
        LOG_ERROR("LightComponent::CreateLightComponent invalid param");
        return nullptr;
    }
    LightComponentPtr pLightComponent = nullptr;
    switch (lightType)
    {
    case LightType::Ambient:
        pLightComponent = std::static_pointer_cast<LightComponent>(MakeSharedPtr<AmbientLightComponent>(context));
        break;
    case LightType::Directional:
        pLightComponent = std::static_pointer_cast<LightComponent>(MakeSharedPtr<DirectionalLightComponent>(context));
        break;
    case LightType::Spot:
        pLightComponent = std::static_pointer_cast<LightComponent>(MakeSharedPtr<SpotLightComponent>(context));
        break;
    case LightType::Point:
        pLightComponent = std::static_pointer_cast<LightComponent>(MakeSharedPtr<PointLightComponent>(context));
        break;
    case LightType::Unknown:
    case LightType::Num:
        break;
    }
    pLightComponent->SetName(name);
    return pLightComponent;
}
void LightComponent::CastShadow(bool v)
{
    m_iAttrib |= v ? (m_iAttrib | LightAttrib::CastShadow) : (m_iAttrib & !(LightAttrib::CastShadow));
}
bool LightComponent::CastShadow()
{
    return m_iAttrib & LightAttrib::CastShadow;
}
void LightComponent::SoftShadow(bool v)
{
    m_iAttrib |= v ? (m_iAttrib | LightAttrib::SoftShadow) : (m_iAttrib & !(LightAttrib::SoftShadow));
}
bool LightComponent::SoftShadow()
{
    return m_iAttrib & LightAttrib::SoftShadow;
}
void LightComponent::CascadedShadow(bool v)
{
    m_iAttrib |= v ? (m_iAttrib | LightAttrib::CascadedShadow) : (m_iAttrib & !(LightAttrib::CascadedShadow));
}
bool LightComponent::CascadedShadow()
{
    return m_iAttrib & LightAttrib::CascadedShadow;
}
void LightComponent::IndirectLighting(bool v)
{
    m_iAttrib |= v ? (m_iAttrib | LightAttrib::IndirectLighting) : (m_iAttrib & !(LightAttrib::IndirectLighting));
}
bool LightComponent::IndirectLighting()
{
    return m_iAttrib & LightAttrib::IndirectLighting;
}
float3 LightComponent::GetDirection()
{
    return GetWorldForwardVec();
}
float3 LightComponent::GetDirectionOrigin()
{
    if (Math::Length(m_fDirectionOrigin) < 0.01)
    {
        m_fDirectionOrigin = GetDirection();
    }
    return m_fDirectionOrigin;
}
void LightComponent::SetDirection(float3 direction)
{
    m_fDirectionOrigin = direction;

    Matrix4 inv_view = Math::LookAtLH(float3(0.0), direction, float3(0, 1, 0)).Inverse();
    float3 scale;
    Quaternion rot;
    float3 trans;
    Math::TransformDecompose(scale, rot, trans, inv_view);
    if (std::isnan(rot[0]))
    {
        Matrix4 inv_view = Math::LookAtLH(float3(0.0), direction, float3(0, 0, 1)).Inverse();
        Math::TransformDecompose(scale, rot, trans, inv_view);
    }

    this->SetWorldRotate(rot);
}
float3 LightComponent::GetLightPos()
{
    return this->GetWorldTransform().GetTranslation();
}
void LightComponent::SetLightPos(float3 pos)
{
    this->SetWorldTranslation(pos);
}
Quaternion LightComponent::GetRotation()
{
    float3 scale;
    Quaternion rot;
    float3 trans;
    Math::TransformDecompose(scale, rot, trans, this->GetWorldMatrix());
    return rot;
}
void LightComponent::SetIntensity(float intensity, LightIntensityUnit unit)
{
    float luminous_power = intensity;
    float luminous_intensity = 0;
    switch (m_eLightType)
    {
    case LightType::Directional:
    {
        luminous_intensity = luminous_power;
        break;
    }
    case LightType::Spot:
    {
        if (unit == LightIntensityUnit::Lumen_Lux)
        {
            // li = lp / (2 * pi * (1 - cos(out)))
            SpotLightComponent* sl = (SpotLightComponent*)this;
            float out_cutoff = sl->GetInOutCutoff().y();
            luminous_intensity = luminous_power / (Math::PI2 * (1.0 - cos(out_cutoff)));
        }
        else if (unit == LightIntensityUnit::Candela)
        {
            luminous_intensity = luminous_power;
        }
        break;
    }
    case LightType::Point:
    {
        if (unit == LightIntensityUnit::Lumen_Lux)
        {
            // li = lp / (4*pi)
            luminous_intensity = luminous_power * 0.25 / Math::PI;
        }
        else
            luminous_intensity = luminous_power;
        break;
    }
    case LightType::Ambient:
    case LightType::Unknown:
    case LightType::Num:
        break;
    }
    m_fIntensity = luminous_intensity;
    m_fIntensityOrigin = intensity;
}

/******************************************************************************
 * AmbientLightComponent
 ******************************************************************************/
AmbientLightComponent::AmbientLightComponent(Context* context, std::string const& name)
    : LightComponent(context, LightType::Ambient, name)
{
}
AmbientLightComponent::~AmbientLightComponent()
{
}

/******************************************************************************
 * DirectionalLightComponent
 ******************************************************************************/
DirectionalLightComponent::DirectionalLightComponent(Context* context, std::string const& name)
    : LightComponent(context, LightType::Directional, name)
{
    m_pSMCamera = MakeSharedPtr<LightCameraComponent>(context);
    m_pSMCamera->ProjOrthographicParams(10, 10, DEFAULT_SM_CAMERA_NEAR, DEFAULT_SM_CAMERA_FAR);
    m_pSMCamera->SetName("Directional SM Camera");
    m_pSMCamera->SetParent(this);
    this->AddChild(m_pSMCamera);
}
DirectionalLightComponent::~DirectionalLightComponent()
{
    if (m_pSMCamera)
        m_pSMCamera->SetParent(nullptr);
    //if (m_pCsmCamera)
    //    m_pCsmCamera->SetParent(nullptr);
}
void DirectionalLightComponent::CascadedShadow(bool v)
{
    LightComponent::CascadedShadow(v);
    //if (v && !m_pCsmCamera)
    //{
    //    // only deferred shading use CSM
    //    m_pCsmCamera = MakeSharedPtr<CsmCameraComponent>(m_pContext);
    //    m_pCsmCamera->SetParam(10, 10, DEFAULT_SM_CAMERA_NEAR, DEFAULT_SM_CAMERA_FAR);
    //    m_pCsmCamera->SetParent(this);
    //    this->AddChild(m_pCsmCamera);
    //}
}
CameraComponent* DirectionalLightComponent::GetShadowMapCamera(size_t)
{
    return m_pSMCamera.get();
}
/******************************************************************************
 * SpotLightComponent
 ******************************************************************************/
SpotLightComponent::SpotLightComponent(Context* context, std::string const& name)
    : LightComponent(context, LightType::Spot, name)
{
    m_pSMCamera = MakeSharedPtr<LightCameraComponent>(context);
    m_pSMCamera->ProjPerspectiveParams(m_fInOutCutoff[1] * 2.0, 1.0, DEFAULT_SM_CAMERA_NEAR, DEFAULT_SM_CAMERA_FAR);
    m_pSMCamera->SetName("Spot SM Camera");
    m_pSMCamera->SetParent(this);
    this->AddChild(m_pSMCamera);
}
SpotLightComponent::~SpotLightComponent()
{
    if (m_pSMCamera)
        m_pSMCamera->SetParent(nullptr);
}
void SpotLightComponent::SetInOutCutoff(float2 cutoff)
{
    m_fInOutCutoff = cutoff;
    if (m_pSMCamera)
    {
        m_pSMCamera->ProjPerspectiveParams(cutoff[1] * 2.0, 1.0, DEFAULT_SM_CAMERA_NEAR, DEFAULT_SM_CAMERA_FAR);
    }
}
CameraComponent* SpotLightComponent::GetShadowMapCamera(size_t)
{
    return m_pSMCamera.get();
}
/******************************************************************************
 * PointLightComponent
 ******************************************************************************/
static std::vector<float3> lookat_offset = {
        float3(1.0f,   0.0f,  0.0f),
        float3(-1.0f,  0.0f,  0.0f),
        float3(0.0f,   1.0f,  0.0f),
        float3(0.0f,  -1.0f,  0.0f),
        float3(0.0f,   0.0f,  1.0f),
        float3(0.0f,   0.0f, -1.0f),
};
static std::vector<float3> up = {
    float3(0.0f,  1.0f,  0.0),
    float3(0.0f,  1.0f,  0.0),
    float3(0.0f,  0.0f,  1.0),
    float3(0.0f,  0.0f,  1.0),
    float3(0.0f,  1.0f,  0.0),
    float3(0.0f,  1.0f,  0.0),
};
PointLightComponent::PointLightComponent(Context* context, std::string const& name)
    : LightComponent(context, LightType::Point, name)
{    
    m_fShadowBias = 0.05;
    Matrix4 proj = Math::PerspectiveLH(90.0 * Math::DEG2RAD, 1.0, DEFAULT_SM_CAMERA_NEAR, DEFAULT_SM_CAMERA_FAR);
    for (uint32_t face = (uint32_t)CubeFaceType::Positive_X; face < (uint32_t)CubeFaceType::Num; face++)
    {
        m_aSMCameras[face] = MakeSharedPtr<LightCameraComponent>(context);
        float3 pos = this->GetWorldTransform().GetTranslation();
        Matrix4 view = Math::LookAtLH(pos, pos + lookat_offset[face], up[face]);
        m_aSMCameras[face]->SetViewMatrix(view);
        m_aSMCameras[face]->SetProjMatrix(proj);
        m_aSMCameras[face]->SetName("Point SM Camera_" + std::to_string(face));
        m_aSMCameras[face]->SetNearPlane(DEFAULT_SM_CAMERA_NEAR);
        m_aSMCameras[face]->SetFarPlane(DEFAULT_SM_CAMERA_FAR);

        m_aSMCameras[face]->SetParent(this);
        this->AddChild(m_aSMCameras[face]);
    }
}
PointLightComponent::~PointLightComponent()
{
    for (uint32_t i = 0; i < 6; i++)
    {
        m_aSMCameras[i]->SetParent(nullptr);
    }
}

void PointLightComponent::UpdateShadowMapCamera()
{
    //    if (m_bWorldDirty)
    {
        //m_bWorldDirty = false;
        for (uint32_t face = (uint32_t)CubeFaceType::Positive_X; face < (uint32_t)CubeFaceType::Num; face++)
        {
            float3 pos = this->GetWorldTransform().GetTranslation();
            Matrix4 view = Math::LookAtLH(pos, pos + lookat_offset[face], up[face]);
            m_aSMCameras[face]->SetViewMatrix(view);
        }
    }

}
CameraComponent* PointLightComponent::GetShadowMapCamera(size_t index)
{
    if (index > 5)
        return nullptr;
    return m_aSMCameras[index].get();
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
