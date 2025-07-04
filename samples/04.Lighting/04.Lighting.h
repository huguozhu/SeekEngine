#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class Lighting : public AppFramework
{
public:
    Lighting() :AppFramework("Lighting") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    RHITexturePtr CreateCubeFromEquirectangular(RHITexturePtr tex_equirectangular);
    RHITexturePtr ConvertIrradianceConvolution(RHITexturePtr tex_cube_env);
    void TestSplitSumApproximation(RHITexturePtr tex_cube_env);

private:
    EntityPtr m_pMeshEntity[2] = { nullptr };
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[5] = { nullptr };

    EntityPtr m_pSkyBoxEntity = nullptr;
    EntityPtr m_pPlaneEntity = nullptr;

    EntityPtr m_pSphereEntity = nullptr;
    EntityPtr m_pConeEntity = nullptr;

    FirstPersonCameraController m_CameraController;
};
