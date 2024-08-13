#include "components/spring_skeleton_component.h"
#include "utils/log.h"
#include <cmath>

#define SEEK_MACRO_FILE_UID 90     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

SpringSkeletonComponent::SpringSkeletonComponent(Context* context, std::string const& name)
    :Component(context, name, ComponentType::SpringSkeleton)
{
}

void SpringSkeletonComponent::AddJointMass(std::string jointName, SceneComponentPtr joint, bool pinned, float restCoefficient, float dampingCoefficient)
{
    JointMass jointMass;
    jointMass.jointName = jointName;
    jointMass.joint = joint;
    jointMass.pinned = pinned;
    jointMass.restCoefficient = restCoefficient;
    jointMass.dampingCoefficient = dampingCoefficient;
    float3 pos = joint->GetWorldMatrix().Row(3) * m_fScaleMappingFactor;
    jointMass.jointOriginLocalTransform = joint->GetLocalTransform();
    jointMass.startPosition = pos;
    jointMass.currentPosition = pos;
    jointMass.lastPosition = pos;
    jointMass.restPosition = pos;
    m_vJointMasses.push_back(jointMass);
    return;
}

void SpringSkeletonComponent::AddSpring(std::string jointNameA, std::string jointNameB, float springCoefficient)
{
    Spring s;
    s.jointNameA = jointNameA;
    s.jointNameB = jointNameB;
    s.springCoefficient = springCoefficient;
    for (int32_t i = 0; i < m_vJointMasses.size(); i++)
    {
        if (m_vJointMasses[i].jointName == jointNameA)
        {
            s.jointMassA = &(m_vJointMasses[i]);
            break;
        }
    }
    for (int32_t i = 0; i < m_vJointMasses.size(); i++)
    {
        if (m_vJointMasses[i].jointName == jointNameB)
        {
            s.jointMassB = &(m_vJointMasses[i]);
            break;
        }
    }
    if (s.jointMassA && s.jointMassB)
    {
        s.restLength = Math::Length(s.jointMassA->currentPosition - s.jointMassB->currentPosition);
        m_vSprings.push_back(s);
    }
    else
    {
        LOG_ERROR("SpringSkeletonComponent::AddSpring, not find joint name: %s, %s", jointNameA.c_str(), jointNameB.c_str());
    }
    return;
}

SResult SpringSkeletonComponent::Tick(float delta_time)
{
    if (m_bResetStatusFlag || m_bBypassTickFlag)
    {
        if (m_bResetStatusFlag)
        {
            ResetStatus();
            m_bResetStatusFlag = false;
        }
        if (m_bBypassTickFlag)
        {
            // for generate thumbnail
            for (int32_t i = 0; i < m_vJointMasses.size(); i++)
            {
                m_vJointMasses[i].joint->SetLocalTransform(m_vJointMasses[i].jointOriginLocalTransform);
            }
        }
        return S_Success;
    }

    for (int32_t i = 0; i < m_vJointMasses.size(); i++)
    {
        m_vJointMasses[i].joint->SetLocalTransform(m_vJointMasses[i].jointOriginLocalTransform);
    }
    for (int32_t i = 0; i < m_vJointMasses.size(); i++)
    {
        m_vJointMasses[i].restPosition = m_vJointMasses[i].joint->GetWorldMatrix().Row(3) * m_fScaleMappingFactor;
        if (m_vJointMasses[i].pinned)
            m_vJointMasses[i].currentPosition = m_vJointMasses[i].restPosition;
    }
    for (auto& collider: m_vColliders)
    {
        float4 pos(collider.centerLocalPosition[0], collider.centerLocalPosition[1], collider.centerLocalPosition[2], 1.0f);
        collider.centerWorldPosition = pos * collider.bindJoint->GetWorldMatrix() * m_fScaleMappingFactor;
    }

    for (uint32_t i = 0; i < m_iStepsPerFrame; i++)
    {
        Update(m_fIntervalLength / float(m_iStepsPerFrame));
        CollisionResponseMethod1();
        //CollisionResponseMethod2(m_pRootJoint, -1);
    }

    ApplySkeleton(m_pRootJoint, -1);
    return S_Success;
}

