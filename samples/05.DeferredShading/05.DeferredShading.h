#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class DeferredShading : public AppFramework
{
public:
    DeferredShading() :AppFramework("DeferredShading") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);
    
    float3 CalcBestCamPosFromMeshAABBox(AABBox mesh_aabbox);

private:

    EntityPtr m_pMeshEntity[2] = { nullptr };
    EntityPtr m_pGltfMeshEntity = nullptr;
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[5] = { nullptr };

    EntityPtr m_pSkyBoxEntity = nullptr;

    FirstPersonCameraController m_CameraController;
};
