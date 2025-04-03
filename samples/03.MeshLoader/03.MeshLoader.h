#pragma once
#include "app_framework.h"
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class MeshLoader : public AppFramework
{
public:
    MeshLoader();
    virtual ~MeshLoader();

    virtual SResult         OnCreate() override;
    virtual SResult         OnUpdate() override;

    EntityPtr CreateEntityFromFile(std::string filePath);

private:
    LoaderPtr           m_pGLTFLoader = nullptr;

    EntityPtr           m_pCameraEntity = nullptr;

    EntityPtr           m_pMeshEntity = nullptr;

    SphereMeshComponentPtr    m_pMesh = nullptr;

};

