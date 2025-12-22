#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class GlobalIlluminationSample : public AppFramework
{
public:
    GlobalIlluminationSample() :AppFramework("GlobalIlluminationSample") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    void AddSkyboxEntity();

private:
    EntityPtr m_pGltfMeshEntity[10] = { nullptr };
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity = nullptr;
    EntityPtr m_pSkyBoxEntity = nullptr;

    FirstPersonCameraController m_CameraController;
};


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
    pCam->SetLookAt(float3(-5, 1, 0), float3(0, 1, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());

    // Step3: add Light Entity
    float3 spot_pos = float3(0, 4, 0);
    float3 look_at = float3(0, 0, 0);
    float cutoff_in = Math::DEG2RAD * 25;
    float cutoff_out = Math::DEG2RAD * 30;
    LightComponentPtr pLight = MakeSharedPtr<SpotLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(look_at - spot_pos);
    pLight->SetWorldTranslation(spot_pos);
    pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
    pLight->SetIntensity(40.24);
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
            return -1;
        }
        m_pGltfMeshEntity[i]->GetRootComponent()->SetWorldTransform(mesh_infos[i].world_transform);
        m_pGltfMeshEntity[i]->AddToTopScene();
        m_pContext->SceneManagerInstance().PrintTree();
    }
    this->AddSkyboxEntity();

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
    info.rhi_type = RHIType::D3D11;
    info.enable_debug = true;
    info.renderer_type = RendererType::Deferred;
    info.anti_aliasing_mode = AntiAliasingMode::TAA;
    info.gi_mode = GlobalIlluminationMode::LPV;
    info.preferred_adapter = 0;
    info.HDR = true;

    m_pContext = MakeSharedPtr<Context>(info);
    SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));

    return S_Success;
}
void GlobalIlluminationSample::AddSkyboxEntity()
{
    // Step4 add SkyBox Entity
    RHITexture::Desc desc;
    desc.type = TextureType::Cube;
    desc.width = 1024;
    desc.height = 1024;
    desc.depth = 1;
    desc.num_mips = 1;
    desc.num_samples = 1;
    desc.format = PixelFormat::R8G8B8A8_UNORM;
    desc.flags = RESOURCE_FLAG_GPU_READ;
    std::vector<BitmapBufferPtr> datas(6, nullptr);
    std::string cube_files[6] = {
        FullPath("asset/textures/skybox/daylight0.png"),
        FullPath("asset/textures/skybox/daylight1.png"),
        FullPath("asset/textures/skybox/daylight2.png"),
        FullPath("asset/textures/skybox/daylight3.png"),
        FullPath("asset/textures/skybox/daylight4.png"),
        FullPath("asset/textures/skybox/daylight5.png"),
    };
    for (uint32_t i = 0; i < 6; i++)
    {
        BitmapBufferPtr bit = ImageDecodeFromFile(cube_files[i], ImageType::PNG);
        datas[i] = bit;
    }
    RHITexturePtr tex_cube = m_pContext->RHIContextInstance().CreateTextureCube(desc, datas);
    m_pSkyBoxEntity = MakeSharedPtr<Entity>(m_pContext.get());
    SkyBoxComponentPtr pSkybox = MakeSharedPtr<SkyBoxComponent>(m_pContext.get());
    pSkybox->SetSkyBoxTex(tex_cube);
    m_pSkyBoxEntity->AddSceneComponent(pSkybox);
    m_pSkyBoxEntity->AddToTopScene();
}
int main()
{
    GlobalIlluminationSample theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
