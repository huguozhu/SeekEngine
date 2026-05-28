#include "app_framework.h"
#include "seek_engine.h"
#include "components/particle_component.h"
#include "components/liquid_glass_component.h"
#include "components/metaball_component.h"
#include "math/color.h"
#include "common/first_person_camera_controller.h"
#include <cmath>
#include <fstream>
#include <sstream>

USING_NAMESPACE_SEEK

// ============================================================================
// IScene — 场景接口
// ============================================================================
class IScene
{
public:
    IScene() = default;
    virtual ~IScene() = default;
    virtual const char* GetName() const = 0;
    virtual SResult OnCreate(Context* ctx) = 0;
    virtual SResult OnUpdate(Context* ctx, float deltaTime) { return S_Success; }
    virtual void OnDestroy(Context* ctx) {}
};

// ============================================================================
// SceneBasic — 基础场景（原 01.Tutorial：圆锥体 + 相机）
// ============================================================================
class SceneBasic : public IScene
{
public:
    SceneBasic() = default;
    const char* GetName() const override { return "Basic (Cone)"; }

    SResult OnCreate(Context* ctx) override
    {
        RHIContext& rc = ctx->RHIContextInstance();
        Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
        float w = vp.width;
        float h = vp.height;

        m_pEntity = MakeSharedPtr<Entity>(ctx);
        m_pEntity->AddToTopScene();

        CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(ctx);
        pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
        pCam->SetLookAt(float3(0, 2, 3), float3(0, 0, 0), float3(0, 1, 0));
        m_pEntity->AddSceneComponent(pCam);

        ConeMeshComponentPtr pMesh = MakeSharedPtr<ConeMeshComponent>(ctx);
        m_pEntity->AddSceneComponent(pMesh);

        return S_Success;
    }

    void OnDestroy(Context* ctx) override
    {
        if (m_pEntity)
            m_pEntity->DeleteFromTopScene();
        m_pEntity = nullptr;
    }

private:
    EntityPtr m_pEntity = nullptr;
};

// ============================================================================
// SceneParticles — 粒子系统场景（原 02.Particles）
// ============================================================================
class SceneParticles : public IScene
{
public:
    SceneParticles() = default;
    const char* GetName() const override { return "Particles"; }

    SResult OnCreate(Context* ctx) override
    {
        m_pContext = ctx;

        RHIContext& rc = ctx->RHIContextInstance();
        Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
        float w = vp.width;
        float h = vp.height;

        m_pCameraEntity = MakeSharedPtr<Entity>(ctx, "CameraEntity");
        CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(ctx);
        pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
        pCam->SetLookAt(float3(0, 0, -5), float3(0, 0, 0), float3(0, 1, 0));
        m_pCameraEntity->AddSceneComponent(pCam);
        m_pCameraEntity->AddToTopScene();
        m_CameraController.SetCamera(pCam.get());

        CreateParticleEntities();
        return S_Success;
    }

    SResult OnUpdate(Context* ctx, float deltaTime) override
    {
        m_CameraController.Update(deltaTime);
        return S_Success;
    }

