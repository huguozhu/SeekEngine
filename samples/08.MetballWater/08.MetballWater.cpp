
#include "08.MetballWater.h" 

#define SEEK_MACRO_FILE_UID 91     // this code is auto generated, don't touch it!!!

SResult MetaballWater::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    m_pEntity = MakeSharedPtr<Entity>(m_pContext.get(), "CameraEntity");
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 0, -5), float3(0, 0, 0), float3(0, 1, 0));
    m_pEntity->AddSceneComponent(pCam);
    m_pEntity->AddToTopScene();

    Metaball2DComponentPtr pMetaball = MakeSharedPtr<Metaball2DComponent>(m_pContext.get(), (uint32_t)w, (uint32_t)h);
    m_pContext->SceneManagerInstance().AddSprite2DComponent(pMetaball);

    return S_Success;
}

SResult MetaballWater::OnUpdate()
{
    SEEK_RETIF_FAIL(m_pContext->Tick());
    SEEK_RETIF_FAIL(m_pContext->BeginRender());
    SEEK_RETIF_FAIL(m_pContext->RenderFrame());
    IMGUI_Begin();
    IMGUI_Rendering();
    SEEK_RETIF_FAIL(m_pContext->EndRender());
    return S_Success;
}
SResult MetaballWater::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info = {};
    info.rhi_type = RHIType::D3D11;
    info.enable_debug = true;
    info.renderer_type = RendererType::Forward;
    info.anti_aliasing_mode = AntiAliasingMode::TAA;
    info.gi_mode = GlobalIlluminationMode::LPV;
    info.preferred_adapter = 0;
    info.HDR = true;

    m_pContext = MakeSharedPtr<Context>(info);
    SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));

    return S_Success;
}
int main()
{
    MetaballWater theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
