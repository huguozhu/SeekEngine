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
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[3] = { nullptr };

    FirstPersonCameraController m_CameraController;

    AnimationComponent* m_pAnimComponent = nullptr;
    float m_AnimDuration = 0.0f;
};

SResult SkeletalAnimation::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    // Step1: Load BrainStem with skeletal animation (animation auto-plays)
    std::string modelPath = FullPath("asset/gltf/BrainStem/BrainStem.glb");
    m_pMeshEntity = this->CreateEntityFromFile(modelPath);
    if (!m_pMeshEntity)
    {
        LOG_ERROR("Failed to load BrainStem model!");
        return -1;
    }
    m_pMeshEntity->AddToTopScene();
    m_pMeshEntity->SetLocalScale(0.5);

    // Get animation component (already auto-playing from CreateEntityFromFile)
    Component* comp = m_pMeshEntity->GetComponent(ComponentType::Animation);
    if (comp)
    {
        m_pAnimComponent = static_cast<AnimationComponent*>(comp);
        auto& sections = m_pAnimComponent->GetAnimSectionInfo();
        if (!sections.empty())
            m_AnimDuration = sections[0].endTime;
        LOG_INFO("Animation loaded: %.2fs, joints: %d",
                 m_AnimDuration,
                 m_pAnimComponent->GetTransformAnimationTracks().size());
    }

    // Step2: Camera
    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get());
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(45.0 * Math::DEG2RAD, w / h, 0.01f, 200.0f);
    pCam->SetLookAt(float3(0, 0.5, -1.5), float3(0, 0.3, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();
    m_CameraController.SetCamera(pCam.get());
    m_CameraController.SetMoveSpeed(0.05);

    // Step3: Ambient Light
    LightComponentPtr pLight = MakeSharedPtr<AmbientLightComponent>(m_pContext.get());
    pLight->SetColor(Color(0.15f, 0.15f, 0.15f));
    m_pLightEntity[0] = MakeSharedPtr<Entity>(m_pContext.get(), "Ambient Light");
    m_pLightEntity[0]->AddSceneComponent(pLight);

    // Step4: Directional Light
    pLight = MakeSharedPtr<DirectionalLightComponent>(m_pContext.get());
    pLight->SetColor(Color::White);
    pLight->SetDirection(float3(0.5, -1, -0.5));
    pLight->SetIntensity(2.0);
    m_pLightEntity[1] = MakeSharedPtr<Entity>(m_pContext.get(), "Directional Light");
    m_pLightEntity[1]->AddSceneComponent(pLight);
    m_pLightEntity[1]->AddToTopScene();

    // Step5: Point Light (warm fill)
    pLight = MakeSharedPtr<PointLightComponent>(m_pContext.get());
    pLight->SetColor(Color(0.8f, 0.6f, 0.3f));
    pLight->SetIntensity(15);
    pLight->SetFalloffRadius(10);
    pLight->SetWorldTranslation(float3(2, 1, 2));
    m_pLightEntity[2] = MakeSharedPtr<Entity>(m_pContext.get(), "Point Light");
    m_pLightEntity[2]->AddSceneComponent(pLight);
    m_pLightEntity[2]->AddToTopScene();

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
