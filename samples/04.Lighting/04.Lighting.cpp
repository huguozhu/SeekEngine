#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

class Lighting : public AppFramework
{
public:
    Lighting() :AppFramework("Lighting") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    RHITexturePtr CreateCubeFromEquirectangular(RHITexturePtr tex_equirectangular);
    RHITexturePtr ConvertIrradianceConvolution(RHITexturePtr tex_cube_env);
    void TestSplitSumApproximation(RHITexturePtr tex_cube_env);

private:
    EntityPtr m_pMeshEntity[2] = { nullptr };
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[5] = { nullptr };

    EntityPtr m_pSkyBoxEntity = nullptr;
    EntityPtr m_pPlaneEntity = nullptr;

    EntityPtr m_pSphereEntity = nullptr;
    EntityPtr m_pConeEntity = nullptr;

    FirstPersonCameraController m_CameraController;
};


SResult Lighting::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(60 * Math::DEG2RAD, w/h, 0.1f, 200.0f);
    pCam->SetLookAt(float3(0, 4, -4), float3(0, 0, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());
    m_CameraController.SetMoveSpeed(0.1f);

    // Step3: add Light Entity
    // NOTE: if the lighting result is wrong, check if hdr is enabled(now, it's disabled for phong)
    Color c = Color::DefaultAmbientColor;
    LightComponentPtr pLight = MakeSharedPtr<AmbientLightComponent>(m_pContext.get());
    pLight->SetColor(c);
    m_pLightEntity[0] = MakeSharedPtr<Entity>(m_pContext.get(), "Ambient Light");
    m_pLightEntity[0]->AddSceneComponent(pLight);
    m_pLightEntity[0]->AddToTopScene();

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
    m_pLightEntity[1]->AddToTopScene();

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
    m_pLightEntity[2]->AddToTopScene();

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
    pLight->SoftShadow(1);
    pLight->SetFalloffRadius(15.4);
	pLight->SetWorldTranslation(point_pos);
    m_pLightEntity[4] = MakeSharedPtr<Entity>(m_pContext.get(), "Point Light");
    m_pLightEntity[4]->AddSceneComponent(pLight);
    m_pLightEntity[4]->AddToTopScene();

    BitmapBufferPtr bit = ImageDecodeFromFile(FullPath("asset/textures/boy.jpg"), ImageType::JPEG);
    RHITexturePtr tex_0 = rc.CreateTexture2D(bit);

    PlaneMeshComponentPtr pPlane = MakeSharedPtr<PlaneMeshComponent>(m_pContext.get(), 7.6f, 7.6f);
    MaterialPtr material = pPlane->GetMeshByIndex(0)->GetMaterial();
    material->albedo_tex = tex_0;
    m_pPlaneEntity = MakeSharedPtr<Entity>(m_pContext.get());
    m_pPlaneEntity->SetWorldTranslation(float3(0, -0.5, 0));
    m_pPlaneEntity->AddSceneComponent(pPlane);
    m_pPlaneEntity->AddToTopScene();


    SphereMeshComponentPtr pSphere = MakeSharedPtr<SphereMeshComponent>(m_pContext.get());
    pSphere->SetWorldTranslation(float3(-2, 0.5, 0));
    pSphere->SetWorldScale(float3(1.0f));
    material = pSphere->GetMeshByIndex(0)->GetMaterial();
    material->albedo_tex = tex_0;
    m_pSphereEntity = MakeSharedPtr<Entity>(m_pContext.get());
    m_pSphereEntity->AddSceneComponent(pSphere);    
    m_pSphereEntity->AddToTopScene();


    ConeMeshComponentPtr pCone = MakeSharedPtr<ConeMeshComponent>(m_pContext.get());
	pCone->SetWorldTranslation(float3(2, 1.0, 0));
    material = pCone->GetMeshByIndex(0)->GetMaterial();
    material->albedo_tex = tex_0;
    m_pConeEntity = MakeSharedPtr<Entity>(m_pContext.get());
    m_pConeEntity->AddSceneComponent(pCone);
    m_pConeEntity->AddToTopScene();
    
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

#if (0)
    std::string equirectangular_file = FullPath("asset/textures/Hamarikyu_Bridge.jpg");
    BitmapBufferPtr equirectangular_bitmap = ImageDecodeFromFile(equirectangular_file, ImageType::JPEG);
    RHITexture::Desc equirectangular_desc;
    equirectangular_desc.type = TextureType::Tex2D;
    equirectangular_desc.width = equirectangular_bitmap->Width();
    equirectangular_desc.height = equirectangular_bitmap->Height();
    equirectangular_desc.depth = 1;
    equirectangular_desc.num_mips = 1;
    equirectangular_desc.num_samples = 1;
    equirectangular_desc.format = equirectangular_bitmap->Format();
    equirectangular_desc.flags = RESOURCE_FLAG_GPU_READ;    
    
    RHITexturePtr tex_equirectangular = rc.CreateTexture2D(equirectangular_desc, equirectangular_bitmap);
    RHITexturePtr tex_cube_env = this->CreateCubeFromEquirectangular(tex_equirectangular);
    pSkybox->SetSkyBoxTex(tex_cube_env);

    RHITexturePtr tex_ir_conv = this->ConvertIrradianceConvolution(tex_cube_env);
    //pSkybox->SetSkyBoxTex(tex_ir_conv);

    //this->TestSplitSumApproximation(tex_cube_env);

#endif

    return S_Success;
}

