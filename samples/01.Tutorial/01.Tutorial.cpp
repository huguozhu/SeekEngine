#include "01.Tutorial.h"

Tutorial::Tutorial()
    :AppFramework("Tutorial")
{
}

SResult Tutorial::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;
    
    m_pEntity = MakeSharedPtr<Entity>(m_pContext.get());
    m_pEntity->AddToTopScene();

    // Camera
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 2, 3), float3(0, 0, 0), float3(0, 1, 0));
    m_pEntity->AddSceneComponent(pCam);

    // Mesh
    ConeMeshComponentPtr pMesh = MakeSharedPtr<ConeMeshComponent>(m_pContext.get());
    m_pEntity->AddSceneComponent(pMesh);

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