    void OnDestroy(Context* ctx) override
    {
        for (auto& e : m_ParticleList)
        {
            if (e) e->DeleteFromTopScene();
        }
        m_ParticleList.clear();
        m_pParticleComponentList.clear();
        m_vBitmaps.clear();

        if (m_pCameraEntity)
            m_pCameraEntity->DeleteFromTopScene();
        m_pCameraEntity = nullptr;
    }

private:
    void CreateParticleEntities()
    {
        const uint32_t file_index = 3;
        std::string filepaths[] = {
            AppFramework::FullPath("02.Particles/fire_3x3.json"),
            AppFramework::FullPath("02.Particles/fire_4x4.json"),
            AppFramework::FullPath("02.Particles/wave_5x4.json"),
            AppFramework::FullPath("02.Particles/nine_birds_3x3.json"),
            AppFramework::FullPath("02.Particles/particle_3x3_reflect.json"),
            AppFramework::FullPath("02.Particles/particle_no_tex.json"),
            AppFramework::FullPath("02.Particles/particle_pos_over_time.json"),
            AppFramework::FullPath("02.Particles/particle_pos_over_time_tex.json"),
            AppFramework::FullPath("02.Particles/particle_pos_over_time_plus_size_over_life.json"),
        };
        std::ifstream inp(filepaths[file_index]);
        std::stringstream buffer;
        buffer << inp.rdbuf();

        rapidjson::Document doc;
        doc.Parse(buffer.str().c_str());
        uint32_t size = doc.Size();
        std::vector<ParticleSystemParam> params(size);
        for (uint32_t i = 0; i < size; i++)
        {
            auto& node = doc[i];

            if (node.HasMember("duration"))
                params[i].duration = node["duration"].GetFloat();

            if (node.HasMember("position"))
            {
                float3 pos;
                pos[0] = node["position"][0].GetFloat();
                pos[1] = node["position"][1].GetFloat();
                pos[2] = node["position"][2].GetFloat();
                params[i].positions_over_time.push_back(std::make_pair(0.0f, pos));
            }
            else if (node.HasMember("positions_over_time"))
            {
                auto& pot = node["positions_over_time"];
                uint32_t c = pot.MemberCapacity();
                params[i].positions_over_time.resize(c);
                uint32_t j = 0;
                for (auto it = pot.MemberBegin(); it < pot.MemberEnd(); ++it)
                {
                    params[i].positions_over_time[j].first = (float)atof(it->name.GetString());
                    params[i].positions_over_time[j].second = float3(
                        it->value[0].GetFloat(), it->value[1].GetFloat(), it->value[2].GetFloat());
                    ++j;
                }
            }
            else
                params[i].positions_over_time.push_back(std::make_pair(0.0f, float3()));

            if (node.HasMember("emit_direction_type"))
                params[i].emit_direction_type = (EmitDirectionType)node["emit_direction_type"].GetUint();

            params[i].direction = float3(node["direction"][0].GetFloat(), node["direction"][1].GetFloat(), node["direction"][2].GetFloat());
            params[i].direction_spread_percent = node["direction_spread_percent"].GetFloat();
            params[i].emit_shape_type = (EmitShapeType)node["emit_shape_type"].GetUint();
            params[i].sphere_radius = node["sphere_radius"].GetFloat();
            params[i].box_size = float3(node["box_size"][0].GetFloat(), node["box_size"][1].GetFloat(), node["box_size"][2].GetFloat());

            params[i].particles_per_sec = node["particles_per_sec"].GetFloat();
            params[i].min_init_speed = node["min_init_speed"].GetFloat();
            params[i].max_init_speed = node["max_init_speed"].GetFloat();
            params[i].min_life_time = node["min_life_time"].GetFloat();
            params[i].max_life_time = node["max_life_time"].GetFloat();

            params[i].gravity = float3(node["gravity"][0].GetFloat(), node["gravity"][1].GetFloat(), node["gravity"][2].GetFloat());
            params[i].wind = float3(node["wind"][0].GetFloat(), node["wind"][1].GetFloat(), node["wind"][2].GetFloat());

            if (node.HasMember("particle_size"))
            {
                params[i].particle_size_over_life.push_back(std::make_pair(0.0f,
                    float2(node["particle_size"][0].GetFloat(), node["particle_size"][1].GetFloat())));
            }
            else if (node.HasMember("particle_size_over_life"))
            {
                auto& sol = node["particle_size_over_life"];
                uint32_t c = sol.MemberCapacity();
                params[i].particle_size_over_life.resize(c);
                uint32_t j = 0;
                for (auto it = sol.MemberBegin(); it < sol.MemberEnd(); ++it)
                {
                    params[i].particle_size_over_life[j].first = (float)atof(it->name.GetString());
                    params[i].particle_size_over_life[j].second = float2(it->value[0].GetFloat(), it->value[1].GetFloat());
                    ++j;
                }
            }
            else
                params[i].particle_size_over_life.push_back(std::make_pair(0.0f, float2(1, 1)));

            if (node.HasMember("particle_color"))
            {
                params[i].particle_color_over_life.push_back(std::make_pair(0.0f,
                    float4(node["particle_color"][0].GetFloat(), node["particle_color"][1].GetFloat(),
                           node["particle_color"][2].GetFloat(), node["particle_color"][3].GetFloat())));
            }
            else if (node.HasMember("particle_color_over_life"))
            {
                auto& col = node["particle_color_over_life"];
                uint32_t c = col.MemberCapacity();
                params[i].particle_color_over_life.resize(c);
                uint32_t j = 0;
                for (auto it = col.MemberBegin(); it < col.MemberEnd(); ++it)
                {
                    params[i].particle_color_over_life[j].first = (float)atof(it->name.GetString());
                    params[i].particle_color_over_life[j].second = float4(
                        it->value[0].GetFloat(), it->value[1].GetFloat(),
                        it->value[2].GetFloat(), it->value[3].GetFloat());
                    ++j;
                }
            }
            else
                params[i].particle_color_over_life.push_back(std::make_pair(0.0f, float4(1.0, 1.0, 1.0, 1.0)));

            params[i].tex_rows_cols = uint2(node["tex_rows"].GetUint(), node["tex_cols"].GetUint());
            params[i].tex_frames_per_sec = node["tex_frames_per_sec"].GetFloat();
            params[i].tex_time_sampling_type = (TexTimeSamplingType)node["tex_time_sampling_type"].GetUint();

            std::string path = AppFramework::FullPath(std::string(node["tex_path"].GetString()));
            BitmapBufferPtr tex_bitmap = ImageDecodeFromFile(path);
            m_vBitmaps.push_back(tex_bitmap);
            if (tex_bitmap)
            {
                RHITexture::Desc desc;
                desc.width = tex_bitmap->Width();
                desc.height = tex_bitmap->Height();
                desc.type = TextureType::Tex2D;
                desc.format = tex_bitmap->Format();
                desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_CPU_WRITE;
                params[i].particle_tex = m_pContext->RHIContextInstance().CreateTexture2D(desc, tex_bitmap);
            }

            EntityPtr pParticleEntity = MakeSharedPtr<Entity>(m_pContext, "ParticleEntity");
            ParticleComponentPtr pParticle = MakeSharedPtr<ParticleComponent>(m_pContext, params[i]);
            pParticle->Play();
            m_pParticleComponentList.push_back(pParticle);
            pParticleEntity->AddSceneComponent(pParticle);
            pParticleEntity->AddToTopScene();
            m_ParticleList.push_back(pParticleEntity);
        }
    }

