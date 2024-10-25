#include "components/animation_impl.h"
#include "components/scene_component.h"
#include "components/mesh_component.h"
#include "rhi/base/rhi_mesh.h"
#include "math/math_utility.h"
#include "utils/log.h"
#include <cmath>

#define SEEK_MACRO_FILE_UID 57     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/* *******************************************************************
 * Transform Animation
 * *******************************************************************/
TransformKeyFrame::TransformKeyFrame(TransformAnimationTrack* parent)
    :KeyFrame(parent)
{
}
void TransformKeyFrame::SetTranslate(float3 translate)
{
    m_fTranslate = translate;
    m_iTransformType |= TransformType::Translate;
}
void TransformKeyFrame::SetScale(float3 scale)
{
    m_fScale = scale;
    m_iTransformType |= TransformType::Scale;
}
void TransformKeyFrame::SetRotate(Quaternion rotate)
{
    m_qRotate = rotate;
    m_iTransformType |= TransformType::Rotate;
}


TransformAnimationTrack::TransformAnimationTrack(AnimationComponent* parent)
    :AnimationTrack(parent)
{
}
SResult TransformAnimationTrack::Run(float time)
{
    return Run(time, JointFusionMode::Cover);
}
SResult TransformAnimationTrack::Run(float time, JointFusionMode jointFusionMode)
{
    if (!m_pSceneComponent)
    {
        LOG_WARNING("TransformAnimationTrack::Run m_pSceneComponent is nullptr.");
        return S_Success;
    }
    if (m_vKeyFrames.empty())
    {
        LOG_ERROR("TransformAnimationTrack::Run m_vKeyFrames is empty");
        return ERR_INVALID_INIT;
    }

    TransformKeyFrame kf(nullptr);
    this->GetInterpolatedKeyFrame(time, kf, jointFusionMode);
    if (kf.HasTranslate())
    {
        float3 translate = kf.GetTranslate();
        if(jointFusionMode == JointFusionMode::Cover)
            m_pSceneComponent->SetLocalTranslation(translate);
        else if(jointFusionMode == JointFusionMode::Add)
            m_pSceneComponent->LocalTranslate(translate);
    }
    if (kf.HasScale())
    {
        float3 scale = kf.GetScale();
        if (jointFusionMode == JointFusionMode::Cover)
            m_pSceneComponent->SetLocalScale(scale);
        else if (jointFusionMode == JointFusionMode::Add)
            m_pSceneComponent->LocalScale(scale);
    }
    if (kf.HasRotate())
    {
        Quaternion rotate = kf.GetRotate();
        if (jointFusionMode == JointFusionMode::Cover)
            m_pSceneComponent->SetLocalRotation(rotate);
        else if (jointFusionMode == JointFusionMode::Add)
            m_pSceneComponent->LocalRotate(rotate);
    }
    return S_Success;
}
TransformKeyFramePtr TransformAnimationTrack::CreateTransformKeyFrame()
{
    TransformKeyFramePtr key = MakeSharedPtr<TransformKeyFrame>(this);
    m_vKeyFrames.push_back(std::static_pointer_cast<KeyFrame>(key));
    return key;
}
SResult TransformAnimationTrack::GetInterpolatedKeyFrame(float time, KeyFrame& kf)
{
    return GetInterpolatedKeyFrame(time, kf, JointFusionMode::Cover);
}
SResult TransformAnimationTrack::GetInterpolatedKeyFrame(float time, KeyFrame & kf, JointFusionMode jointFusionMode)
{
    uint32_t kfIdx;
    SEEK_RETIF_FAIL(GetKeyFrameIdxByTime(time, kfIdx));

    TransformKeyFrame& out = (TransformKeyFrame&)kf;
    TransformKeyFramePtr k0, k1, k2, k3;
    k0 = std::dynamic_pointer_cast<TransformKeyFrame>(m_vKeyFrames[Math::Max<int32_t>(0, kfIdx - 1)]);
    k1 = std::dynamic_pointer_cast<TransformKeyFrame>(m_vKeyFrames[kfIdx]);
    k2 = std::dynamic_pointer_cast<TransformKeyFrame>(m_vKeyFrames[kfIdx + 1]);
    k3 = std::dynamic_pointer_cast<TransformKeyFrame>(m_vKeyFrames[Math::Min<uint32_t>((uint32_t)(m_vKeyFrames.size() - 1), kfIdx + 2)]);

    float t0 = k0->GetTime();
    float t1 = k1->GetTime();
    float t2 = k2->GetTime();
    float t3 = k3->GetTime();

    float t  = (time - t1) / (t2 - t1);
    float tt = (time - t0) / (t3 - t0);

    if (k1->HasTranslate())
    {
        float3 v1 = jointFusionMode == JointFusionMode::Cover ? k1->GetTranslate() : k1->GetOffsetTranslate();
        float3 v2 = jointFusionMode == JointFusionMode::Cover ? k2->GetTranslate() : k2->GetOffsetTranslate();
        float3 v;
        switch (m_eInterpolationType)
        {
            case InterpolationType::Step:
                v = v1;
                break;
            case InterpolationType::Linear:
            case InterpolationType::SphericalLinear:
                v = Math::Lerp(v1, v2, t);
                break;
            case InterpolationType::CubicSpline:
            {
                float3 v0 = jointFusionMode == JointFusionMode::Cover ? k0->GetTranslate() : k0->GetOffsetTranslate();
                float3 v3 = jointFusionMode == JointFusionMode::Cover ? k3->GetTranslate() : k3->GetOffsetTranslate();
                v = Math::CubicSpline(v0, v1, v2, v3, tt);
                break;
            }
        }
        out.SetTranslate(v);
    }
    if (k1->HasScale())
    {
        float3 v1 = jointFusionMode == JointFusionMode::Cover ? k1->GetScale() : k1->GetOffsetScale();
        float3 v2 = jointFusionMode == JointFusionMode::Cover ? k2->GetScale() : k2->GetOffsetScale();
        float3 v;
        switch (m_eInterpolationType)
        {
            case InterpolationType::Step:
                v = v1;
                break;
            case InterpolationType::Linear:
            case InterpolationType::SphericalLinear:
                v = Math::Lerp(v1, v2, t);
                break;
            case InterpolationType::CubicSpline:
            {
                float3 v0 = jointFusionMode == JointFusionMode::Cover ? k0->GetScale() : k0->GetOffsetScale();
                float3 v3 = jointFusionMode == JointFusionMode::Cover ? k3->GetScale() : k3->GetOffsetScale();
                v = Math::CubicSpline(v0, v1, v2, v3, tt);
                break;
            }
        }
        out.SetScale(v);
    }
    if (k1->HasRotate())
    {
        Quaternion v1 = jointFusionMode == JointFusionMode::Cover ? k1->GetRotate() : k1->GetOffsetRotate();
        Quaternion v2 = jointFusionMode == JointFusionMode::Cover ? k2->GetRotate() : k2->GetOffsetRotate();
        Quaternion v;
        switch (m_eInterpolationType)
        {
            case InterpolationType::Step:
                v = v1;
                break;
            case InterpolationType::Linear:
            case InterpolationType::SphericalLinear:
                v = Math::QuatLerp(v1, v2, t);
                break;
            case InterpolationType::CubicSpline:
            {
                Quaternion v0 = jointFusionMode == JointFusionMode::Cover ? k0->GetRotate() : k0->GetOffsetRotate();
                Quaternion v3 = jointFusionMode == JointFusionMode::Cover ? k3->GetRotate() : k3->GetOffsetRotate();
                v = Math::QuatCubicSpline(v0, v1, v2, v3, tt);
                break;
            }
        }
        out.SetRotate(v);
    }

    return S_Success;
}

