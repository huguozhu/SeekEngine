#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class GlobalIlluminationSample : public AppFramework
{
public:
    GlobalIlluminationSample() :AppFramework("GlobalIlluminationSample") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    void AddSkyboxEntity();

private:

    EntityPtr m_pMeshEntity[2] = { nullptr };
    EntityPtr m_pGltfMeshEntity[10] = { nullptr };
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity =  nullptr;
    EntityPtr m_pSkyBoxEntity = nullptr;

    FirstPersonCameraController m_CameraController;
};
