#pragma once

#include "components/component.h"
#include "math/matrix.h"

SEEK_NAMESPACE_BEGIN

enum class AnimationMsgType
{
    Start,
    Pause,
    Resume,
    Stop,
};

typedef void (*AnimationCallback)(std::string const& anim_name, AnimationMsgType msg_type, void* user_data);

enum class InterpolationType : uint32_t  // glTF2.0 defined
{
    Step,           // https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#_step_interpolation
    Linear,         // https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#interpolation-lerp
    SphericalLinear,// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#interpolation-slerp
    CubicSpline,    // https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#interpolation-cubic
};

enum class AnimationState : uint32_t
{
    Stopped,
    Playing,
    Pause,
};

enum class MorphTargetWeightFusionMode
{
    Cover,
    Add,
};

enum class JointFusionMode
{
    Cover,
    Add,
};

struct AnimInfo
{
    float                       startTime = 0.0f;
    float                       endTime = 0.0f;
    float                       preFrameTime = 0.0f;
    bool                        loop = false;
    AnimationState              state = AnimationState::Stopped;
    MorphTargetWeightFusionMode weightFusionMode = MorphTargetWeightFusionMode::Cover;
    JointFusionMode             jointFusionMode = JointFusionMode::Cover;
};

class AnimationComponent : public Component
{
public:
    AnimationComponent(Context* context);
    virtual ~AnimationComponent() {}

    virtual SResult           Tick(float delta_time) override;
    SResult                   SectionTick(float delta_time, uint32_t sectionIdx);
    void                        Play(uint32_t sectionIdx = 0);
    void                        Pause(uint32_t sectionIdx = 0);
    void                        Resume(uint32_t sectionIdx = 0);
    void                        Stop(uint32_t sectionIdx = 0);
    void                        StopAtEndTime(uint32_t sectionIdx = 0);
    void                        AddAnimSectionInfo(AnimInfo& animSectionInfo) { m_sAnimSectionInfo.push_back(animSectionInfo); }
    std::vector<AnimInfo>&      GetAnimSectionInfo() { return m_sAnimSectionInfo; }
    std::vector<TransformAnimationTrackPtr>&        GetTransformAnimationTracks() { return m_vTransformAnimationTracks; }
    std::vector<MorphTargetAnimationTrackPtr>&      GetMorphTargetAnimationTracks() { return m_vMorphTargetAnimationTracks; }

    TransformAnimationTrackPtr      CreateTransformTrack();
    MorphTargetAnimationTrackPtr    CreateMorphTargetTrack();
    void                            RegisterAnimationCallback(AnimationCallback cb, void* user_data);

protected:
    std::vector<AnimInfo>                       m_sAnimSectionInfo;
    std::vector<TransformAnimationTrackPtr>     m_vTransformAnimationTracks;
    std::vector<MorphTargetAnimationTrackPtr>   m_vMorphTargetAnimationTracks;
    AnimationCallback                           m_pAnimationCallback = nullptr;
    void*                                       m_pAnimationCallbackData = nullptr;
};

//***************************************************************
// Abstract class
//***************************************************************

class KeyFrame
{
public:
    virtual ~KeyFrame() {}
    float   GetTime() const { return m_fTime; }
    void    SetTime(float time) { m_fTime = time; }

protected:
    KeyFrame(AnimationTrack* parent): m_pParent(parent) {}

    AnimationTrack* m_pParent = nullptr;
    float           m_fTime = 0.0f;
};

// A set of keyframes for an action(transform or morph) on a scene node (bone or mesh).
class AnimationTrack
{
public:
    virtual ~AnimationTrack() { m_vKeyFrames.clear(); }
    virtual SResult   Run(float time) = 0;

    InterpolationType   GetInterpolationType() const { return m_eInterpolationType; }
    void                SetInterpolationType(InterpolationType type) { m_eInterpolationType = type; }
    SceneComponentPtr   GetSceneComponent() const { return m_pSceneComponent; }
    void                SetSceneComponent(SceneComponentPtr sc) { m_pSceneComponent = sc; }

protected:
    AnimationTrack(AnimationComponent* parent): m_pParent(parent) {}
    virtual SResult   GetInterpolatedKeyFrame(float time, KeyFrame & kf) = 0;
    SResult           GetKeyFrameIdxByTime(float & time, uint32_t & kfIdx);

    AnimationComponent*         m_pParent = nullptr;
    std::vector<KeyFramePtr>    m_vKeyFrames;
    InterpolationType           m_eInterpolationType = InterpolationType::Linear;
    SceneComponentPtr           m_pSceneComponent = nullptr;
};

//***************************************************************
// ~ Abstract class
//***************************************************************


SEEK_NAMESPACE_END
