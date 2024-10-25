#pragma once
#include "app_framework.h"
#include "seek_engine.h"
#include <cmath>
#include <fstream>
#include <sstream>

USING_NAMESPACE_SEEK

class Particles : public AppFramework
{
public:
    Particles() :AppFramework("Particles") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;

    void CreateParticleEntities();

private:
    // Camera
    EntityPtr           m_pCameraEntity = nullptr;
    // Skybox
    EntityPtr           m_pSkyBoxEntity = nullptr;


    double              m_dStartTime = 0;
    
    std::vector<EntityPtr>              m_ParticleList;
    std::vector<ParticleComponentPtr>   m_pParticleComponentList;
    std::vector<BitmapBufferPtr>        m_vBitmaps;
};