    Context* m_pContext = nullptr;
    EntityPtr m_pCameraEntity = nullptr;
    std::vector<EntityPtr> m_ParticleList;
    std::vector<ParticleComponentPtr> m_pParticleComponentList;
    std::vector<BitmapBufferPtr> m_vBitmaps;
    FirstPersonCameraController m_CameraController;
};

// ============================================================================
// SceneSDF — SDF 场景（原 03.SDF_Metaball：LiquidGlass / MetaballWater）
// ============================================================================
static const char* g_szSDFSceneNames[] = { "LiquidGlass", "MetaballWater" };

class SceneSDF : public IScene
{
public:
    SceneSDF() = default;
    const char* GetName() const override { return "SDF (LiquidGlass / Metaball)"; }

    SResult OnCreate(Context* ctx) override
    {
        m_pContext = ctx;

        RHIContext& rc = ctx->RHIContextInstance();
        Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
        float w = vp.width;
        float h = vp.height;

        if (!m_pGlass)
        {
            std::string bg_file = AppFramework::FullPath("asset/textures/the_one.jpg");
            BitmapBufferPtr bit = ImageDecodeFromFile(bg_file, ImageType::JPEG);
            if (bit)
            {
                m_pBgTexture = ctx->RHIContextInstance().CreateTexture2D(bit);
            }

            m_pGlass = MakeSharedPtr<LiquidGlassComponent>(ctx, (uint32_t)w, (uint32_t)h);
            if (m_pBgTexture)
            {
                m_pGlass->SetImage(m_pBgTexture);
            }
            ctx->SceneManagerInstance().AddSprite2DComponent(m_pGlass);

            m_pMetaball = MakeSharedPtr<Metaball2DComponent>(ctx, (uint32_t)w, (uint32_t)h);
            if (m_pBgTexture)
            {
                m_pMetaball->SetImage(m_pBgTexture);
            }
            m_pMetaball->SetEnabled(false);
            ctx->SceneManagerInstance().AddSprite2DComponent(m_pMetaball);
        }

        // 重新激活时恢复启用状态
        if (m_pGlass)
        {
            m_pGlass->SetEnabled(m_eActiveSubScene == SDFSubScene::LiquidGlass);
            m_pMetaball->SetEnabled(m_eActiveSubScene == SDFSubScene::MetaballWater);
        }

        if (!m_pCameraEntity)
        {
            m_pCameraEntity = MakeSharedPtr<Entity>(ctx, "CameraEntity");
            CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(ctx);
            pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
            pCam->SetLookAt(float3(0, 0, -5), float3(0, 0, 0), float3(0, 1, 0));
            m_pCameraEntity->AddSceneComponent(pCam);
        }
        m_pCameraEntity->AddToTopScene();

        return S_Success;
    }