void SpringSkeletonComponent::Update(float delta_time)
{
    for (auto& spring : m_vSprings)
    {
        float3 spring_dir = spring.jointMassB->currentPosition - spring.jointMassA->currentPosition;
        float spring_len = Math::Length(spring_dir);
        if (spring_len > 0.0001f)
        {
            spring_dir = spring_dir / spring_len;
            float3 force1;
            force1 = float3(spring.springCoefficient * (spring_len - spring.restLength)) * spring_dir;
            //float force1_len = Math::Length(force1);
            //force1 = force1_len > spring.springForceUpperLimit ? (force1 / force1_len * spring.springForceUpperLimit) : force1;
            spring.jointMassA->forces += force1;
            spring.jointMassB->forces -= force1;
        }
    }

    for (auto& mass : m_vJointMasses)
    {
        if (!mass.pinned)
        {
            mass.forces += m_fGravityAcceleration * mass.mass;
            float3 rest_force = (mass.restPosition - mass.currentPosition) * mass.restCoefficient;
            //float rest_force_len = Math::Length(rest_force);
            //rest_force = rest_force_len > mass.restForceUpperLimit ? (rest_force / rest_force_len * mass.restForceUpperLimit) : rest_force;
            mass.forces += rest_force;
            mass.forces += float3(-mass.dampingCoefficient) * mass.velocity;

            float3 acc = mass.forces / mass.mass;
            if (m_bIsExplicit)
            {
                mass.currentPosition = mass.currentPosition + mass.velocity * delta_time;
                mass.velocity = mass.velocity + acc * delta_time;
            }
            else
            {
                mass.velocity = mass.velocity + acc * delta_time;
                mass.currentPosition = mass.currentPosition + mass.velocity * delta_time;
            }
        }

        mass.forces = float3(0.0f, 0.0f, 0.0f);
    }
}

void SpringSkeletonComponent::ApplySkeleton(SceneComponentPtr joint, int32_t massIdx)
{
    uint32_t child_num = (uint32_t)joint->NumChildren();
    for (uint32_t i = 0; i < child_num; i++)
    {
        SceneComponentPtr child_joint = joint->ChildByIndex(i);
        int32_t child_massIdx = -1;
        for (int32_t j = 0; j < m_vJointMasses.size(); j++)
        {
            if (m_vJointMasses[j].joint == child_joint)
            {
                child_massIdx = j;
                break;
            }
        }
        if (child_massIdx >= 0)
        {
            if (!m_vJointMasses[child_massIdx].pinned)
            {
                float3 srcVec = child_joint->GetWorldMatrix().Row(3) - joint->GetWorldMatrix().Row(3);
                float3 dstVec;
                //if (massIdx >= 0)
                //    dstVec = m_vJointMasses[child_massIdx].currentPosition / m_fScaleMappingFactor - m_vJointMasses[massIdx].currentPosition / m_fScaleMappingFactor;
                //else
                    dstVec = m_vJointMasses[child_massIdx].currentPosition / m_fScaleMappingFactor - float3(joint->GetWorldMatrix().Row(3));
                float angle = acos(Math::Dot(srcVec, dstVec) / (Math::Length(srcVec) * Math::Length(dstVec)));
                if (angle > 0.001f)  //if (isnormal(angle) || angle > 0.001f)
                {
                    float3 zVec = Math::Cross(srcVec, dstVec);
                    float3 yVec = Math::Normalize(Math::Cross(zVec, srcVec));
                    float3 xVec = Math::Normalize(srcVec);
                    zVec = Math::Normalize(zVec);
                    Matrix3 rot;
                    rot.Row(0, xVec); rot.Row(1, yVec); rot.Row(2, zVec);
                    rot = rot.Inverse() * Math::Mat3RotationZ(angle) * rot;
                    for (int32_t j = 0; j < 9; j++)
                    {
                        if (!std::isnormal(rot[j]) && rot[j] != 0.0f)
                        {
                            rot = Matrix3::Identity();
                            break;
                        }
                    }

                    //Quaternion qua = Math::MatrixToQuaternion(Matrix4(rot));
                    //for (int32_t j = 0; j < 4; j++)
                    //{
                    //    if (!isnormal(qua[j]) && qua[j] != 0.0f)
                    //    {
                    //        qua = Quaternion::Identity();
                    //        break;
                    //    }
                    //}
                    //Quaternion tmp = joint->GetWorldTransform().GetRotation();
                    //joint->SetLocalRotation(tmp * qua * tmp.Inverse() * joint->GetLocalTransform().GetRotation());
                    Matrix4 tmp = joint->GetWorldMatrix();
                    tmp[12] = 0.0f; tmp[13] = 0.0f; tmp[14] = 0.0f; tmp[15] = 1.0f;
                    tmp = tmp * Matrix4(rot) * tmp.Inverse();
                    joint->SetLocalTransform(tmp * joint->GetLocalMatrix());
                }
                float4 pos(m_vJointMasses[child_massIdx].currentPosition[0] / m_fScaleMappingFactor, m_vJointMasses[child_massIdx].currentPosition[1] / m_fScaleMappingFactor, m_vJointMasses[child_massIdx].currentPosition[2] / m_fScaleMappingFactor, 1.0f);
                child_joint->SetLocalTranslation(pos * joint->GetWorldMatrixInv());
            }
        }
        ApplySkeleton(child_joint, child_massIdx);
    }
}

void SpringSkeletonComponent::AddColliders(SphereCollider& collider)
{
    if (collider.bindJoint)
    {
        //float4 pos(collider.centerWorldPosition[0], collider.centerWorldPosition[1], collider.centerWorldPosition[2], 1.0f);
        //collider.centerLocalPosition = pos * collider.bindJoint->GetWorldMatrixInv();
        collider.centerWorldPosition = collider.centerWorldPosition * m_fScaleMappingFactor;
        collider.radius = collider.radius * m_fScaleMappingFactor;
        m_vColliders.push_back(collider);
    }
}

