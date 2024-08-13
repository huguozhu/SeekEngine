/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : scene_component.h
**
**      Brief                     : scene_component
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-06-04  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "components/component.h"
#include "math/transform.h"

SEEK_NAMESPACE_BEGIN

class SceneComponent : public Component
{
public:
    SceneComponent(Context* context, std::string const& name = "SceneComponet", ComponentType type = ComponentType::Scene)
        : Component(context, name, type)
    {}
    virtual ~SceneComponent() {}

    /* ***************************************************************
     * Scene
     * ***************************************************************/
    SceneComponent*     GetParent() const { return m_pParent; }
    void                SetParent(SceneComponent* parent) { m_pParent = parent; }
    bool                HasChild(SceneComponentPtr const& child);
    void                AddChild(SceneComponentPtr const& child);
    void                DelChild(SceneComponentPtr const& child);
    std::vector<SceneComponentPtr>&     GetChildren() { return m_vChildren; }
    size_t              NumChildren();
    SceneComponentPtr   ChildByIndex(size_t index);
    void                GetAllComponentByType(std::vector<ComponentPtr>& components, ComponentType type);

    /* ***************************************************************
     * Transform...
     * ***************************************************************/
    // World
    Transform const&    GetWorldTransform();
    Matrix4   const&    GetWorldMatrix();
    Matrix4   const&    GetWorldMatrixInv();

    float3              GetWorldEulerRotation();
    void                SetWorldEulerRotation(float3 euler_rot);
    void                SetWorldTransform(Matrix4 const& matrix);
    void                SetWorldTranslation(float3 const& translation);
    void                SetWorldRotate(Quaternion const& rotate);
    void                SetWorldScale(float3 const& scale);
    void                WorldTranslate(float3 const& trans);
    void                WorldRotate(Quaternion const& rot);
    void                WorldScale(float3 const& scale);

    float3              GetWorldRightVec();
    float3              GetWorldUpVec();
    float3              GetWorldForwardVec();
    // Local
    Transform const&    GetLocalTransform();
    Matrix4   const&    GetLocalMatrix();
    void                SetLocalTransform(Transform const& transform);
    void                SetLocalTransform(Matrix4 const& matrix);
    void                SetLocalTransform(float3 const& translation, Quaternion const& rotation, float3 const& scale);
    void                SetLocalTranslation(float3 const& translation);
    void                SetLocalRotation(Quaternion const& rot);
    void                SetLocalScale(float3 const& scale);
    void                LocalTransform(Matrix4 const& matrix);
    void                LocalTransform(float3 const* scale_center, float3 const* scale, float3 const* rotate_center, Quaternion const* rotate, float3 const* trans);
    void                LocalTranslate(float3 const& trans);
    void                LocalRotate(Quaternion const& rot);
    void                LocalScale(float3 const& scale);

protected:
    virtual void        SetWorldDirty(); // set own and children's transform is dirty
    void                RefreshWorldBasedLocal(); // only refresh own world transform (not contain children), based parent's worldTransform and own local transform
    void                RefreshLocalBasedWorld(); // only refresh own local trasnform (not contain children), based parent's worldTransform and own world transform

protected:
    SceneComponent*                     m_pParent = nullptr;
    std::vector<SceneComponentPtr>      m_vChildren;

    mutable Transform                   m_cLocal; // Transform based parent coordinate system
    mutable Transform                   m_cWorld; // Transform based world  coordinate system
    bool                                m_bWorldDirty = true; // Own world transform and children's world transform are all dirty.
};

SEEK_NAMESPACE_END