/* *******************************************************************
 * Morph target Animation
 * *******************************************************************/
MorphTargetKeyFrame::MorphTargetKeyFrame(MorphTargetAnimationTrack* parent)
    :KeyFrame(parent)
{
}

MorphTargetAnimationTrack::MorphTargetAnimationTrack(AnimationComponent* parent)
    :AnimationTrack(parent)
{
}
SResult MorphTargetAnimationTrack::Run(float time)
{
    return Run(time, MorphTargetWeightFusionMode::Cover);
}
SResult MorphTargetAnimationTrack::Run(float time, MorphTargetWeightFusionMode weightFusionMode)
{
    if (!m_pSceneComponent)
    {
        LOG_WARNING("MorphTargetAnimationTrack::Run, m_pSceneComponent is nullptr.");
        return S_Success;
    }
    if (m_pSceneComponent->GetComponentType() != ComponentType::Mesh &&
        m_pSceneComponent->GetComponentType() != ComponentType::SkeletalMesh)
    {
        LOG_WARNING("MorphTargetAnimationTrack::Run, m_pSceneComponent.ComponentType is not mesh|skeletal %d.", m_pSceneComponent->GetComponentType());
        return S_Success;
    }

    MorphTargetKeyFrame kf(nullptr);
    this->GetInterpolatedKeyFrame(time, kf);

    std::vector<RHIMeshPtr>& meshes = ((MeshComponent*)m_pSceneComponent.get())->GetMeshes();
    std::vector<float> const& weights = kf.GetWeights();
    for (auto& mesh : meshes)
    {
        if (mesh->GetMorphInfo().morph_target_type != MorphTargetType::None)
        {
            if (mesh->GetMorphInfo().morph_target_weights.size() == weights.size())
            {
                switch (weightFusionMode)
                {
                    case MorphTargetWeightFusionMode::Cover:
                        mesh->GetMorphInfo().morph_target_weights = weights;
                        break;
                    case MorphTargetWeightFusionMode::Add:
                    {
                        for (size_t i = 0; i < mesh->GetMorphInfo().morph_target_weights.size(); i++)
                        {
                            mesh->GetMorphInfo().morph_target_weights[i] += weights[i];
                            mesh->GetMorphInfo().morph_target_weights[i] = Math::Clamp<float>(mesh->GetMorphInfo().morph_target_weights[i], 0.0, 1.0);
                        }
                        break;
                    }
                }
            }
            else
            {
                LOG_ERROR("MorphTargetAnimationTrack::Run animation morph cnt != mesh morph cnt, %d!=%d",
                          weights.size(), mesh->GetMorphInfo().morph_target_weights.size());
            }
        }
        else
        {
            LOG_WARNING("MorphTargetAnimationTrack::Run Mesh's morph_target_type == MorphTargetType::None");
        }
    }

    std::vector<std::string>& morphTargetNames = meshes[0]->GetMorphInfo().morph_target_names;
    for (auto& linkageSceneComponent : m_pLinkageSceneComponents)
    {
        std::vector<RHIMeshPtr>& linkageMeshes = ((MeshComponent*)linkageSceneComponent.get())->GetMeshes();
        for (auto& linkageMesh : linkageMeshes)
        {
            if (linkageMesh->GetMorphInfo().morph_target_type != MorphTargetType::None)
            {
                std::vector<std::string>& linkageMorphTargetNames = linkageMesh->GetMorphInfo().morph_target_names;
                switch (weightFusionMode)
                {
                    case MorphTargetWeightFusionMode::Cover:
                    {
                        for (size_t i = 0; i < linkageMorphTargetNames.size(); i++)
                        {
                            for (size_t j = 0; j < morphTargetNames.size(); j++)
                            {
                                if (linkageMorphTargetNames[i] == morphTargetNames[j])
                                {
                                    linkageMesh->GetMorphInfo().morph_target_weights[i] = weights[j];
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case MorphTargetWeightFusionMode::Add:
                    {
                        for (size_t i = 0; i < linkageMorphTargetNames.size(); i++)
                        {
                            for (size_t j = 0; j < morphTargetNames.size(); j++)
                            {
                                if (linkageMorphTargetNames[i] == morphTargetNames[j])
                                {
                                    linkageMesh->GetMorphInfo().morph_target_weights[i] += weights[j];
                                    linkageMesh->GetMorphInfo().morph_target_weights[i] = Math::Clamp<float>(linkageMesh->GetMorphInfo().morph_target_weights[i], 0.0, 1.0);
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            else
            {
                LOG_WARNING("MorphTargetAnimationTrack::Run Mesh's morph_target_type == MorphTargetType::None");
            }
        }
    }
    return S_Success;
}

MorphTargetKeyFramePtr MorphTargetAnimationTrack::CreateMorphTargetKeyFrame()
{
    MorphTargetKeyFramePtr key = MakeSharedPtr<MorphTargetKeyFrame>(this);
    m_vKeyFrames.push_back(std::static_pointer_cast<KeyFrame>(key));
    return key;
}
SResult MorphTargetAnimationTrack::GetInterpolatedKeyFrame(float time, KeyFrame & kf)
{
    uint32_t kfIdx;
    SEEK_RETIF_FAIL(GetKeyFrameIdxByTime(time, kfIdx));

    MorphTargetKeyFrame& out = (MorphTargetKeyFrame&)kf;
    MorphTargetKeyFramePtr kf0 = std::static_pointer_cast<MorphTargetKeyFrame>(m_vKeyFrames[kfIdx]);
    MorphTargetKeyFramePtr kf1 = std::static_pointer_cast<MorphTargetKeyFrame>(m_vKeyFrames[kfIdx + 1]);

    std::vector<float> const& weights_kf0 = kf0->GetWeights();
    std::vector<float> const& weights_kf1 = kf1->GetWeights();
    std::vector<float> weights_out = out.GetWeights();
    weights_out.resize(weights_kf0.size());

    if (weights_kf0.size() != weights_kf1.size())
    {
        LOG_ERROR("MorphTargetAnimationTrack::GetInterpolatedKeyFrame Morph Target KeyFrame's Invalid Data");
        return ERR_INVALID_DATA;
    }

    float t = (time - kf0->GetTime()) / (kf1->GetTime() - kf0->GetTime());

    switch (m_eInterpolationType)
    {
        case InterpolationType::CubicSpline:
        {
//            if (weight_size * 3 == srcData1.size())
//            {
//                // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
//                float t1 = lerpRatio;
//                float t2 = t1 * t1;
//                float t3 = t2 * t1;
//                size_t outStep = weight_size << 1;
//                float l0 = t3 * 2.0f - t2 * 3.0f + 1.0f;
//                float l1 = t3 - t2 * 2.0f + t1;
//                float l2 = t2 * 3.0f - t3 * 2.0f;
//                float l3 = t3 - t2;
//                for (size_t w = 0; w < weight_size; ++w)
//                {
//                    float m0 = srcData2[w] * lerpRange;
//                    float m1 = srcData1[w + outStep] * lerpRange;
//                    float w0 = srcData1[w + weight_size];
//                    float w1 = srcData2[w + weight_size];
//                    float dw = l0 * w0 + l1 * m1 + l2 * w1 + l3 * m0;
//                    dst_data[w] = dw;
//                }
//            }
            return ERR_NOT_SUPPORT;
            break;
        }
        case InterpolationType::Linear:
        case InterpolationType::SphericalLinear:
        {
            for (size_t w = 0; w < weights_out.size(); ++w)
            {
                weights_out[w] = Math::Lerp(weights_kf0[w], weights_kf1[w], t);
            }
            break;
        }
        case InterpolationType::Step:
        {
            weights_out = t < 0.5f ? weights_kf0 : weights_kf1;
            break;
        }
    }
    out.SetWeights(weights_out);
    return S_Success;
}

void MorphTargetAnimationTrack::AddLinkageSceneComponent(SceneComponentPtr const& sc)
{
    auto iter = std::find(m_pLinkageSceneComponents.begin(), m_pLinkageSceneComponents.end(), sc);
    if (iter == m_pLinkageSceneComponents.end())
    {
        m_pLinkageSceneComponents.push_back(sc);
    }
}

void MorphTargetAnimationTrack::DeleteLinkageSceneComponent(SceneComponentPtr const& sc)
{
    auto iter = std::find(m_pLinkageSceneComponents.begin(), m_pLinkageSceneComponents.end(), sc);
    if (iter != m_pLinkageSceneComponents.end())
    {
        m_pLinkageSceneComponents.erase(iter);
    }
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
