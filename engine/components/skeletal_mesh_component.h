/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : mesh_component.h
**
**      Brief                  : skeletal mesh component base class
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-06-07  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "components/mesh_component.h"
#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

class SkeletalMeshComponent : public MeshComponent
{
public:
    SkeletalMeshComponent(Context* context);
    virtual ~SkeletalMeshComponent();

    void                    AddInverseBindMatrix(Matrix4 const& mat);
    void                    SetInverseBindMatrix(std::vector<Matrix4> const& matArray);
    Matrix4 const&          GetInverseBindMatrixByIndex(uint32_t index) const;

    void                    AddJoint(SceneComponentPtr joint);
    void                    SetJoint(std::vector<SceneComponentPtr>& jointArray);
    void                    SetJointByIndex(uint32_t index, SceneComponentPtr joint);
    SceneComponentPtr       GetJointByIndex(uint32_t index) const;
    uint32_t                GetJointCount() const { return (uint32_t)m_vJoints.size(); }

    //SceneComponentPtr     GetSkeletonRoot() { return m_pSkeletonRoot; } // not used
    void                    SetSkeletonRoot(SceneComponentPtr root) { m_pSkeletonRoot = root; }

    void                    UpdateJointFinalMatrices();
    std::vector<Matrix4>&   GetJointFinalMatrices() { return m_vJointFinalMatrices; };

    void                    SetMeshTransformFlag(bool flag) { m_bMeshTransformFlag = flag; };
    void                    SetMeshTransformMatrix(Matrix4 mat) { m_MeshTransformMatrix = mat; };

    //virtual SResult         OnRenderBegin(Technique* tech, RHIMeshPtr pMesh) override;
    //virtual SResult         OnRenderEnd() override;
protected:
    std::vector<Matrix4>            m_vInverseBindMatrices;
    std::vector<SceneComponentPtr>  m_vJoints;
    SceneComponentPtr               m_pSkeletonRoot = nullptr; // not used

    std::vector<Matrix4>            m_vJointFinalMatrices;
    SkeletalJointMat                m_JointFinalMatricesToGPU;

    bool                            m_bMeshTransformFlag = false;
    Matrix4                         m_MeshTransformMatrix = Matrix4::Identity();

    RHIGpuBufferPtr              m_JointsCBuffer;
};

SEEK_NAMESPACE_END
