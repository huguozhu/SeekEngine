/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : light_component.h
**
**      Brief                     : light component base class
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-06-07  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "components/scene_component.h"
#include "math/vector.h"
#include "math/color.h"
#include "math/math_utility.h"

SEEK_NAMESPACE_BEGIN

enum class LightType : uint32_t
{
    Unknown     = 0xffffffff,
    Ambient     = 0x00,
    Directional,
    Spot,
    Point,

    Num,
};
enum class LightIntensityUnit : uint32_t
{
    Lumen_Lux,      // intensity specified in lumens (for punctual lights) or lux (for directional)
                    // Lumen(luminous flux):    w (φ lm)
                    // Lux(illuminance):        w/m2 (E lx)
    Candela,        // intensity specified in candela (only applicable to punctual lights, spot/point light)
                    // w/sr
};

enum LightAttrib : uint32_t
{
    IndirectLighting    = 1UL << 2,
};

class LightComponent : public SceneComponent
{
public:
    virtual ~LightComponent();
    static LightComponentPtr CreateLightComponent(Context* context, LightType lightType, std::string const& name = "LightComponent");

    LightType           GetLightType() const { return m_eLightType; }
    std::string const&  GetLightTypeString() const { return m_sLightTypeStr; }

    void            SetEnable(bool v) { m_bIsEnable = v; }
    bool            IsEnable() { return m_bIsEnable; }

    void            IndirectLighting(bool v);
    bool            IndirectLighting();

    Color           GetColor() { return m_cColor; }
    void            SetColor(Color c) { m_cColor = c; }

    float3          GetDirection();
    float3          GetDirectionOrigin();
    void            SetDirection(float3 direction);

    float3          GetLightPos();
    void            SetLightPos(float3 pos);
    Quaternion      GetRotation();

    void            SetIntensity(float intensity, LightIntensityUnit unit = LightIntensityUnit::Lumen_Lux);
    float           GetIntensity()       const { return m_fIntensity; }
    float           GetIntensityOrigin() const { return m_fIntensityOrigin; }

    void            SetFalloffRadius(float v) { m_fFalloffRadius = v; }
    float           GetFalloffRadius() { return m_fFalloffRadius; }

    virtual void    SetInOutCutoff(float2 cutoff) {}

protected:
    LightComponent(Context* context, LightType lightType, std::string const& name = "LightComponent");

    LightType           m_eLightType = LightType::Unknown;
    std::string         m_sLightTypeStr;
    Color               m_cColor = Color::White;
    uint32_t            m_iAttrib = 0;
    bool                m_bIsEnable = true;
    float               m_fFalloffRadius = 10.0;
    float               m_fIntensity = 1.0;


    float               m_fIntensityOrigin = 1.0;
    float3              m_fDirectionOrigin = float3(0.0);
};

/******************************************************************************
 * AmbientLightComponent
 ******************************************************************************/
class AmbientLightComponent : public LightComponent
{
public:
    AmbientLightComponent(Context* context, std::string const& name = "AmbientLightComponent");
    virtual ~AmbientLightComponent();
    float GetAmbientFactor() const { return m_fAmbientFactor; }

private:
    float m_fAmbientFactor = 0.1f;       // default ambient factor = 0.1
};

/******************************************************************************
 * DirectionalLightComponent
 ******************************************************************************/
class DirectionalLightComponent : public LightComponent
{
public:
    DirectionalLightComponent(Context* context, std::string const& name = "DirectionalLightComponent");
    virtual ~DirectionalLightComponent();
};

/******************************************************************************
 * SpotLightComponent
 ******************************************************************************/
class SpotLightComponent : public LightComponent
{
public:
    SpotLightComponent(Context* context, std::string const& name = "SpotLightComponent");
    virtual ~SpotLightComponent();

    virtual  void SetInOutCutoff(float2 cutoff) override;
    float2 GetInOutCutoff() { return m_fInOutCutoff; }

protected:
    float2              m_fInOutCutoff = float2(float(Math::PI * 0.3f), float(Math::PI * 0.375f));
};

/******************************************************************************
 * PointLightComponent
 ******************************************************************************/
class PointLightComponent : public LightComponent
{
public:
    PointLightComponent(Context* context, std::string const& name = "PointLightComponent");
    virtual ~PointLightComponent();
};
SEEK_NAMESPACE_END