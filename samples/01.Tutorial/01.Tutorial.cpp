#include "01.Tutorial.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


#define DEFAULT_RENDER_WIDTH  1280
#define DEFAULT_RENDER_HEIGHT 720

Tutorial::Tutorial()
    :AppFramework("Tutorial")
{
}

Tutorial::~Tutorial()
{
    
}

SResult Tutorial::OnCreate()
{
    // Camera
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, DEFAULT_RENDER_WIDTH * 1.0f / DEFAULT_RENDER_HEIGHT, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 2, 3), float3(0, 0, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    // Mesh
    m_pMeshEntity = MakeSharedPtr<Entity>(m_pContext.get());
    ConeMeshComponentPtr pMesh = MakeSharedPtr<ConeMeshComponent>(m_pContext.get());
    m_pMeshEntity->AddSceneComponent(pMesh);
    m_pMeshEntity->AddToTopScene();

    return S_Success;
}
SResult Tutorial::OnUpdate()
{    
    return m_pContext->Update();
}


int main()
{
    Tutorial app;
    return APP_RUN(&app);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