void SpringSkeletonComponent::CollisionResponseMethod1()
{
    for (auto& mass: m_vJointMasses)
    {
        if (!mass.pinned)
        {
            for (auto& c: m_vColliders)
            {
                float3 distVec = mass.currentPosition - c.centerWorldPosition;
                float distLen = Math::Length(distVec);
                float safeDist = c.radius + mass.safeRadius;
                if (abs(distLen) < 0.0001f)
                    continue;
                if (distLen < safeDist)  //////distLen != 0.0f
                {
                    mass.currentPosition = distVec / distLen * safeDist + c.centerWorldPosition;
                    mass.velocity *= c.frictionCoefficient;
                }
            }
        }
    }
}

void SpringSkeletonComponent::CollisionResponseMethod2(SceneComponentPtr joint, int32_t massIdx)
{
    uint32_t child_num = (uint32_t)joint->NumChildren();
    for (uint32_t i = 0; i < child_num; i++)
    {
        SceneComponentPtr child_joint = joint->ChildByIndex(i);
        int32_t child_massIdx = -1;
        for (int32_t j = 0; j < m_vJointMasses.size(); j++)
        {
            if (m_vJointMasses[j].joint == child_joint)
            {
                child_massIdx = j;
                break;
            }
        }
        if (child_massIdx >= 0)
        {
            if (!m_vJointMasses[child_massIdx].pinned)
            {
                float3 mass1_pos;
                if (massIdx >= 0)
                    mass1_pos = m_vJointMasses[massIdx].currentPosition;
                else
                    mass1_pos = float3(joint->GetWorldMatrix().Row(3)) * m_fScaleMappingFactor;
                float3 mass2_pos = m_vJointMasses[child_massIdx].currentPosition;
                for (auto& c: m_vColliders)
                {
                    float3 distVec1 = c.centerWorldPosition - mass1_pos;
                    float distLen1 = Math::Length(distVec1);
                    float safeDist = c.radius + m_vJointMasses[child_massIdx].safeRadius;
                    if (distLen1 <= safeDist)
                    {
                        float3 distVec2 = mass2_pos - c.centerWorldPosition;
                        float distLen2 = Math::Length(distVec2);
                        if (abs(distLen2) < 0.0001f)
                            continue;
                        if (distLen2 < safeDist)  //////distLen2 != 0.0f
                        {
                            m_vJointMasses[child_massIdx].currentPosition = distVec2 / distLen2 * safeDist + c.centerWorldPosition;
                            m_vJointMasses[child_massIdx].velocity *= c.frictionCoefficient;
                        }
                    }
                    else
                    {
                        float angle1 = asin(safeDist / distLen1);
                        float3 distVec2 = mass2_pos - mass1_pos;
                        float distLen2 = Math::Length(distVec2);
                        if (abs(distLen2) < 0.0001f)
                            continue;
                        float angle2 = acos(Math::Dot(distVec1, distVec2) / (distLen1 * distLen2));
                        float distLen3 = sqrt(distLen1 * distLen1 - safeDist * safeDist);
                        if (angle2 < angle1 && ((distLen2 > distLen3) || (Math::Length(c.centerWorldPosition - mass2_pos) < safeDist)))
                        {
                            float3 zVec = Math::Cross(distVec1, distVec2);
                            float3 yVec = Math::Normalize(Math::Cross(zVec, distVec1));
                            float3 xVec = Math::Normalize(distVec1);
                            zVec = Math::Normalize(zVec);
                            Matrix3 rot;
                            rot.Row(0, xVec); rot.Row(1, yVec); rot.Row(2, zVec);
                            rot = rot.Inverse() * Math::Mat3RotationZ(angle1) * rot;
                            for (int32_t j = 0; j < 9; j++)
                            {
                                if (!std::isnormal(rot[j]) && rot[j] != 0.0f)
                                {
                                    rot = Matrix3::Identity();
                                    break;
                                }
                            }
                            float3 tmp = distVec1 * rot;
                            m_vJointMasses[child_massIdx].currentPosition = tmp / Math::Length(tmp) * distLen2 + mass1_pos;
                            m_vJointMasses[child_massIdx].velocity *= c.frictionCoefficient;
                        }
                    }
                }
            }
        }

        CollisionResponseMethod2(child_joint, child_massIdx);
    }
}

void SpringSkeletonComponent::ResetStatus()
{
    for (int32_t i = 0; i < m_vJointMasses.size(); i++)
    {
        m_vJointMasses[i].joint->SetLocalTransform(m_vJointMasses[i].jointOriginLocalTransform);
    }

    for (auto& mass : m_vJointMasses)
    {
        float3 pos = mass.joint->GetWorldMatrix().Row(3) * m_fScaleMappingFactor;
        mass.currentPosition = pos;
        mass.restPosition = pos;
        mass.velocity = float3(0.0f, 0.0f, 0.0f);
    }
    return;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
