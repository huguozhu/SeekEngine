#include "first_person_camera_controller.h"

SEEK_NAMESPACE_BEGIN

void FirstPersonCameraController::Update(float delta_time)
{
    if (!m_pCamera)
        return;

    ImGuiIO& io = ImGui::GetIO();
    
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        static const float MAX_RAG = 88.0f * Math::DEG2RAD;
        float yaw = io.MouseDelta.x * m_fMouseSensitivityX;
        float pitch = io.MouseDelta.y * m_fMouseSensitivityY;
        float3 euler_rot = m_pCamera->GetWorldEulerRotation();
        euler_rot[0] += pitch;
        euler_rot[1] += yaw;
        euler_rot[2] = 0.0f;
        if (euler_rot[0] > MAX_RAG)
            euler_rot[0] = MAX_RAG;
        if (euler_rot[0] < -MAX_RAG)
            euler_rot[0] = -MAX_RAG;
        m_pCamera->SetWorldEulerRotation(euler_rot);
    }

    int forward = (
        (ImGui::IsKeyDown(ImGuiKey_W) ? 1 : 0) +
        (ImGui::IsKeyDown(ImGuiKey_S) ? -1 : 0)
        );
    int strafe = (
        (ImGui::IsKeyDown(ImGuiKey_A) ? -1 : 0) +
        (ImGui::IsKeyDown(ImGuiKey_D) ? 1 : 0)
        );

    if (forward || strafe)
    {
        float3 dir = m_pCamera->GetWorldForwardVec() * (float)forward + m_pCamera->GetWorldRightVec() * (float)strafe;
        m_MoveDir = dir;
        m_fMoveVelocity = m_fMoveSpeed;
        m_fDragTimer = m_fTotalDragTimeToZero;
        m_fVelocityDrag = m_fMoveSpeed / m_fDragTimer;
    }
    else
    {
        if (m_fDragTimer > 0.0f)
        {
            m_fDragTimer -= delta_time;
            m_fMoveVelocity -= m_fVelocityDrag * delta_time;
        }
        else
        {
            m_fMoveVelocity = 0.0f;
        }
    }    
    m_pCamera->LocalTranslate(Math::Normalize(m_MoveDir) * m_fMoveVelocity * delta_time);
}

SEEK_NAMESPACE_END
