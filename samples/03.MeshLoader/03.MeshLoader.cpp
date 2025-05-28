#include "03.MeshLoader.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


#define DEFAULT_RENDER_WIDTH  1280
#define DEFAULT_RENDER_HEIGHT 720

MeshLoader::MeshLoader()
    :AppFramework("MeshLoader")
{
}

MeshLoader::~MeshLoader()
{
    
}

SResult MeshLoader::OnCreate()
{
    // Camera
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, DEFAULT_RENDER_WIDTH * 1.0f / DEFAULT_RENDER_HEIGHT, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 2, 0), float3(1, 2, -1), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    // Load gltf2 Mesh
    static std::vector<std::string> model_files = {
        FullPath("asset/Sponza/Sponza.gltf"),
    };
    static int model_selected = 0;

    // Load Model
    if (model_selected >= 0)
    {
        if (m_pMeshEntity)
            m_pMeshEntity->DeleteFromTopScene();
        m_pMeshEntity = this->CreateEntityFromFile(model_files[model_selected]);
        if (!m_pMeshEntity)
        {
            LOG_ERROR("CreateEntityFromFile error!");
            return ERR_INVALID_INIT;
        }
        m_pMeshEntity->AddToTopScene();
        m_pContext->SceneManagerInstance().PrintTree();
        model_selected = -1;
    }

    return S_Success;
}
SResult MeshLoader::OnUpdate()
{    
    return m_pContext->Update();
}

int main()
{
    MeshLoader app;
    return APP_RUN(&app);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
