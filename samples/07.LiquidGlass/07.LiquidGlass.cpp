
#include "07.LiquidGlass.h"

#define SEEK_MACRO_FILE_UID 91     // this code is auto generated, don't touch it!!!


static int g_iClickDownId = 0;
static int g_iMoveId = 0;
static int g_iClickUpId = 0;
enum class MouseState
{
    None,
    Clicked,
};
static MouseState g_MouseState = MouseState::None;
bool OnLeftButtonClick(const MouseEvent& event, void* userData)
{
    g_MouseState = MouseState::Clicked;

    return true;
}
bool OnMouseMove(const MouseEvent& event, void* userData) 
{
    if (g_MouseState != MouseState::Clicked)
        return true;

    LiquidGlass* pApp = (LiquidGlass*)userData;
    LiquidGlassComponentPtr pGlass = pApp->GetLiquidGlassComponent();
    return true;
}
bool OnLeftButtonUp(const MouseEvent& event, void* userData) 
{
    g_MouseState = MouseState::None;

    return true;
}



LiquidGlass::LiquidGlass() 
    :AppFramework("LiquidGlass") 
{
    MouseHookManager& mouseManager = MouseHookManager::GetInstance();

    // 注册事件回调
    g_iClickDownId = mouseManager.RegisterCallback(
        MouseEventType::LeftButtonDown, OnLeftButtonClick, this);

    g_iMoveId = mouseManager.RegisterCallback(
        MouseEventType::MouseMove, OnMouseMove, this);

    g_iClickUpId = mouseManager.RegisterCallback(
        MouseEventType::LeftButtonUp, OnLeftButtonUp, this);
}
LiquidGlass::~LiquidGlass()
{
    MouseHookManager& mouseManager = MouseHookManager::GetInstance();
    // 卸载钩子
    mouseManager.UninstallHook();

    // 注销回调
    mouseManager.UnregisterCallback(g_iClickDownId);
    mouseManager.UnregisterCallback(g_iMoveId);
    mouseManager.UnregisterCallback(g_iClickUpId);
}
SResult LiquidGlass::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    m_pGlass = MakeSharedPtr<LiquidGlassComponent>(m_pContext.get(), (uint32_t)w, (uint32_t)h);
    m_pContext->SceneManagerInstance().AddSprite2DComponent(m_pGlass);

    std::string bg_files = FullPath("asset/textures/the_one.jpg");
    BitmapBufferPtr bit = ImageDecodeFromFile(bg_files, ImageType::JPEG);
    RHITexturePtr bg_tex = m_pContext->RHIContextInstance().CreateTexture2D(bit);
    m_pGlass->SetImage(bg_tex);

    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get(), "CameraEntity");
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 0, -5), float3(0, 0, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    return S_Success;
}

SResult LiquidGlass::OnUpdate()
{
    SEEK_RETIF_FAIL(m_pContext->Tick());
    SEEK_RETIF_FAIL(m_pContext->BeginRender());
    SEEK_RETIF_FAIL(m_pContext->RenderFrame());
    IMGUI_Begin();
    IMGUI_Rendering();
    SEEK_RETIF_FAIL(m_pContext->EndRender());
    return S_Success;
}
SResult LiquidGlass::InitContext(void* device, void* native_wnd)
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
    LiquidGlass theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
