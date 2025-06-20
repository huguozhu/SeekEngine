#include "components/scene_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "kernel/context.h"
#include "math/math_utility.h"

#define SEEK_MACRO_FILE_UID 37     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

void SceneComponent::SetWorldDirty()
{
    m_bWorldDirty = true;

    m_pContext->SceneManagerInstance().SetSceneDirty(true);

    if (m_eComponentType == ComponentType::Camera)
        ((CameraComponent*)(this))->SetViewDirty(true);

    for (auto& child : m_vChildren)
        child->SetWorldDirty();
}

/* ***************************************************************
 * Scene
 * ***************************************************************/
bool SceneComponent::HasChild(SceneComponentPtr const& child)
{
    auto iter = std::find(m_vChildren.begin(), m_vChildren.end(), child);
    return iter != m_vChildren.end();
}

void SceneComponent::AddChild(SceneComponentPtr const& child)
{
    if (!HasChild(child))
    {
        child->SetParent(this);
        m_vChildren.push_back(child);
        m_pContext->SceneManagerInstance().SetSceneDirty(true);
    }
}

void SceneComponent::DelChild(SceneComponentPtr const& child)
{
    auto iter = std::find(m_vChildren.begin(), m_vChildren.end(), child);
    if (iter != m_vChildren.end())
    {
        m_vChildren.erase(iter);
        m_pContext->SceneManagerInstance().SetSceneDirty(true);
    }
}

size_t SceneComponent::NumChildren()
{
    return m_vChildren.size();
}

SceneComponentPtr SceneComponent::ChildByIndex(size_t index)
{
    if (index >= NumChildren())
        return nullptr;
    return m_vChildren[index];
}
void SceneComponent::GetAllComponentByType(std::vector<ComponentPtr>& components, ComponentType type)
{
    for (SceneComponentPtr m : m_vChildren)
    {
        if (type == m->GetComponentType())
            components.push_back(m);
        m->GetAllComponentByType(components, type);
    }
}
/* ***************************************************************
 * Transform...
 * ***************************************************************/
void SceneComponent::RefreshWorldBasedLocal()
{
    if (!m_bWorldDirty) return;
    m_bWorldDirty = false;

    m_cWorld = m_cLocal;
    if (m_pParent)
    {
        Transform trans = m_pParent->GetWorldTransform();
        m_cWorld.CombineWithParent(trans);
    }
}

void SceneComponent::RefreshLocalBasedWorld()
{
    RefreshWorldBasedLocal();

    if (m_pParent)
    {
        Matrix4 mat = m_cWorld.Matrix() * m_pParent->GetWorldMatrixInv();
        m_cLocal.Set(mat);
    }
    else
    {
        m_cLocal = m_cWorld;
    }
}

Transform const& SceneComponent::GetWorldTransform()
{
    RefreshWorldBasedLocal();
    return m_cWorld;
}

Matrix4 const& SceneComponent::GetWorldMatrix()
{
    RefreshWorldBasedLocal();
    return m_cWorld.Matrix();
}

Matrix4 const& SceneComponent::GetWorldMatrixInv()
{
    RefreshWorldBasedLocal();
    return m_cWorld.InvMatrix();
}
float3 SceneComponent::GetWorldEulerRotation()
{
    if (m_bWorldDirty)
        RefreshWorldBasedLocal();
    Quaternion const& rot = m_cWorld.GetRotation();

    float sinX = 2 * (rot.w() * rot.x() - rot.y() * rot.z());
    float sinY_cosX = 2 * (rot.w() * rot.y() + rot.x() * rot.z());
    float cosY_cosX = 1 - 2 * (rot.x() * rot.x() + rot.y() * rot.y());
    float sinZ_cosX = 2 * (rot.w() * rot.z() + rot.x() * rot.y());
    float cosZ_cosX = 1 - 2 * (rot.x() * rot.x() + rot.z() * rot.z());

    float3 rotation;
    if (fabs(sinX) >= 1.0f)
        rotation[0] = copysignf(Math::PI / 2, sinX);
    else
        rotation[0] = asinf(sinX);
    rotation[1] = atan2f(sinY_cosX, cosY_cosX);
    rotation[2] = atan2f(sinZ_cosX, cosZ_cosX);

    return rotation;
}
void SceneComponent::SetWorldEulerRotation(float3 euler_rot)
{
    SetWorldDirty();
    Quaternion q = Math::FromPitchYawRoll(euler_rot[0], euler_rot[1], euler_rot[2]);
    RefreshWorldBasedLocal();
    m_cWorld.SetRotation(q);
    RefreshLocalBasedWorld();
}
void SceneComponent::SetWorldTransform(Matrix4 const& matrix)
{
    SetWorldDirty();
    Matrix4 mat = matrix;
    if (m_pParent)
        mat *= m_pParent->GetWorldMatrixInv();
    SetLocalTransform(mat);
}

