#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

#define NUM_LIGHTS 128

class DeferredShading : public AppFramework
{
public:
    DeferredShading() :AppFramework("DeferredShading") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    void    InitRotatingLights();
    void    UpdateRotatingLights();

private:
    EntityPtr m_pMeshEntity = nullptr;
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[5] = { nullptr };
    EntityPtr m_pSkyBoxEntity = nullptr;

    EntityPtr m_pRotatingLights[NUM_LIGHTS] = { nullptr };
    EntityPtr m_pRotatingSphere[NUM_LIGHTS] = { nullptr };

    FirstPersonCameraController m_CameraController;
};

void DeferredShading::InitRotatingLights()
{
    for (uint32_t i = 0; i < NUM_LIGHTS; i++)
    {
        float3 pos = float3();
        float3 color_f3 = Math::GenerateRandom(float3(0.0f), float3(1.0f));
        Color color(color_f3);
        LightComponentPtr pLight = LightComponent::CreateLightComponent(m_pContext.get(), LightType::Point);
        pLight->SetColor(color);
        pLight->SetIntensity(2);
        if (i % 5 == 0)
            pLight->SetEnable(false);
        m_pRotatingLights[i] = MakeSharedPtr<Entity>(m_pContext.get(), "LightType::Point");
        m_pRotatingLights[i]->AddSceneComponent(pLight);
        m_pRotatingLights[i]->SetWorldTranslation(pos);
        m_pRotatingLights[i]->AddToTopScene();

        SphereMeshComponentPtr pSphere = MakeSharedPtr<SphereMeshComponent>(m_pContext.get(), 16, 16);
        MaterialPtr material = pSphere->GetMeshByIndex(0)->GetMaterial();
        material->albedo_factor = color.ToFloat4();
        m_pRotatingSphere[i] = MakeSharedPtr<Entity>(m_pContext.get());
        m_pRotatingSphere[i]->AddSceneComponent(pSphere);
        m_pRotatingSphere[i]->AddToParent(m_pRotatingLights[i].get());
        m_pRotatingSphere[i]->SetLocalScale(0.05);
    }
}
void DeferredShading::UpdateRotatingLights()
{
    for (uint32_t i = 0; i < NUM_LIGHTS; i++)
    {
        static float first_time = m_pContext->GetCurTime();
        float cur_time = m_pContext->GetCurTime() - first_time;
        float factor = (50.0f + cur_time * 0.6f) / NUM_LIGHTS;
        float x = 2.0f * sin(factor * i);
        float y = 0.1 + (i / 10) * 0.25;
        float z = 2.0f * cos(factor * i);
        Matrix4 m = Math::Translate(x, y, z);
        m_pRotatingLights[i]->GetRootComponent()->SetLocalTransform(m);
    }
}
SResult DeferredShading::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(45.0 * Math::DEG2RAD, w / h, 0.01f, 200.0f);
    pCam->SetLookAt(float3(-6.2, 5.8, -1.1), float3(-6.0, 5.6, -1.1), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());
    m_CameraController.SetMoveSpeed(0.1);

    // Step3: add Light Entity
    Color c = Color::DefaultAmbientColor;
    LightComponentPtr pLight = MakeSharedPtr<AmbientLightComponent>(m_pContext.get());
    pLight->SetColor(c);
    m_pLightEntity[0] = MakeSharedPtr<Entity>(m_pContext.get(), "Ambient Light");
    m_pLightEntity[0]->AddSceneComponent(pLight);
    m_pLightEntity[0]->AddToTopScene();

    float p = 1.0;
    pLight = MakeSharedPtr<DirectionalLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(float3(p, -p, p));
    pLight->SetIntensity(0.5);
    pLight->CastShadow(1);
	pLight->SoftShadow(1);
	pLight->SetWorldTranslation(float3(-p, p, -p));
    //pLight->CascadedShadow(1);
    m_pLightEntity[1] = MakeSharedPtr<Entity>(m_pContext.get(), "Directional Light");
    m_pLightEntity[1]->AddSceneComponent(pLight);
    //m_pLightEntity[1]->AddToTopScene();

    float3 spot_pos = float3(-p, p, -p);
    float3 look_at = float3(0, 0, 0);
    float cutoff_in = Math::DEG2RAD * 25;
    float cutoff_out = Math::DEG2RAD * 45;
    pLight = MakeSharedPtr<SpotLightComponent>(m_pContext.get());
    pLight->SetColor(Color::Blue);
    pLight->SetDirection(look_at - spot_pos);
	pLight->SetWorldTranslation(spot_pos);
    pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
    pLight->SetIntensity(50.24);
    pLight->CastShadow(1);
    m_pLightEntity[2] = MakeSharedPtr<Entity>(m_pContext.get(), "Spot Light");
    m_pLightEntity[2]->AddSceneComponent(pLight);
    //m_pLightEntity[2]->AddToTopScene();

    spot_pos = float3(p, p, -p);
    look_at = float3(0, 0, 0);
    pLight = MakeSharedPtr<SpotLightComponent>(m_pContext.get());
    pLight->SetColor(Color::Red);
    pLight->SetDirection(look_at - spot_pos);
    pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
    pLight->SetIntensity(5.24);
    pLight->CastShadow(1);
    pLight->SetWorldTranslation(spot_pos);
    m_pLightEntity[3] = MakeSharedPtr<Entity>(m_pContext.get(), "Spot Light");
    m_pLightEntity[3]->AddSceneComponent(pLight);
    //m_pLightEntity[3]->AddToTopScene();

    float3 point_pos = float3(0, 3.0, 0);
    pLight = MakeSharedPtr<PointLightComponent>(m_pContext.get());
    pLight->SetColor(Color::Green);    
    pLight->SetIntensity(45);
    pLight->CastShadow(1);
    //pLight->SoftShadow(1);
    pLight->SetFalloffRadius(15.4);
	pLight->SetWorldTranslation(point_pos);
    m_pLightEntity[4] = MakeSharedPtr<Entity>(m_pContext.get(), "Point Light");
    m_pLightEntity[4]->AddSceneComponent(pLight);
    //m_pLightEntity[4]->AddToTopScene();

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
            return -1;
        }
        m_pMeshEntity->AddToTopScene();
        m_pContext->SceneManagerInstance().PrintTree();
        model_selected = -1;
    }

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
    RHITexturePtr tex_cube = rc.CreateTextureCube(desc, datas);
    m_pSkyBoxEntity = MakeSharedPtr<Entity>(m_pContext.get());
    SkyBoxComponentPtr pSkybox = MakeSharedPtr<SkyBoxComponent>(m_pContext.get());
    pSkybox->SetSkyBoxTex(tex_cube);
    m_pSkyBoxEntity->AddSceneComponent(pSkybox);
    m_pSkyBoxEntity->AddToTopScene();

    return S_Success;
}

SResult DeferredShading::OnUpdate()
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
SResult DeferredShading::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.enable_debug = true;
    info.lighting_mode = LightingMode::PBR;
    info.renderer_type = RendererType::Deferred;
    info.anti_aliasing_mode = AntiAliasingMode::TAA;
    info.preferred_adapter = 0;
    info.HDR = true;

    m_pContext = MakeSharedPtr<Context>(info);
    SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));

    return S_Success;
}
int main()
{
    DeferredShading theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
