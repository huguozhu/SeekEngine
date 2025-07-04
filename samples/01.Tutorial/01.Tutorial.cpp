#include "01.Tutorial.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!

Tutorial::Tutorial()
    :AppFramework("Tutorial")
{
}

Tutorial::~Tutorial()
{
    
}

SResult Tutorial::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    // Camera
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
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
