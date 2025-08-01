#include "05.DeferredShading.h" 

#define SEEK_MACRO_FILE_UID 91     // this code is auto generated, don't touch it!!!


SResult DeferredShading::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(60.0 * Math::DEG2RAD, w / h, 0.01f, 200.0f);
    pCam->SetLookAt(float3(0, 2, 0), float3(1, 2, -1), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    m_CameraController.SetCamera(pCam.get());
    m_CameraController.SetMoveSpeed(0.1);

    // Step3: add Light Entity
    // NOTE: if the lighting result is wrong, check if hdr is enabled(now, it's disabled for phong)
    Color c = Color::DefaultAmbientColor;
    LightComponentPtr pLight = MakeSharedPtr<AmbientLightComponent>(m_pContext.get());
    pLight->SetColor(c);
    m_pLightEntity[0] = MakeSharedPtr<Entity>(m_pContext.get(), "Ambient Light");
    m_pLightEntity[0]->AddSceneComponent(pLight);
    //m_pLightEntity[0]->AddToTopScene();

    float p = 3.0;
    float3 directional_pos = float3(-p, p, p);
    pLight = MakeSharedPtr<DirectionalLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(float3(p, -p, -p));
    pLight->SetIntensity(0.5);
    pLight->CastShadow(1);
	pLight->SoftShadow(1);
	pLight->SetWorldTranslation(directional_pos); 
    //pLight->CascadedShadow(1);
    m_pLightEntity[1] = MakeSharedPtr<Entity>(m_pContext.get(), "Directional Light");
    m_pLightEntity[1]->AddSceneComponent(pLight);
    //m_pLightEntity[1]->AddToTopScene();

    float3 spot_pos = float3(-p, p, 0);
    float3 look_at = float3(0, 0, 0);
    float cutoff_in = Math::DEG2RAD * 25;
    float cutoff_out = Math::DEG2RAD * 45;
    pLight = MakeSharedPtr<SpotLightComponent>(m_pContext.get());
    pLight->SetColor(Color::Red);
    pLight->SetDirection(look_at - spot_pos);
	pLight->SetWorldTranslation(spot_pos);
    pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
    pLight->SetIntensity(50.24);
    pLight->CastShadow(1);
	pLight->SoftShadow(1);
    m_pLightEntity[2] = MakeSharedPtr<Entity>(m_pContext.get(), "Spot Light");
    m_pLightEntity[2]->AddSceneComponent(pLight);
    //m_pLightEntity[2]->AddToTopScene();

    spot_pos = float3(p, p, -p);
    look_at = float3(0, 0, 0);
    pLight = MakeSharedPtr<SpotLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(look_at - spot_pos);
    pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
    pLight->SetIntensity(5.24);
    //pLight->CastShadow(1);
    //pLight->SoftShadow(1);
    pLight->SetWorldTranslation(spot_pos);
    m_pLightEntity[3] = MakeSharedPtr<Entity>(m_pContext.get(), "Spot Light");
    m_pLightEntity[3]->AddSceneComponent(pLight);
    m_pLightEntity[3]->AddToTopScene();

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
        if (m_pGltfMeshEntity)
            m_pGltfMeshEntity->DeleteFromTopScene();
        m_pGltfMeshEntity = this->CreateEntityFromFile(model_files[model_selected]);
        if (!m_pGltfMeshEntity)
        {
            LOG_ERROR("CreateEntityFromFile error!");
            return ERR_INVALID_INIT;
        }
        m_pGltfMeshEntity->AddToTopScene();
        m_pContext->SceneManagerInstance().PrintTree();
        model_selected = -1;
    }

#ifdef SEEK_PLATFORM_WINDOWS
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
        FullPath("asset/textures/skybox/positive_x.jpg"),
        FullPath("asset/textures/skybox/negative_x.jpg"),
        FullPath("asset/textures/skybox/positive_y.jpg"),
        FullPath("asset/textures/skybox/negative_y.jpg"),
        FullPath("asset/textures/skybox/positive_z.jpg"),
        FullPath("asset/textures/skybox/negative_z.jpg"),
    };
    for (uint32_t i = 0; i < 6; i++)
    {
        BitmapBufferPtr bit = ImageDecodeFromFile(cube_files[i], ImageType::JPEG);
        datas[i] = bit;
    }
    RHITexturePtr tex_cube = rc.CreateTextureCube(desc, datas);
    m_pSkyBoxEntity = MakeSharedPtr<Entity>(m_pContext.get());
    SkyBoxComponentPtr pSkybox = MakeSharedPtr<SkyBoxComponent>(m_pContext.get());
    pSkybox->SetSkyBoxTex(tex_cube);
    m_pSkyBoxEntity->AddSceneComponent(pSkybox);
    //m_pSkyBoxEntity->AddToTopScene();

#endif
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
    info.lighting_mode = LightingMode::Phong;
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
