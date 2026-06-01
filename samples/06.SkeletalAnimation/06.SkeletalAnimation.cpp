#include "app_framework.h"
#include "seek_engine.h"
#include "common/first_person_camera_controller.h"
#include "components/animation_component.h"
#include "components/skeletal_mesh_component.h"

USING_NAMESPACE_SEEK

class SkeletalAnimation : public AppFramework
{
public:
    SkeletalAnimation() : AppFramework("SkeletalAnimation") {}

    virtual SResult OnCreate() override;
    virtual SResult OnUpdate() override;
    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr);

private:
    EntityPtr m_pMeshEntity = nullptr;
    EntityPtr m_pBrainStemEntity = nullptr;
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[4] = { nullptr };

    FirstPersonCameraController m_CameraController;
};

SResult SkeletalAnimation::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    // Step1: Load Sponza (known to work) as scene ground
    std::string sponzaPath = FullPath("asset/gltf/Sponza/Sponza.gltf");
    m_pMeshEntity = this->CreateEntityFromFile(sponzaPath);
    if (m_pMeshEntity)
    {
        m_pMeshEntity->AddToTopScene();
        LOG_INFO("Sponza loaded");
    }

    // Step2: Load BrainStem with skeletal animation
    std::string bsPath = FullPath("asset/gltf/BrainStem/BrainStem.glb");
    m_pBrainStemEntity = this->CreateEntityFromFile(bsPath);
    if (!m_pBrainStemEntity)
    {
        LOG_ERROR("Failed to load BrainStem!");
        return -1;
    }
    m_pBrainStemEntity->AddToTopScene();
    // Position BrainStem in the center, slightly above ground
    m_pBrainStemEntity->SetWorldTranslation(float3(0, 0.5, 0));

    // Log animation info
    Component* comp = m_pBrainStemEntity->GetComponent(ComponentType::Animation);
    if (comp)
    {
        auto* anim = static_cast<AnimationComponent*>(comp);
        auto& sec = anim->GetAnimSectionInfo();
        LOG_INFO("BrainStem animation: %.1fs, %d tracks",
                 sec.empty() ? 0.0f : sec[0].endTime,
                 (int)anim->GetTransformAnimationTracks().size());
    }

    // Step3: Camera (same as Sample5)
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(45.0 * Math::DEG2RAD, w / h, 0.01f, 200.0f);
    pCam->SetLookAt(float3(0, 2, -15), float3(0, 0, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());
    m_CameraController.SetMoveSpeed(0.5);

    // Step4: Lighting (same as Sample5)
    Color c = Color::DefaultAmbientColor;
    LightComponentPtr pLight = MakeSharedPtr<AmbientLightComponent>(m_pContext.get());
    pLight->SetColor(c);
    m_pLightEntity[0] = MakeSharedPtr<Entity>(m_pContext.get(), "Ambient Light");
    m_pLightEntity[0]->AddSceneComponent(pLight);

    float p = 1.0;
    pLight = MakeSharedPtr<DirectionalLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(float3(p, -p, p));
    pLight->SetIntensity(1.5);
    pLight->SetWorldTranslation(float3(-p, p, -p));
    m_pLightEntity[1] = MakeSharedPtr<Entity>(m_pContext.get(), "Directional Light");
    m_pLightEntity[1]->AddSceneComponent(pLight);
    m_pLightEntity[1]->AddToTopScene();

    m_pContext->SceneManagerInstance().PrintTree();
    return S_Success;
}

SResult SkeletalAnimation::OnUpdate()
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

SResult SkeletalAnimation::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.enable_debug = true;
    info.renderer_type = RendererType::Forward;
    info.anti_aliasing_mode = AntiAliasingMode::None;
    info.preferred_adapter = 0;

    m_pContext = MakeSharedPtr<Context>(info);
    SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));

    return S_Success;
}

int main()
{
    SkeletalAnimation theApp;
    return APP_RUN(&theApp);
}

#undef SEEK_MACRO_FILE_UID
