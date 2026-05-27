#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"

USING_NAMESPACE_SEEK

enum class SDFSceneType : int
{
    LiquidGlass = 0,
    MetaballWater = 1,
};

static const char* g_szSceneNames[] = { "LiquidGlass", "MetaballWater" };

class SDF_Metaball : public AppFramework
{
public:
    SDF_Metaball();
    ~SDF_Metaball();
    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

    void HandleMouseEvent();
    void SwitchScene(SDFSceneType scene);

private:
    EntityPtr               m_pCameraEntity = nullptr;
    LiquidGlassComponentPtr m_pGlass = nullptr;
    Metaball2DComponentPtr  m_pMetaball = nullptr;
    RHITexturePtr           m_pBgTexture;
    SDFSceneType            m_eActiveScene = SDFSceneType::LiquidGlass;

};

static int g_iChosenShapeIndex = -1;
enum class ChoseState
{
    None,
    Chosen,
};
static ChoseState g_ChoseState = ChoseState::None;

void SDF_Metaball::HandleMouseEvent()
{
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImVec2 pos = ImGui::GetMousePos();
            m_pGlass->SetCurShapeCenter(g_iChosenShapeIndex, float2(pos.x, pos.y));
        }
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
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

void SDF_Metaball::SwitchScene(SDFSceneType scene)
{
    if (m_eActiveScene == scene)
        return;

    m_eActiveScene = scene;

    if (m_pGlass)
        m_pGlass->SetEnabled(scene == SDFSceneType::LiquidGlass);
    if (m_pMetaball)
        m_pMetaball->SetEnabled(scene == SDFSceneType::MetaballWater);
}

SDF_Metaball::SDF_Metaball()
    :AppFramework("SDF_Metaball")
{
}
SDF_Metaball::~SDF_Metaball()
{

}
SResult SDF_Metaball::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    std::string bg_files = FullPath("asset/textures/the_one.jpg");
    BitmapBufferPtr bit = ImageDecodeFromFile(bg_files, ImageType::JPEG);
    m_pBgTexture = m_pContext->RHIContextInstance().CreateTexture2D(bit);

    m_pGlass = MakeSharedPtr<LiquidGlassComponent>(m_pContext.get(), (uint32_t)w, (uint32_t)h);
    m_pGlass->SetImage(m_pBgTexture);
    m_pContext->SceneManagerInstance().AddSprite2DComponent(m_pGlass);

    m_pMetaball = MakeSharedPtr<Metaball2DComponent>(m_pContext.get(), (uint32_t)w, (uint32_t)h);
    m_pMetaball->SetImage(m_pBgTexture);
    m_pMetaball->SetEnabled(false);
    m_pContext->SceneManagerInstance().AddSprite2DComponent(m_pMetaball);

    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get(), "CameraEntity");
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 0, -5), float3(0, 0, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    return S_Success;
}

SResult SDF_Metaball::OnUpdate()
{
    if (m_eActiveScene == SDFSceneType::LiquidGlass)
        this->HandleMouseEvent();

    SEEK_RETIF_FAIL(m_pContext->Tick());
    SEEK_RETIF_FAIL(m_pContext->BeginRender());
    SEEK_RETIF_FAIL(m_pContext->RenderFrame());

    IMGUI_Begin();
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(260, 80), ImGuiCond_FirstUseEver);
        ImGui::Begin("SDF Scene Selector");
        {
            int current_scene = (int)m_eActiveScene;
            if (ImGui::Combo("Scene", &current_scene, g_szSceneNames, IM_ARRAYSIZE(g_szSceneNames)))
            {
                SwitchScene((SDFSceneType)current_scene);
            }
            ImGui::Text("Current: %s", g_szSceneNames[(int)m_eActiveScene]);
        }
        ImGui::End();
    }
    IMGUI_Rendering();

    SEEK_RETIF_FAIL(m_pContext->EndRender());
    return S_Success;
}
SResult SDF_Metaball::InitContext(void* device, void* native_wnd)
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
    SDF_Metaball theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