    SResult OnUpdate(Context* ctx, float deltaTime) override
    {
        return S_Success;
    }

    void OnDestroy(Context* ctx) override
    {
        // 禁用而非销毁 Sprite2D 组件（SceneManager 持有引用，无法通过置空释放）
        if (m_pGlass)
            m_pGlass->SetEnabled(false);
        if (m_pMetaball)
            m_pMetaball->SetEnabled(false);

        if (m_pCameraEntity)
            m_pCameraEntity->DeleteFromTopScene();
        // 保留 m_pCameraEntity，下次切换回来时复用
    }

    void ShowUI()
    {
        // 鼠标交互需要在 ImGui 帧内进行（已在 OnUpdate 外部由 IMGUI_Begin/End 包裹）
        if (m_eActiveSubScene == SDFSubScene::LiquidGlass && m_pGlass)
            HandleMouseEvent();

        ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(260, 80), ImGuiCond_FirstUseEver);
        ImGui::Begin("SDF Sub-Scene");
        {
            int current = (int)m_eActiveSubScene;
            if (ImGui::Combo("Mode", &current, g_szSDFSceneNames, IM_ARRAYSIZE(g_szSDFSceneNames)))
            {
                SwitchSubScene((SDFSubScene)current);
            }
            ImGui::Text("Current: %s", g_szSDFSceneNames[(int)m_eActiveSubScene]);
        }
        ImGui::End();
    }

private:
    enum class SDFSubScene : int
    {
        LiquidGlass = 0,
        MetaballWater = 1,
    };

    void SwitchSubScene(SDFSubScene scene)
    {
        if (m_eActiveSubScene == scene) return;
        m_eActiveSubScene = scene;
        if (m_pGlass)
            m_pGlass->SetEnabled(scene == SDFSubScene::LiquidGlass);
        if (m_pMetaball)
            m_pMetaball->SetEnabled(scene == SDFSubScene::MetaballWater);
    }

    void HandleMouseEvent()
    {
        if (!m_pGlass) return;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImVec2 pos = ImGui::GetMousePos();
            int shape_index = -1;
            bool hit = m_pGlass->HitShape(float2(pos.x, pos.y), shape_index);
            if (hit)
            {
                m_eChoseState = ChoseState::Chosen;
                m_iChosenShapeIndex = shape_index;
                m_pGlass->SetSpringState(shape_index, SpringMassDamperState::Stopped);
            }
            else
                m_eChoseState = ChoseState::None;
        }
        if (m_eChoseState == ChoseState::Chosen && m_iChosenShapeIndex >= 0)
        {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                ImVec2 pos = ImGui::GetMousePos();
                m_pGlass->SetCurShapeCenter(m_iChosenShapeIndex, float2(pos.x, pos.y));
            }
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (m_iChosenShapeIndex >= 0)
            {
                ImVec2 pos = ImGui::GetMousePos();
                m_pGlass->SetInitShapeCenter(m_iChosenShapeIndex, float2(pos.x, pos.y));
                m_pGlass->SetSpringState(m_iChosenShapeIndex, SpringMassDamperState::Playing);
                m_pGlass->Reset(m_iChosenShapeIndex);
            }
            m_iChosenShapeIndex = -1;
            m_eChoseState = ChoseState::None;
        }
    }

    Context* m_pContext = nullptr;
    EntityPtr m_pCameraEntity = nullptr;
    LiquidGlassComponentPtr m_pGlass = nullptr;
    Metaball2DComponentPtr m_pMetaball = nullptr;
    RHITexturePtr m_pBgTexture = nullptr;
    SDFSubScene m_eActiveSubScene = SDFSubScene::LiquidGlass;

    enum class ChoseState { None, Chosen };
    ChoseState m_eChoseState = ChoseState::None;
    int m_iChosenShapeIndex = -1;
};

