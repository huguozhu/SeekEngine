#include <cmath>
#include <float.h>
#include <stdlib.h>
#include "06.GlobalIllumination.h" 

#define SEEK_MACRO_FILE_UID 91     // this code is auto generated, don't touch it!!!


#define DEFAULT_RENDER_WIDTH  1280
#define DEFAULT_RENDER_HEIGHT 720

struct MeshInfo
{
    std::string res_path;
    Matrix4 world_transform = Matrix4::Identity();
};

SResult GlobalIlluminationSample::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(60.0 * Math::DEG2RAD, w/h, 0.01f, 200.0f);
    pCam->SetLookAt(float3(-4, 1, 0), float3(0, 1, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());

    // Step3: add Light Entity
    float3 spot_pos = float3(0, 3, 0);
    float3 look_at = float3(0, 0, 0);
    float cutoff_in = Math::DEG2RAD * 25;
    float cutoff_out = Math::DEG2RAD * 30;
    LightComponentPtr pLight = MakeSharedPtr<SpotLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(look_at - spot_pos);
    pLight->SetWorldTranslation(spot_pos);
    pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
    pLight->SetIntensity(5.24);
    pLight->CastShadow(1);
    pLight->IndirectLighting(1);
    m_pLightEntity = MakeSharedPtr<Entity>(m_pContext.get(), "Spot Light");
    m_pLightEntity->AddSceneComponent(pLight);
    m_pLightEntity->AddToTopScene();

    // Load gltf2 Mesh
    static std::vector<MeshInfo> mesh_infos = {
        { FullPath("asset/gltf/Sponza/Sponza.gltf")},
        { FullPath("asset/gltf/BoomBox/BoomBox.gltf"), Math::Scale(30, 30, 30) * Math::RotationY(90 * Math::DEG2RAD) * Math::Translate(0, 0.5, 0) },
    };
    for (uint32_t i = 0; i< mesh_infos.size(); i++)
    {
        if (m_pGltfMeshEntity[i])
            m_pGltfMeshEntity[i]->DeleteFromTopScene();
        m_pGltfMeshEntity[i] = this->CreateEntityFromFile(mesh_infos[i].res_path);
        if (!m_pGltfMeshEntity)
        {
            LOG_ERROR("CreateEntityFromFile error!");
            return ERR_INVALID_INIT;
        }
        m_pGltfMeshEntity[i]->GetRootComponent()->SetWorldTransform(mesh_infos[i].world_transform);
        m_pGltfMeshEntity[i]->AddToTopScene();
        m_pContext->SceneManagerInstance().PrintTree();
    }

    return S_Success;
}

SResult GlobalIlluminationSample::OnUpdate()
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
SResult GlobalIlluminationSample::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.device = device;
    info.native_wnd = native_wnd;
    info.renderer_type = RendererType::Deferred;
    info.anti_aliasing_mode = AntiAliasingMode::TAA;
    info.gi_mode = GlobalIlluminationMode::RSM;
    info.preferred_adapter = 0;
    info.HDR = true;

    m_pContext = MakeSharedPtr<Context>();
    SEEK_RETIF_FAIL(m_pContext->Init(info));

    return S_Success;
}
int main()
{
    GlobalIlluminationSample theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
