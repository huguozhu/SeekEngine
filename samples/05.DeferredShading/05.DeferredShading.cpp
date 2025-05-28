#include <cmath>
#include <float.h>
#include <stdlib.h>
#include "05.DeferredShading.h" 

#define SEEK_MACRO_FILE_UID 91     // this code is auto generated, don't touch it!!!


#define DEFAULT_RENDER_WIDTH  1280
#define DEFAULT_RENDER_HEIGHT 720



float3 DeferredShading::CalcBestCamPosFromMeshAABBox(AABBox mesh_aabbox)
{
    float3 max = mesh_aabbox.Max();
    float3 min = mesh_aabbox.Min();
    float length = Math::Length(max - min);
    return float3(0, 0, -length * 1.5);
}


SResult DeferredShading::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    int width = vp.width;
    int height = vp.height;
    float fov = 45.0 * Math::DEG2RAD;
    float aspect = (float)width / (float)height;
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(fov, aspect, 0.1f, 200.0f);
    pCam->SetLookAt(float3(0, 2, 0), float3(1, 2, -1), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    m_CameraController.SetCamera(pCam.get());

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
        FullPath("asset/Sponza/Sponza.gltf"),
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
    desc.flags = RESOURCE_FLAG_SHADER_RESOURCE;
    std::vector<BitmapBufferPtr> datas(6, nullptr);
    std::string cube_files[6] = {
        FullPath("asset/textures/skybox/positive_x.png"),
        FullPath("asset/textures/skybox/negative_x.png"),
        FullPath("asset/textures/skybox/positive_y.png"),
        FullPath("asset/textures/skybox/negative_y.png"),
        FullPath("asset/textures/skybox/positive_z.png"),
        FullPath("asset/textures/skybox/negative_z.png"),
    };
    for (uint32_t i = 0; i < 6; i++)
    {
        BitmapBufferPtr bit = ImageDecodeFromFile(cube_files[i], ImageType::PNG);
        datas[i] = bit;
    }
    RHITexturePtr tex_cube = rc.CreateTextureCube(desc, &datas);
    m_pSkyBoxEntity = MakeSharedPtr<Entity>(m_pContext.get());
    SkyBoxComponentPtr pSkybox = MakeSharedPtr<SkyBoxComponent>(m_pContext.get());
    pSkybox->SetSkyBoxTex(tex_cube);
    m_pSkyBoxEntity->AddSceneComponent(pSkybox);
    //m_pSkyBoxEntity->AddToTopScene();

#if (0)
    std::string equirectangular_file = FullPath("asset/textures/Hamarikyu_Bridge.jpg");
    BitmapBufferPtr equirectangular_bitmap = ImageDecodeFromFile(equirectangular_file, ImageType::JPEG);
    Texture::Desc equirectangular_desc;
    equirectangular_desc.type = TextureType::Tex2D;
    equirectangular_desc.width = equirectangular_bitmap->Width();
    equirectangular_desc.height = equirectangular_bitmap->Height();
    equirectangular_desc.depth = 1;
    equirectangular_desc.num_mips = 1;
    equirectangular_desc.num_samples = 1;
    equirectangular_desc.format = equirectangular_bitmap->Format();
    equirectangular_desc.flags = RESOURCE_FLAG_SHADER_RESOURCE;    
    
    TexturePtr tex_equirectangular = rc.CreateTexture2D(equirectangular_desc, equirectangular_bitmap);
    TexturePtr tex_cube_env = this->ConvertEquirectangularToCubeMap(tex_equirectangular);
    m_pSkyBoxEntity->SetSkyBoxTex(tex_cube_env);

    TexturePtr tex_ir_conv = this->ConvertIrradianceConvolution(tex_cube_env);
    m_pSkyBoxEntity->SetSkyBoxTex(tex_ir_conv);

    this->TestSplitSumApproximation(tex_cube_env);

#endif
#endif
    return S_Success;
}

SResult DeferredShading::OnUpdate()
{
    m_CameraController.Update(m_pContext->GetDeltaTime());
    return m_pContext->Update();
}
SResult DeferredShading::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.debug = false;
    info.device = device;
    info.native_wnd = native_wnd;
    info.lighting_mode = LightingMode::Phong;
    info.renderer_type = RendererType::Deferred;
    info.anti_aliasing_mode = AntiAliasingMode::TAA;
    info.preferred_adapter = 0;
    info.HDR = true;

    m_pContext = MakeSharedPtr<Context>();
    SEEK_RETIF_FAIL(m_pContext->Init(info));

    return S_Success;
}
int main()
{
    DeferredShading theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
