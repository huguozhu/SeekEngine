#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class MeshLoader : public AppFramework
{
public:
    MeshLoader();
    virtual ~MeshLoader();

    virtual SResult         OnCreate() override;
    virtual SResult         OnUpdate() override;

private:
    EntityPtr                   m_pCameraEntity = nullptr;
    EntityPtr                   m_pMeshEntity = nullptr;
    SphereMeshComponentPtr      m_pMesh = nullptr;
    FirstPersonCameraController m_CameraController;
};

