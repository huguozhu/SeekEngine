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
}
DirectionalLightComponent::~DirectionalLightComponent()
{
}
/******************************************************************************
 * SpotLightComponent
 ******************************************************************************/
SpotLightComponent::SpotLightComponent(Context* context, std::string const& name)
    : LightComponent(context, LightType::Spot, name)
{
}
SpotLightComponent::~SpotLightComponent()
{
}
void SpotLightComponent::SetInOutCutoff(float2 cutoff)
{
    m_fInOutCutoff = cutoff;
}
/******************************************************************************
 * PointLightComponent
 ******************************************************************************/
PointLightComponent::PointLightComponent(Context* context, std::string const& name)
    : LightComponent(context, LightType::Point, name)
{    
}
PointLightComponent::~PointLightComponent()
{
    
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
