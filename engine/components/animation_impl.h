#pragma once

#include "kernel/kernel.h"
#include "components/animation_component.h"
#include "math/quaternion.h"

SEEK_NAMESPACE_BEGIN

/* *******************************************************************
 * Transform Animation
 * *******************************************************************/
enum TransformType : uint32_t
{
    None        = 0x00,
    Translate   = 0x01,
    Scale       = 0x02,
    Rotate      = 0x04,
    All         = 0xff,
};

class TransformAnimationTrack;
class TransformKeyFrame : public KeyFrame
{
public:
    TransformKeyFrame(TransformAnimationTrack* parent);
    virtual ~TransformKeyFrame() {}

    void        SetTranslate(float3 translate);
    void        SetScale(float3 scale);
    void        SetRotate(Quaternion rotate);

    float3      GetTranslate() const { return m_fTranslate; }
    float3      GetScale()     const { return m_fScale; }
    Quaternion  GetRotate()    const { return m_qRotate; }

    bool        HasTranslate()      { return (bool)(m_iTransformType & TransformType::Translate); }
    bool        HasScale()          { return (bool)(m_iTransformType & TransformType::Scale); }
    bool        HasRotate()         { return (bool)(m_iTransformType & TransformType::Rotate); }

    void        SetOffsetTranslate(float3 translate) { m_fOffsetTranslate = translate; }
    void        SetOffsetScale(float3 scale) { m_fOffsetScale = scale; }
    void        SetOffsetRotate(Quaternion rotate) { m_qOffsetRotate = rotate; }

    float3      GetOffsetTranslate() const { return m_fOffsetTranslate; }
    float3      GetOffsetScale()     const { return m_fOffsetScale; }
    Quaternion  GetOffsetRotate()    const { return m_qOffsetRotate; }

protected:
    float3          m_fTranslate        = float3(0, 0, 0);
    float3          m_fScale            = float3(1, 1, 1);
    Quaternion      m_qRotate           = Quaternion::Identity();
    uint32_t        m_iTransformType    = TransformType::None;     // TransformType

    float3          m_fOffsetTranslate  = float3(0, 0, 0);
    float3          m_fOffsetScale      = float3(1, 1, 1);
    Quaternion      m_qOffsetRotate     = Quaternion::Identity();
};

class TransformAnimationTrack : public AnimationTrack
{
public:
    TransformAnimationTrack(AnimationComponent* parent);
    virtual SResult                   Run(float time) override;
    SResult                           Run(float time, JointFusionMode jointFusionMode);

    TransformKeyFramePtr                CreateTransformKeyFrame();

protected:
    virtual SResult                   GetInterpolatedKeyFrame(float time, KeyFrame & kf) override;
    SResult                           GetInterpolatedKeyFrame(float time, KeyFrame & kf, JointFusionMode jointFusionMode);
};

/* *******************************************************************
 * Morph target Animation
 * *******************************************************************/
class MorphTargetKeyFrame : public KeyFrame
{
public:
    MorphTargetKeyFrame(MorphTargetAnimationTrack* parent);

    std::vector<float> const&           GetWeights() const { return m_vWeights; }
    void                                SetWeights(std::vector<float> weights) { m_vWeights = weights; }

private:
    std::vector<float>                  m_vWeights;
};

class MorphTargetAnimationTrack : public AnimationTrack
{
public:
    MorphTargetAnimationTrack(AnimationComponent* parent);
    virtual SResult                   Run(float time) override;
    SResult                           Run(float time, MorphTargetWeightFusionMode weightFusionMode);

    MorphTargetKeyFramePtr              CreateMorphTargetKeyFrame();
    void                                AddLinkageSceneComponent(SceneComponentPtr const& sc);
    void                                DeleteLinkageSceneComponent(SceneComponentPtr const& sc);

protected:
    virtual SResult                   GetInterpolatedKeyFrame(float time, KeyFrame & kf) override;
    std::vector<SceneComponentPtr>      m_pLinkageSceneComponents;
};

SEEK_NAMESPACE_END