// ============================================================================
// SceneLighting — 光照场景（原 04.Lighting）
// ============================================================================
class SceneLighting : public IScene
{
public:
    SceneLighting() = default;
    const char* GetName() const override { return "Lighting"; }

    SResult OnCreate(Context* ctx) override
    {
        m_pContext = ctx;

        RHIContext& rc = ctx->RHIContextInstance();
        Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
        float w = vp.width;
        float h = vp.height;

        m_pCameraEntity = MakeSharedPtr<Entity>(ctx);
        CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(ctx);
        pCam->ProjPerspectiveParams(60 * Math::DEG2RAD, w / h, 0.1f, 200.0f);
        pCam->SetLookAt(float3(0, 4, -4), float3(0, 0, 0), float3(0, 1, 0));
        m_pCameraEntity->AddSceneComponent(pCam);
        m_pCameraEntity->AddToTopScene();
        m_CameraController.SetCamera(pCam.get());
        m_CameraController.SetMoveSpeed(0.1f);

        CreateLights();
        CreateGeometry();
        CreateSkybox();

        return S_Success;
    }

    SResult OnUpdate(Context* ctx, float deltaTime) override
    {
        m_CameraController.Update(deltaTime);
        return S_Success;
    }

    void OnDestroy(Context* ctx) override
    {
        for (auto& e : m_pLightEntity)
        {
            if (e) e->DeleteFromTopScene();
            e = nullptr;
        }
        if (m_pPlaneEntity)  { m_pPlaneEntity->DeleteFromTopScene();  m_pPlaneEntity = nullptr; }
        if (m_pSphereEntity) { m_pSphereEntity->DeleteFromTopScene(); m_pSphereEntity = nullptr; }
        if (m_pConeEntity)   { m_pConeEntity->DeleteFromTopScene();   m_pConeEntity = nullptr; }
        if (m_pSkyBoxEntity) { m_pSkyBoxEntity->DeleteFromTopScene(); m_pSkyBoxEntity = nullptr; }
        if (m_pCameraEntity) { m_pCameraEntity->DeleteFromTopScene(); m_pCameraEntity = nullptr; }
    }

private:
    void CreateLights()
    {
        Color c = Color::DefaultAmbientColor;
        LightComponentPtr pLight = MakeSharedPtr<AmbientLightComponent>(m_pContext);
        pLight->SetColor(c);
        m_pLightEntity[0] = MakeSharedPtr<Entity>(m_pContext, "Ambient Light");
        m_pLightEntity[0]->AddSceneComponent(pLight);
        m_pLightEntity[0]->AddToTopScene();

        float p = 3.0f;
        float3 directional_pos = float3(-p, p, p);
        pLight = MakeSharedPtr<DirectionalLightComponent>(m_pContext);
        pLight->SetColor(Color::White);
        pLight->SetDirection(float3(p, -p, -p));
        pLight->SetIntensity(0.5f);
        pLight->CastShadow(1);
        pLight->SoftShadow(1);
        pLight->SetWorldTranslation(directional_pos);
        m_pLightEntity[1] = MakeSharedPtr<Entity>(m_pContext, "Directional Light");
        m_pLightEntity[1]->AddSceneComponent(pLight);
        m_pLightEntity[1]->AddToTopScene();

        float3 spot_pos = float3(-p, p, 0);
        float3 look_at = float3(0, 0, 0);
        float cutoff_in = Math::DEG2RAD * 25;
        float cutoff_out = Math::DEG2RAD * 45;
        pLight = MakeSharedPtr<SpotLightComponent>(m_pContext);
        pLight->SetColor(Color::Red);
        pLight->SetDirection(look_at - spot_pos);
        pLight->SetWorldTranslation(spot_pos);
        pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
        pLight->SetIntensity(50.24f);
        pLight->CastShadow(1);
        pLight->SoftShadow(1);
        m_pLightEntity[2] = MakeSharedPtr<Entity>(m_pContext, "Spot Light");
        m_pLightEntity[2]->AddSceneComponent(pLight);
        m_pLightEntity[2]->AddToTopScene();

        spot_pos = float3(p, p, -p);
        pLight = MakeSharedPtr<SpotLightComponent>(m_pContext);
        pLight->SetColor(Color::White);
        pLight->SetDirection(look_at - spot_pos);
        pLight->SetInOutCutoff(float2(cutoff_in, cutoff_out));
        pLight->SetIntensity(5.24f);
        pLight->SetWorldTranslation(spot_pos);
        m_pLightEntity[3] = MakeSharedPtr<Entity>(m_pContext, "Spot Light");
        m_pLightEntity[3]->AddSceneComponent(pLight);
        m_pLightEntity[3]->AddToTopScene();

        float3 point_pos = float3(0, 3.0f, 0);
        pLight = MakeSharedPtr<PointLightComponent>(m_pContext);
        pLight->SetColor(Color::Green);
        pLight->SetIntensity(45);
        pLight->CastShadow(1);
        pLight->SoftShadow(1);
        pLight->SetFalloffRadius(15.4f);
        pLight->SetWorldTranslation(point_pos);
        m_pLightEntity[4] = MakeSharedPtr<Entity>(m_pContext, "Point Light");
        m_pLightEntity[4]->AddSceneComponent(pLight);
        m_pLightEntity[4]->AddToTopScene();
    }

