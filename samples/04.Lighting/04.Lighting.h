#pragma once
#include "app_framework.h"
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class Lighting : public AppFramework
{
public:
    Lighting() :AppFramework("Lighting") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    
    float3 CalcBestCamPosFromMeshAABBox(AABBox mesh_aabbox);


private:

    EntityPtr m_pMeshEntity[2] = { nullptr };
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[5] = { nullptr };

    EntityPtr m_pSkyBoxEntity = nullptr;
    EntityPtr m_pPlaneEntity = nullptr;

    EntityPtr m_pSphereEntity = nullptr;
    EntityPtr m_pConeEntity = nullptr;

};
