/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : spring_skeleton_component.h
**
**      Brief                  : spring skeleton component class
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2022-07-25  Created by Joe Lin
**
**************************************************************************************************/
#pragma once

#include "components/scene_component.h"
#include "math/transform.h"
#include "math/vector.h"
#include "math/math_utility.h"

SEEK_NAMESPACE_BEGIN

struct JointMass
{
    std::string         jointName = "";
    SceneComponentPtr   joint = nullptr;
    Transform           jointOriginLocalTransform;
    float               mass = 0.5f;
    bool                pinned = false;
    float3              startPosition;
    float3              currentPosition;
    float3              lastPosition;
    float3              velocity = float3(0.0f, 0.0f, 0.0f);
    float3              forces = float3(0.0f, 0.0f, 0.0f);
    float3              restPosition;
    float               restCoefficient = 0.05f;
    //float               restForceUpperLimit = 20.0f;
    float               dampingCoefficient = 0.05f;
    float               safeRadius = 0.0f;
};

struct Spring
{
    std::string         jointNameA = "";
    std::string         jointNameB = "";
    JointMass*          jointMassA = nullptr;
    JointMass*          jointMassB = nullptr;
    float               springCoefficient = 1.0f;
    float               restLength = 0.0f;
    //float               springForceUpperLimit = 20.0f;
};

struct SphereCollider
{
    float3              centerWorldPosition;
    float               radius = 0.0f;
    SceneComponentPtr   bindJoint = nullptr;
    float3              centerLocalPosition;
    float               frictionCoefficient = 1.0f;
};

class SpringSkeletonComponent : public Component
{
public:
    SpringSkeletonComponent(Context* context, std::string const& name);
    virtual ~SpringSkeletonComponent() {}

    void                        SetScaleMappingFactor(float factor) { m_fScaleMappingFactor = factor; };
    void                        SetRootJoint(SceneComponentPtr rootJoint) { m_pRootJoint = rootJoint; };
    void                        SetGravityAcceleration(float3 ga) { m_fGravityAcceleration = ga; };
    void                        SetIntervalLength(float intervalLength) { m_fIntervalLength = intervalLength; };
    void                        AddJointMass(std::string jointName, SceneComponentPtr joint, bool pinned, float restCoefficient, float dampingCoefficient);
    void                        AddSpring(std::string jointNameA, std::string jointNameB, float springCoefficient);
    virtual SResult           Tick(float delta_time) override;
    void                        Update(float delta_time);
    void                        AddColliders(SphereCollider& collider);
    void                        CollisionResponseMethod1();
    void                        CollisionResponseMethod2(SceneComponentPtr joint, int32_t massIdx);
    void                        ApplySkeleton(SceneComponentPtr joint, int32_t massIdx);
    void                        ResetStatus();
    void                        SetResetStatusFlag(bool resetFlag) { m_bResetStatusFlag = resetFlag; };
    void                        SetBypassTickFlag(bool bypassFlag) { m_bBypassTickFlag = bypassFlag; };

protected:
    std::vector<JointMass>      m_vJointMasses;
    std::vector<Spring>         m_vSprings;
    SceneComponentPtr           m_pRootJoint = nullptr;
    bool                        m_bIsExplicit = false;
    float                       m_fScaleMappingFactor = 1.0f;
    float3                      m_fGravityAcceleration = float3(0.0f, -0.0f, 0.0f);
    float                       m_fIntervalLength = 1.5f;
    int32_t                     m_iStepsPerFrame = 256;
    std::vector<SphereCollider> m_vColliders;
    bool                        m_bResetStatusFlag = false;
    bool                        m_bBypassTickFlag = false;
};

SEEK_NAMESPACE_END
