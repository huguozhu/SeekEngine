#include "components/skeletal_mesh_component.h"
#include "components/camera_component.h"
#include "components/light_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/mesh.h"
#include "rhi/texture.h"
#include "rhi/render_buffer.h"
#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"
#include "math/color.h"
#include "math/math_utility.h"
#include <math.h>

#define SEEK_MACRO_FILE_UID 55     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SkeletalMeshComponent::SkeletalMeshComponent(Context* context)
    :MeshComponent(context)
{
    m_eComponentType = ComponentType::SkeletalMesh;
    m_szName = "SkeletalMeshComponent";
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{
}

void SkeletalMeshComponent::AddInverseBindMatrix(Matrix4 const& mat)
{
    m_vInverseBindMatrices.push_back(mat);
}

void SkeletalMeshComponent::SetInverseBindMatrix(std::vector<Matrix4> const& matArray)
{
    m_vInverseBindMatrices = matArray;
}

Matrix4 const& SkeletalMeshComponent::GetInverseBindMatrixByIndex(uint32_t index) const
{
    return m_vInverseBindMatrices[index];
}

void SkeletalMeshComponent::AddJoint(SceneComponentPtr joint)
{
    m_vJoints.push_back(joint);
}

void SkeletalMeshComponent::SetJoint(std::vector<SceneComponentPtr>& jointArray)
{
    m_vJoints = jointArray;
}

void SkeletalMeshComponent::SetJointByIndex(uint32_t index, SceneComponentPtr joint)
{
    if (index >= m_vJoints.size())
    {
        LOG_ERROR("SkeletalMeshComponent::SetJointByIndex(), the index %d exceeds joints array size %d", index, m_vJoints.size());
        return;
    }
    m_vJoints[index] = joint;
}

SceneComponentPtr SkeletalMeshComponent::GetJointByIndex(uint32_t index) const
{
    return m_vJoints[index];
}

void SkeletalMeshComponent::UpdateJointFinalMatrices()
{
    uint32_t count = (uint32_t)m_vInverseBindMatrices.size();
    count = Math::Min<uint32_t>(count, JOINT_MAX_COUNT);

    if (m_vJointFinalMatrices.size() != count)
        m_vJointFinalMatrices.resize(count);

    if(m_bMeshTransformFlag)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            m_vJointFinalMatrices[i] = m_MeshTransformMatrix * m_vInverseBindMatrices[i] * m_vJoints[i]->GetWorldMatrix() * GetWorldMatrixInv();
            m_JointFinalMatricesToGPU.joint_mat[i] = (m_vJointFinalMatrices[i]).Transpose();
        }
    }
    else
    {
        for (uint32_t i = 0; i < count; i++)
        {
            m_vJointFinalMatrices[i] = m_vInverseBindMatrices[i] * m_vJoints[i]->GetWorldMatrix() * GetWorldMatrixInv();
            m_JointFinalMatricesToGPU.joint_mat[i] = (m_vJointFinalMatrices[i]).Transpose();
        }
    }
}

SResult SkeletalMeshComponent::OnRenderBegin(Technique* tech, MeshPtr pMesh)
{
    if (!pMesh)
        return ERR_INVALID_ARG;

    DVF_RETIF_FAIL(MeshComponent::OnRenderBegin(tech, pMesh));

    if (m_vInverseBindMatrices.empty())
        return S_Success;
    if (m_vInverseBindMatrices.size() > JOINT_MAX_COUNT)
    {
        LOG_ERROR("SkeletalMeshComponent::OnRenderBegin(), the number of joints %d exceeds MAX_JOINT %d", m_vInverseBindMatrices.size(), JOINT_MAX_COUNT);
        return ERR_NOT_SUPPORT;
    }

    RenderStage stage = m_pContext->SceneRendererInstance().GetCurRenderStage();
    SResult ret = S_Success;

    switch (stage)
    {
    case RenderStage::RenderScene:
    {
        if (!m_JointsCBuffer)
        {
            m_JointsCBuffer = m_pContext->RenderContextInstance().CreateConstantBuffer(sizeof(m_JointFinalMatricesToGPU), RESOURCE_FLAG_CPU_WRITE);
        }
        m_JointsCBuffer->Update(m_JointFinalMatricesToGPU.joint_mat[0].begin(), sizeof(m_JointFinalMatricesToGPU));
        tech->SetParam("joints", m_JointsCBuffer);
        break;
    }
    default:
        break;
    }

    return S_Success;
}

SResult SkeletalMeshComponent::OnRenderEnd()
{
    return MeshComponent::OnRenderEnd();
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