    void CreateGeometry()
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
        BitmapBufferPtr bit = ImageDecodeFromFile(AppFramework::FullPath("asset/textures/boy.jpg"), ImageType::JPEG);
        RHITexturePtr tex_0 = rc.CreateTexture2D(bit);

        PlaneMeshComponentPtr pPlane = MakeSharedPtr<PlaneMeshComponent>(m_pContext, 7.6f, 7.6f);
        MaterialPtr material = pPlane->GetMeshByIndex(0)->GetMaterial();
        material->albedo_tex = tex_0;
        m_pPlaneEntity = MakeSharedPtr<Entity>(m_pContext);
        m_pPlaneEntity->SetWorldTranslation(float3(0, -0.5f, 0));
        m_pPlaneEntity->AddSceneComponent(pPlane);
        m_pPlaneEntity->AddToTopScene();

        SphereMeshComponentPtr pSphere = MakeSharedPtr<SphereMeshComponent>(m_pContext);
        pSphere->SetWorldTranslation(float3(-2, 0.5f, 0));
        pSphere->SetWorldScale(float3(1.0f));
        material = pSphere->GetMeshByIndex(0)->GetMaterial();
        material->albedo_tex = tex_0;
        m_pSphereEntity = MakeSharedPtr<Entity>(m_pContext);
        m_pSphereEntity->AddSceneComponent(pSphere);
        m_pSphereEntity->AddToTopScene();

        ConeMeshComponentPtr pCone = MakeSharedPtr<ConeMeshComponent>(m_pContext);
        pCone->SetWorldTranslation(float3(2, 1.0f, 0));
        material = pCone->GetMeshByIndex(0)->GetMaterial();
        material->albedo_tex = tex_0;
        m_pConeEntity = MakeSharedPtr<Entity>(m_pContext);
        m_pConeEntity->AddSceneComponent(pCone);
        m_pConeEntity->AddToTopScene();
    }

    void CreateSkybox()
    {
        RHIContext& rc = m_pContext->RHIContextInstance();
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
            AppFramework::FullPath("asset/textures/skybox/daylight0.png"),
            AppFramework::FullPath("asset/textures/skybox/daylight1.png"),
            AppFramework::FullPath("asset/textures/skybox/daylight2.png"),
            AppFramework::FullPath("asset/textures/skybox/daylight3.png"),
            AppFramework::FullPath("asset/textures/skybox/daylight4.png"),
            AppFramework::FullPath("asset/textures/skybox/daylight5.png"),
        };
        for (uint32_t i = 0; i < 6; i++)
        {
            BitmapBufferPtr bit = ImageDecodeFromFile(cube_files[i], ImageType::PNG);
            datas[i] = bit;
        }
        RHITexturePtr tex_cube = rc.CreateTextureCube(desc, datas);
        m_pSkyBoxEntity = MakeSharedPtr<Entity>(m_pContext);
        SkyBoxComponentPtr pSkybox = MakeSharedPtr<SkyBoxComponent>(m_pContext);
        pSkybox->SetSkyBoxTex(tex_cube);
        m_pSkyBoxEntity->AddSceneComponent(pSkybox);
        m_pSkyBoxEntity->AddToTopScene();
    }

    Context* m_pContext = nullptr;
    EntityPtr m_pCameraEntity = nullptr;
    EntityPtr m_pLightEntity[5] = { nullptr };
    EntityPtr m_pSkyBoxEntity = nullptr;
    EntityPtr m_pPlaneEntity = nullptr;
    EntityPtr m_pSphereEntity = nullptr;
    EntityPtr m_pConeEntity = nullptr;
    FirstPersonCameraController m_CameraController;
};

