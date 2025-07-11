#include "03.MeshLoader.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!

MeshLoader::MeshLoader()
    :AppFramework("MeshLoader")
{
}

MeshLoader::~MeshLoader()
{
    
}

SResult MeshLoader::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    // Camera
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w/h, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 2, 0), float3(1, 2, -1), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());
    m_CameraController.SetMoveSpeed(0.2);

    // Load gltf2 Mesh
    static std::vector<std::string> model_files = {
        FullPath("asset/gltf/Sponza/Sponza.gltf"),
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
    m_CameraController.Update(m_pContext->GetDeltaTime());
    SEEK_RETIF_FAIL(m_pContext->Tick());
    SEEK_RETIF_FAIL(m_pContext->BeginRender());
    SEEK_RETIF_FAIL(m_pContext->RenderFrame());
    IMGUI_Begin();
    IMGUI_Rendering();
    SEEK_RETIF_FAIL(m_pContext->EndRender());
    return S_Success;
}

int main()
{
    MeshLoader app;
    return APP_RUN(&app);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
