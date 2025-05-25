#pragma once

#include "app_framework.h"
#include <string>

SEEK_NAMESPACE_BEGIN

class FirstPersonCameraController
{
public:
    FirstPersonCameraController() = default;
    FirstPersonCameraController& operator=(const FirstPersonCameraController&) = delete;

    void SetCamera(CameraComponent* pCamera) { m_pCamera = pCamera; }
    virtual ~FirstPersonCameraController() {}
    virtual void Update(float delta_time);

private:
    CameraComponent* m_pCamera = nullptr;
    float m_fMoveSpeed = 5.0;
    float m_fMouseSensitivityX = 0.002f;
    float m_fMouseSensitivityY = 0.002f;

    float m_fCurrentYaw = 0.0f;
    float m_fCurrentPitch = 0.0f;

    float3 m_MoveDir;
    float m_fMoveVelocity = 0.0f;
    float m_fVelocityDrag = 0.0f;
    float m_fTotalDragTimeToZero = 0.25f;
    float m_fDragTimer = 0.0f;
};


SEEK_NAMESPACE_END