SResult Lighting::OnUpdate()
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
SResult Lighting::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.enable_debug = true;
    info.renderer_type = RendererType::Forward;
    info.lighting_mode = LightingMode::Phong;
    info.preferred_adapter = 0;
    info.HDR = true;

    m_pContext = MakeSharedPtr<Context>(info);
    SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));

    return S_Success;
}
RHITexturePtr Lighting::CreateCubeFromEquirectangular(RHITexturePtr tex_equirectangular)
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    RHITexture::Desc cube_desc = { TextureType::Cube, 512, 512, 1, 1, 1, 1, PixelFormat::R8G8B8A8_UNORM,
        RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE };
    RHITexturePtr tex_cube = rc.CreateTextureCube(cube_desc);

    EquirectangularToCubeMapPostProcessPtr equirectangular_2_cubemap_pp = MakeSharedPtr<EquirectangularToCubeMapPostProcess>(m_pContext.get());
    equirectangular_2_cubemap_pp->SetSrcTexture(tex_equirectangular);
    equirectangular_2_cubemap_pp->SetDstTexture(tex_cube);
    equirectangular_2_cubemap_pp->Run();

    return tex_cube;
}
RHITexturePtr Lighting::ConvertIrradianceConvolution(RHITexturePtr tex_cube_env)
{
    IrradianceConvolutionPostProcessPtr ir_convolution_pp = MakeSharedPtr<IrradianceConvolutionPostProcess>(m_pContext.get());
    ir_convolution_pp->SetSrcTexture(tex_cube_env);
    ir_convolution_pp->Run();

    return ir_convolution_pp->GetIrradianceConvolutionTex();
}
void Lighting::TestSplitSumApproximation(RHITexturePtr tex_cube_env)
{
    RHIContext& rc = m_pContext->RHIContextInstance();

    SplitSumApproximationPostProcessPtr ssa_pp = MakeSharedPtr<SplitSumApproximationPostProcess>(m_pContext.get());
    ssa_pp->SetSrcEnvTex(tex_cube_env);
    SResult res = ssa_pp->Run();
    res = res;

    RHITexturePtr tex_prefilter = ssa_pp->GetPreFilterTexture();
    ((SkyBoxComponent*)m_pSkyBoxEntity->GetComponent(ComponentType::SkyBox))->SetSkyBoxTex(tex_prefilter);
}

int main()
{
    Lighting theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