void SceneComponent::SetWorldTranslation(float3 const& translation)
{
    SetWorldDirty();
    Matrix4 mat = Math::Transform(translation, m_cLocal.GetRotation(), m_cLocal.GetScale());
    if (m_pParent)
        mat *= m_pParent->GetWorldMatrixInv();
    float3 pos, scale;
    Quaternion qua;
    Math::TransformDecompose(scale, qua, pos, mat);
    SetLocalTranslation(pos);
}

void SceneComponent::SetWorldRotate(Quaternion const& rotate)
{
    SetWorldDirty();
    Matrix4 const& mat = GetWorldMatrix();
    float3 pos, scale;
    Quaternion qua;
    Math::TransformDecompose(scale, qua, pos, mat);
    Matrix4 new_mat = Math::Transform(pos, rotate, scale);
    SetWorldTransform(new_mat);
}
void SceneComponent::SetWorldScale(float3 const& scale)
{
    SetWorldDirty();
    Matrix4 const& mat = GetWorldMatrix();
    float3 pos, s;
    Quaternion qua;
    Math::TransformDecompose(s, qua, pos, mat);
    Matrix4 new_mat = Math::Transform(pos, qua, s);
    SetWorldTransform(new_mat);
}
void SceneComponent::WorldTranslate(float3 const& trans)
{
    SetWorldDirty();
    RefreshWorldBasedLocal();
    m_cWorld.Translate(trans);
    RefreshLocalBasedWorld();
}

void SceneComponent::WorldRotate(Quaternion const& rot)
{
    SetWorldDirty();
    RefreshWorldBasedLocal();
    m_cWorld.Rotate(rot);
    RefreshLocalBasedWorld();
}

void SceneComponent::WorldScale(float3 const& scale)
{
    SetWorldDirty();
    RefreshWorldBasedLocal();
    m_cWorld.Scale(scale);
    RefreshLocalBasedWorld();
}
float3 SceneComponent::GetWorldRightVec()
{
    float4 const& f = this->GetWorldMatrix().Row(0);
    return float3(f.x(), f.y(), f.z());
}
float3 SceneComponent::GetWorldUpVec()
{
    float4 const& f = this->GetWorldMatrix().Row(1);
    return float3(f.x(), f.y(), f.z());
}
float3 SceneComponent::GetWorldForwardVec()
{
    float4 const& f = this->GetWorldMatrix().Row(2);
    return float3(f.x(), f.y(), f.z());
}
Transform const& SceneComponent::GetLocalTransform()
{
    return m_cLocal;
}

Matrix4 const& SceneComponent::GetLocalMatrix()
{
    return m_cLocal.Matrix();
}

void SceneComponent::SetLocalTransform(Transform const& transform)
{
    SetWorldDirty();
    m_cLocal = transform;
}

void SceneComponent::SetLocalTransform(Matrix4 const& matrix)
{
    SetWorldDirty();
    m_cLocal.Set(matrix);
}

void SceneComponent::SetLocalTransform(float3 const& translation, Quaternion const& rotation, float3 const& scale)
{
    SetWorldDirty();
    m_cLocal.Set(translation, rotation, scale);
}

void SceneComponent::SetLocalTranslation(float3 const& translation)
{
    SetWorldDirty();
    m_cLocal.SetTranslation(translation);
}

void SceneComponent::SetLocalRotation(Quaternion const& rot)
{
    SetWorldDirty();
    m_cLocal.SetRotation(rot);
}

void SceneComponent::SetLocalScale(float3 const& scale)
{
    SetWorldDirty();
    m_cLocal.SetScale(scale);
}

void SceneComponent::LocalTransform(Matrix4 const& matrix)
{
    SetWorldDirty();
    Matrix4 local_mat = this->GetLocalMatrix();
    Matrix4 m = matrix * local_mat;
    this->SetLocalTransform(m);
}

void SceneComponent::LocalTransform(float3 const* scale_center, float3 const* scale, float3 const* rotate_center, Quaternion const* rotate, float3 const* trans)
{
    SetWorldDirty();
    Matrix4 mat = Math::TransformEx(scale_center, nullptr, scale, rotate_center, rotate, trans);
    Matrix4 local_mat = this->GetLocalMatrix();
    Matrix4 m = mat * local_mat;
    this->SetLocalTransform(m);
}

void SceneComponent::LocalTranslate(float3 const& trans)
{
    SetWorldDirty();
    m_cLocal.Translate(trans);
}

void SceneComponent::LocalRotate(Quaternion const& rot)
{
    SetWorldDirty();
    m_cLocal.Rotate(rot);
}

void SceneComponent::LocalScale(float3 const& scale)
{
    SetWorldDirty();
    m_cLocal.Scale(scale);
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