// ============================================================================
// Tutorial — 合并后的主应用
// ============================================================================
static constexpr const char* g_szSceneNames[] = {
    "Basic (Cone)",
    "Particles",
    "SDF (LiquidGlass / Metaball)",
    "Lighting",
};

class Tutorial : public AppFramework
{
public:
    Tutorial() : AppFramework("Tutorial") {}

    virtual SResult OnCreate() override
    {
        m_vScenes.push_back(MakeSharedPtr<SceneBasic>());
        m_vScenes.push_back(MakeSharedPtr<SceneParticles>());
        m_vScenes.push_back(MakeSharedPtr<SceneSDF>());
        m_vScenes.push_back(MakeSharedPtr<SceneLighting>());

        // 直接创建首个场景，不经过 SwitchScene 的判等逻辑
        m_iActiveSceneIndex = 3;
        m_pCurScene = m_vScenes[m_iActiveSceneIndex];
        return m_pCurScene->OnCreate(m_pContext.get());
    }

    virtual SResult OnUpdate() override
    {
        float dt = (float)m_pContext->GetDeltaTime();

        if (m_pCurScene)
            m_pCurScene->OnUpdate(m_pContext.get(), dt);

        SEEK_RETIF_FAIL(m_pContext->Tick());
        SEEK_RETIF_FAIL(m_pContext->BeginRender());
        SEEK_RETIF_FAIL(m_pContext->RenderFrame());

        IMGUI_Begin();
        ShowSceneSelector();
        // SDF 场景显示子场景UI
        if (m_iActiveSceneIndex == 2 && m_pCurScene)
        {
            auto* pSDF = static_cast<SceneSDF*>(m_pCurScene.get());
            pSDF->ShowUI();
        }
        IMGUI_Rendering();

        SEEK_RETIF_FAIL(m_pContext->EndRender());
        return S_Success;
    }

    virtual SResult InitContext(void* device = nullptr, void* native_wnd = nullptr)
    {
        RenderInitInfo info = {};
        info.enable_debug = true;
        info.renderer_type = RendererType::Forward;
        info.preferred_adapter = 0;
        info.HDR = true;

        m_pContext = MakeSharedPtr<Context>(info);
        SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));

        return S_Success;
    }

private:
    void SwitchScene(int index)
    {
        if (index < 0 || index >= (int)m_vScenes.size())
            return;
        if (index == m_iActiveSceneIndex && m_pCurScene)
            return;

        // 销毁旧场景
        if (m_pCurScene)
            m_pCurScene->OnDestroy(m_pContext.get());

        // 创建新场景
        m_iActiveSceneIndex = index;
        m_pCurScene = m_vScenes[index];
        m_pCurScene->OnCreate(m_pContext.get());
    }

    void ShowSceneSelector()
    {
        int comboValue = m_iActiveSceneIndex;
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 80), ImGuiCond_FirstUseEver);
        ImGui::Begin("Scene Selector");
        {
            if (ImGui::Combo("Scene", &comboValue, g_szSceneNames, IM_ARRAYSIZE(g_szSceneNames)))
            {
                SwitchScene(comboValue);
            }
            if (m_pCurScene)
                ImGui::Text("Current: %s", m_pCurScene->GetName());
        }
        ImGui::End();
    }

    std::vector<std::shared_ptr<IScene>> m_vScenes;
    std::shared_ptr<IScene> m_pCurScene = nullptr;
    int m_iActiveSceneIndex = -1;
};

int main()
{
    Tutorial app;
    return APP_RUN(&app);
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
