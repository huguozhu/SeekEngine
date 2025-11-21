
#include "07.LiquidGlass.h"

#define SEEK_MACRO_FILE_UID 91     // this code is auto generated, don't touch it!!!


static int g_iChosenShapeIndex = -1;
enum class ChoseState
{
    None,
    Chosen,
};
static ChoseState g_ChoseState = ChoseState::None;

void LiquidGlass::HandleMouseEvent()
{
    if (ImGui::IsMouseClicked(ImGuiPopupFlags_MouseButtonLeft))
    {
        ImVec2 pos = ImGui::GetMousePos();
        int shape_index = -1;
        bool hit = m_pGlass->HitShape(float2(pos.x, pos.y), shape_index);
        if (hit)
        {
            g_ChoseState = ChoseState::Chosen;
            g_iChosenShapeIndex = shape_index;
            m_pGlass->SetSpringState(shape_index, SpringMassDamperState::Stopped);
        }
        else
            g_ChoseState = ChoseState::None;

    }
    if (g_ChoseState == ChoseState::Chosen && g_iChosenShapeIndex >= 0)
    {
        if (ImGui::IsMouseDragging(ImGuiPopupFlags_MouseButtonLeft))
        {
            ImVec2 pos = ImGui::GetMousePos();
            m_pGlass->SetCurShapeCenter(g_iChosenShapeIndex, float2(pos.x, pos.y));
        }
    }
    if (ImGui::IsMouseReleased(ImGuiPopupFlags_MouseButtonLeft))
    {
        if (g_iChosenShapeIndex >= 0)
        {
            ImVec2 pos = ImGui::GetMousePos();
            m_pGlass->SetInitShapeCenter(g_iChosenShapeIndex, float2(pos.x, pos.y));
            m_pGlass->SetSpringState(g_iChosenShapeIndex, SpringMassDamperState::Playing);
            m_pGlass->Reset(g_iChosenShapeIndex);
        }
        g_iChosenShapeIndex = -1;;
        g_ChoseState = ChoseState::None;
    }
}

LiquidGlass::LiquidGlass() 
    :AppFramework("LiquidGlass") 
{
}
LiquidGlass::~LiquidGlass()
{

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
    this->HandleMouseEvent();

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
