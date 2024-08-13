/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : component.h
**
**      Brief                  : component base class
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-06-04  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

enum class ComponentType : uint8_t
{
    Unknown,
    Scene,
    Camera,
    Light,
    Mesh,
    SkeletalMesh,
    SkyBox,
    Sprite2D,
    Image,
    Animation,
    SpringSkeleton,
    ParticleSystem,
};

class Component
{
public:
    std::string const&      GetName() const { return m_szName; }
    void                    SetName(std::string const& name) { m_szName = name; }

    Entity*                 GetOwner() const { return m_pOwner; }
    void                    SetOwner(Entity* entity) { m_pOwner = entity; }

    ComponentType           GetComponentType() const { return m_eComponentType; }

    virtual                 ~Component() {}
    virtual SResult       Tick(float delta_time) { return S_Success; }

protected:
    Component(Context* context, std::string const& name = "UnknownComponent", ComponentType type = ComponentType::Unknown)
        : m_pContext(context)
        , m_szName(name)
        , m_eComponentType(type)
    {}

protected:
    Context*                m_pContext = nullptr;
    std::string             m_szName;
    ComponentType           m_eComponentType = ComponentType::Unknown;
    Entity*                 m_pOwner = nullptr;
};

SEEK_NAMESPACE_END
