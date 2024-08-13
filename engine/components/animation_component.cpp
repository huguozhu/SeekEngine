#include "components/animation_component.h"
#include "components/animation_impl.h"
#include "utils/log.h"
#include "utils/error.h"

#define SEEK_MACRO_FILE_UID 56     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

AnimationComponent::AnimationComponent(Context* context)
    :Component(context, "AnimationComponent", ComponentType::Animation)
{
}

SResult AnimationComponent::SectionTick(float delta_time, uint32_t sectionIdx)
{
    AnimInfo& animSectionInfo = m_sAnimSectionInfo[sectionIdx];
    if (animSectionInfo.state == AnimationState::Playing)
    {
        float length = animSectionInfo.endTime;
        float cur_time = animSectionInfo.preFrameTime + delta_time;
        if (cur_time >= length)
        {
            if (animSectionInfo.loop)
            {
                cur_time = cur_time - (int)(cur_time / length) * length;
            }
            else
            {
                cur_time = length;
                animSectionInfo.state = AnimationState::Stopped;
            }
        }
        animSectionInfo.preFrameTime = cur_time;

        auto itt = m_vTransformAnimationTracks.begin();
        while (itt != m_vTransformAnimationTracks.end())
        {
            (*itt)->Run(cur_time, animSectionInfo.jointFusionMode);
            ++itt;
        }

        auto itwt = m_vMorphTargetAnimationTracks.begin();
        while (itwt != m_vMorphTargetAnimationTracks.end())
        {
            (*itwt)->Run(cur_time, animSectionInfo.weightFusionMode);
            ++itwt;
        }

        if (animSectionInfo.state == AnimationState::Stopped)
            animSectionInfo.preFrameTime = animSectionInfo.startTime;
    }
    return S_Success;
}

SResult AnimationComponent::Tick(float delta_time)
{
    for (uint32_t i = 0; i < m_sAnimSectionInfo.size(); i++)
    {
        SectionTick(delta_time, i);
    }
    return S_Success;
}

void AnimationComponent::Play(uint32_t sectionIdx)
{
    m_sAnimSectionInfo[sectionIdx].state = AnimationState::Playing;
    if (m_pAnimationCallback)
        m_pAnimationCallback(m_szName, AnimationMsgType::Start, m_pAnimationCallbackData);
}

void AnimationComponent::Pause(uint32_t sectionIdx)
{
    m_sAnimSectionInfo[sectionIdx].state = AnimationState::Pause;
    if (m_pAnimationCallback)
        m_pAnimationCallback(m_szName, AnimationMsgType::Pause, m_pAnimationCallbackData);
}

void AnimationComponent::Resume(uint32_t sectionIdx)
{
    AnimInfo& animSection = m_sAnimSectionInfo[sectionIdx];
    animSection.state = AnimationState::Playing;
    animSection.preFrameTime = animSection.startTime;
    if (m_pAnimationCallback)
        m_pAnimationCallback(m_szName, AnimationMsgType::Resume, m_pAnimationCallbackData);
}

void AnimationComponent::Stop(uint32_t sectionIdx)
{
    AnimInfo& animSection = m_sAnimSectionInfo[sectionIdx];
    animSection.state = AnimationState::Playing;
    animSection.preFrameTime = animSection.startTime;
    SectionTick(0.0f, sectionIdx);
    animSection.state = AnimationState::Stopped;
    if (m_pAnimationCallback)
        m_pAnimationCallback(m_szName, AnimationMsgType::Stop, m_pAnimationCallbackData);
}

void AnimationComponent::StopAtEndTime(uint32_t sectionIdx)
{
    AnimInfo& animSection = m_sAnimSectionInfo[sectionIdx];
    animSection.state = AnimationState::Playing;
    animSection.preFrameTime = animSection.endTime;
    SectionTick(0.0f, sectionIdx);
    animSection.state = AnimationState::Stopped;
    animSection.preFrameTime = animSection.startTime;
    if (m_pAnimationCallback)
        m_pAnimationCallback(m_szName, AnimationMsgType::Stop, m_pAnimationCallbackData);
}

TransformAnimationTrackPtr AnimationComponent::CreateTransformTrack()
{
    TransformAnimationTrackPtr track = MakeSharedPtr<TransformAnimationTrack>(this);
    m_vTransformAnimationTracks.push_back(track);
    return track;
}

MorphTargetAnimationTrackPtr AnimationComponent::CreateMorphTargetTrack()
{
    MorphTargetAnimationTrackPtr track = MakeSharedPtr<MorphTargetAnimationTrack>(this);
    m_vMorphTargetAnimationTracks.push_back(track);
    return track;
}

void AnimationComponent::RegisterAnimationCallback(AnimationCallback cb, void* user_data)
{
    m_pAnimationCallback = cb;
    m_pAnimationCallbackData = user_data;
}


SResult AnimationTrack::GetKeyFrameIdxByTime(float & time, uint32_t & kfIdx)
{
    uint32_t kf_count = (uint32_t)m_vKeyFrames.size();

    if (0 == kf_count)
    {
        return ERR_INVALID_DATA;
    }
    else if (1 == kf_count)
    {
        LOG_WARNING("AnimationTrack's KeyFrame cnt is 1, auto insert one");
        m_vKeyFrames.insert(m_vKeyFrames.begin(), m_vKeyFrames[0]);
        m_vKeyFrames[0]->SetTime(0.0);
        kf_count = 2;
    }

    if (time < m_vKeyFrames[0]->GetTime())
        time = m_vKeyFrames[0]->GetTime();
    else if (time > m_vKeyFrames[kf_count - 1]->GetTime())
        time = m_vKeyFrames[kf_count - 1]->GetTime();

    uint32_t i = 0;
    for (i = 0; i < kf_count - 1; i++)
    {
        if (time >= m_vKeyFrames[i]->GetTime() && time <= m_vKeyFrames[i + 1]->GetTime())
        {
            kfIdx = i;
            return S_Success;
        }
    }
    return ERR_INVALID_DATA;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
