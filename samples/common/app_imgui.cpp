#include "app_imgui.h"
#include "app_framework.h"
#include "components/entity.h"
#include "components/mesh_component.h"
#include "components/skeletal_mesh_component.h"
#include "components/light_component.h"
#include "components/camera_component.h"
#include "rhi/base/rhi_mesh.h"
#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

#ifdef DVF_IMGUI

#define UID "##" __LINE_STR__

void IMGUI_Init()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
}

void IMGUI_Setting_Begin()
{
//    ImGui::ShowDemoWindow();

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);
#if !defined(DVF_PLATFORM_IOS)
    ImGui::SetNextWindowSize(ImVec2(DEFAULT_WND_WIDTH / 3, DEFAULT_WND_HEIGHT), ImGuiCond_FirstUseEver);
#endif
    ImGui::Begin("Setting" UID);
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
}

void IMGUI_Setting_End()
{
    ImGui::End();
}

void IMGUI_ShowList(std::string label, std::vector<std::string> lists, int & current_selected, int height, bool needCollaps, bool defaultOpen)
{
    if (needCollaps && ImGui::CollapsingHeader((label+ UID).c_str(), defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0))
    {
        if (ImGui::BeginListBox((label+ UID).c_str(), ImVec2(-FLT_MIN, height * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (size_t n = 0; n < lists.size(); n++)
            {
                const bool is_selected = (current_selected == n);
                if (ImGui::Selectable(lists[n].c_str(), is_selected))
                {
                    current_selected = (int)n;
                }
            }
            ImGui::EndListBox();
        }
    }
}

void IMGUI_ShowCameraList()
{
    static int current_idx = 0;
    if (ImGui::CollapsingHeader("Camera List" UID))
    {
        static std::vector<std::string> cameras;
        if (ImGui::Button("Refresh Camera" UID))
        {
            //cameras = AppFramework::Capture_Refresh();
        }
        ImGui::SameLine();
        if (ImGui::Button("Open Camera" UID))
        {
            //AppFramework::Capture_End();
            //AppFramework::Capture_Begin(current_idx);
        }
        ImGui::SameLine();
        if (ImGui::Button("Close Camera" UID))
        {
            //AppFramework::Capture_End();
        }
        if (ImGui::BeginListBox("Camera List" UID, ImVec2(-FLT_MIN, 3 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (size_t n = 0; n < cameras.size(); n++)
            {
                const bool is_selected = (current_idx == n);
                if (ImGui::Selectable(cameras[n].c_str(), is_selected))
                {
                    current_idx = (int)n;
                }
            }
            ImGui::EndListBox();
        }
    }
    //AppFramework::Capture_DoNext();
}

void IMGUI_ShowTransform(Context* ctx, bool supportMouse)
{
    SceneManager & sm = ctx->SceneManagerInstance();
    CameraComponent* cc = sm.GetActiveCamera();

    float3 position, look_at, up_vec;
    bool look_at_is_change = false;
    if (cc)
    {
        cc->GetLookAt(position, look_at, up_vec);
        if (supportMouse && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            ImGuiIO& io = ImGui::GetIO();
            const float step = 0.1;
            Math::LookAtRotateX(position, look_at, up_vec, io.MouseDelta.x * step);
            Math::LookAtRotateY(position, look_at, up_vec, io.MouseDelta.y * step);
            look_at_is_change = true;
        }
    }

    if (ImGui::CollapsingHeader("Transform" UID, ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (cc)
        {
            if (ImGui::TreeNodeEx("View" UID, ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::DragFloat3("Position" UID, position.data(), 0.1, -100, 100)) look_at_is_change = true;
                if (ImGui::DragFloat3("LookAt" UID, look_at.data(),    0.1, -100, 100)) look_at_is_change = true;
                if (ImGui::DragFloat3("UpVec" UID, up_vec.data(), 0.01, -1, 1))         look_at_is_change = true;
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Proj" UID))
            {
                int width = DEFAULT_WND_WIDTH;      // ctx->GetRenderInitInfo().width;
                int height = DEFAULT_WND_HEIGHT;    // ctx->GetRenderInitInfo().height;

                bool project_type_change = false;
                int project_type = (int)cc->GetCameraProjectionType();
                if (ImGui::RadioButton("Orthographic", &project_type, 1)) project_type_change = true; ImGui::SameLine();
                if (ImGui::RadioButton("Perspective",  &project_type, 0)) project_type_change = true;

                float yfov = cc->GetYFov();
                if (ImGui::DragFloat("fov" UID, &yfov, 0.0087, 0 * Math::PI / 180, 180 * Math::PI / 180))
                    cc->SetYFov(yfov);

                float near_plane = cc->GetNearPlane();
                if (ImGui::DragFloat("near" UID, &near_plane, 0.1, 0.01, 100))
                    cc->SetNearPlane(near_plane);

                float far_plane = cc->GetFarPlane();
                if (ImGui::DragFloat("far" UID, &far_plane, 1, 1, 3000))
                    cc->SetFarPlane(far_plane);

                if (project_type_change)
                {
                    if (project_type == 0)
                        cc->ProjPerspectiveParams(yfov, width * 1.0f / height, near_plane, far_plane);
                    else
                        cc->ProjOrthographicParams(width, height, near_plane, far_plane);
                }

                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::Text("Not found active camera");
            if (ImGui::TreeNodeEx("Model Transform" UID, ImGuiTreeNodeFlags_DefaultOpen))
            {
                static float    transform_model_scale = 1;
                static float    transform_model_yaw = 0;
                static float    transform_model_pitch = 0;
                static float    transform_model_roll = 0;
                static float3   transform_model_translation = float3(0, 0, 0);

                if (ImGui::Button("Reset" UID))
                {
                    transform_model_scale = 1;
                    transform_model_yaw = 0;
                    transform_model_pitch = 0;
                    transform_model_roll = 0;
                    transform_model_translation = float3(0, 0, 0);
                }
                if (!ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    ImGuiIO& io = ImGui::GetIO();
                    const float step = 0.005;
                    transform_model_yaw =   transform_model_yaw - io.MouseDelta.x * step;
                    transform_model_pitch = transform_model_pitch + io.MouseDelta.y * step;
                }
                ImGui::InputFloat("scale",          &transform_model_scale);
                ImGui::SliderFloat("yaw",           &transform_model_yaw,   -Math::PI, Math::PI);
                ImGui::SliderFloat("pitch",         &transform_model_pitch, -Math::PI, Math::PI);
                ImGui::SliderFloat("roll",          &transform_model_roll,  -Math::PI, Math::PI);
                ImGui::InputFloat3("translation",    transform_model_translation.data());
                ImGui::TreePop();

                SceneComponentPtr sc = sm.GetRootComponent();
                if (sc)
                {
                    float3 scale = float3(transform_model_scale);
                    Quaternion quat = Math::FromPitchYawRoll(transform_model_pitch, transform_model_yaw, transform_model_roll);
                    sc->SetLocalScale(scale);
                    sc->SetLocalRotation(quat);
                    sc->SetLocalTranslation(transform_model_translation);
                }
            }
        }
    }

    if (cc && look_at_is_change)
    {
        float3 direct = position - look_at;
        if (Math::LengthSq(direct) > 0.000001)
            cc->SetLookAt(position, look_at, up_vec);
    }
}

void IMGUI_ShowControl(Context* ctx, void* app_framework)
{
    if (ImGui::CollapsingHeader("Control" UID, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Light Mode
        {
            int lightingMode = (int)ctx->GetLightingMode();
            ImGui::RadioButton("Phong", &lightingMode, 0); ImGui::SameLine();
            ImGui::RadioButton("PBR",   &lightingMode, 1);
            if (lightingMode != (int)ctx->GetLightingMode())
            {
                LightingMode lightMode = (LightingMode)lightingMode;
//                bool hdr = lightMode == LightingMode::Phong ? false : true;
//                ctx->SetHDR(hdr);
                ctx->SetLightingMode(lightMode);
            }
        }
        // HDR
        {
            bool b = ctx->IsHDR();
            ImGui::SameLine();
            if (ImGui::Checkbox("HDR", &b))
                ctx->SetHDR(b);
        }
        // AO
        {
            bool b = ctx->EnableAmbientOcclusion();
            ImGui::SameLine();
            if (ImGui::Checkbox("AO", &b))
                ctx->SetEnableAmbientOcclusion(b);
        }
        {
            int antiAliasingMode = (int)ctx->GetAntiAliasingMode();
            
            ImGui::RadioButton("None", &antiAliasingMode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("TAA", &antiAliasingMode, 1);
            ImGui::SameLine();
            ImGui::RadioButton("FXAA", &antiAliasingMode, 2);

            ctx->SetAntiAliasingMode((AntiAliasingMode)antiAliasingMode);
        }
        // Skybox
        {
            static bool have_skybox = false;
            static bool use_skybox = true;
            static Entity* entity = nullptr;

            SkyBoxComponent* skybox = ctx->SceneManagerInstance().GetSkyBoxComponent();
            if (skybox)
            {
                have_skybox = true;
                use_skybox = true;
                entity = skybox->GetOwner();
            }

            if (have_skybox && entity)
            {
                ImGui::SameLine();
                if (ImGui::Checkbox("Sky Box" UID, &use_skybox))
                {
                    if (use_skybox)
                        entity->AddToTopScene();
                    else
                        entity->DeleteFromTopScene();
                }
            }
        }
        // Axis
        {
            static bool show_axis = false;
            static MeshComponentPtr ptr = nullptr;

            if (ImGui::Checkbox("Axis" UID, &show_axis))
            {
                if (show_axis)
                {
                    if (!ptr)
                        ptr = CreateCoordinateAxis(ctx);
                    ctx->SceneManagerInstance().GetRootComponent()->AddChild(ptr);
                }
                else
                    ctx->SceneManagerInstance().GetRootComponent()->DelChild(ptr);
            }
        }
        // Add Light
        {
            static std::vector<EntityPtr> lights; // Entity needs persistent storage
            const float intensity = 1.0;
            if (ImGui::Button("Add Ambient" UID))
            {
                EntityPtr light_entity = MakeSharedPtr<Entity>(ctx, "Ambient Light Entity");
                LightComponentPtr pLight = LightComponent::CreateLightComponent(ctx, LightType::Ambient, "Ambient Light Entity");
                pLight->SetColor(Color(15, 15, 15));
                light_entity->AddSceneComponent(pLight);
                light_entity->AddToTopScene();
                lights.push_back(light_entity);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Directional" UID))
            {
                EntityPtr light_entity = MakeSharedPtr<Entity>(ctx, "Directional Light Entity");

                LightComponentPtr pLight = LightComponent::CreateLightComponent(ctx, LightType::Directional, "Directional Light Entity");
                pLight->SetDirection(float3(0.0, 0.0, 1.0));
                pLight->SetIntensity(intensity);
                pLight->SetColor(Color::White);
                light_entity->AddSceneComponent(pLight);
                light_entity->AddToTopScene();
                lights.push_back(light_entity);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Point" UID))
            {
                EntityPtr light_entity = MakeSharedPtr<Entity>(ctx, "Point Light Entity");
                LightComponentPtr pLight = LightComponent::CreateLightComponent(ctx, LightType::Point, "Point Light");

                pLight->SetIntensity(intensity);
                float3 point_pos = float3(0, 10, 0);
                pLight->SetColor(Color::White);
                pLight->SetLightPos(point_pos);
                
                light_entity->AddSceneComponent(pLight);
                light_entity->AddToTopScene();
                lights.push_back(light_entity);
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Spot" UID))
            {
                EntityPtr light_entity = MakeSharedPtr<Entity>(ctx, "Spot Light Entity");
                LightComponentPtr pLight = LightComponent::CreateLightComponent(ctx, LightType::Spot, "Spot Light");
                pLight->SetIntensity(intensity);
                float3 point_pos = float3(0, 10, 0);
                pLight->SetColor(Color::White);
                pLight->SetLightPos(point_pos);
                
                light_entity->AddSceneComponent(pLight);
                light_entity->AddToTopScene();
                lights.push_back(light_entity);
            }
        }
    }
}

void IMGUI_ShowAnimationControl(int32_t* animationMsg)
{
    if (ImGui::CollapsingHeader("Animation Control" UID, ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int i=0; i<4; i++)
        {
            ImGui::PushID(i);
            ImGui::Text("%d:", i);
            ImGui::SameLine(); ImGui::RadioButton("None" UID,      &animationMsg[i], 0);
            ImGui::SameLine(); ImGui::RadioButton("Play" UID,      &animationMsg[i], 1);
            ImGui::SameLine(); ImGui::RadioButton("Pause" UID,     &animationMsg[i], 2);
            ImGui::SameLine(); ImGui::RadioButton("Resume" UID,    &animationMsg[i], 3);
            ImGui::SameLine(); ImGui::RadioButton("Stop" UID,      &animationMsg[i], 4);
            ImGui::PopID();
        }
    }
}

void ShowMesh(Context* ctx, RHIMeshPtr mesh, int& uid)
{
    uid++;

    ImGui::PushID(uid);
    MaterialPtr material = mesh->GetMaterial();
    if (ImGui::TreeNodeEx(UID, 0, "[Mesh] %s", material->name.c_str()))
    {
        if (ctx->GetLightingMode() == LightingMode::PBR)
        {
            ImGui::Text("");
            if (mesh->HasMorphTarget())
            {
                ImGui::SameLine(); ImGui::Text("[%lu Morphs]", mesh->GetMorphInfo().morph_target_names.size());
            }
            if (mesh->GetSkinningJointBindSize() != SkinningJointBindSize::None)
            {
                ImGui::SameLine(); ImGui::Text("[%d Skinging Joints]", (int)mesh->GetSkinningJointBindSize());
            }

            ImGui::SliderFloat("metallic_factor", &material->metallic_factor, 0., 1.);
            ImGui::SliderFloat("roughness_factor", &material->roughness_factor, 0., 1.);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void SceneRecursion(Context* ctx, SceneComponentPtr sc, int& uid)
{
    uid++;

    ImGui::PushID(uid);
    if (ImGui::TreeNodeEx(UID, 0, "%s", sc->GetName().c_str()))
    {
        ImGui::Text("");

        Transform local = sc->GetLocalTransform();
        if (local.Matrix() != Matrix4::Identity())
        {
            if (local.GetMatDecomposeStatus() == MatDecomposeStatus::No)
            {
                ImGui::SameLine(); ImGui::Text("[Trans NoDecompose]");
            }
            else
            {
                ImGui::SameLine(); ImGui::Text("[Trans]");
            }
        }

        switch (sc->GetComponentType())
        {
            case ComponentType::Mesh:
            case ComponentType::SkeletalMesh:
            {
                MeshComponentPtr mesh = std::dynamic_pointer_cast<MeshComponent>(sc);
                SkeletalMeshComponentPtr skeletalmesh = std::dynamic_pointer_cast<SkeletalMeshComponent>(sc);

                if (sc->GetComponentType() == ComponentType::SkeletalMesh)
                {
                    ImGui::SameLine(); ImGui::Text("[SkeletalMesh Component]");
                    ImGui::SameLine(); ImGui::Text("[%d Joints]", skeletalmesh->GetJointCount());
                }
                else
                {
                    ImGui::SameLine(); ImGui::Text("[Mesh Component]");
                }

                for (size_t i=0; i<mesh->NumMeshes(); i++)
                {
                    ShowMesh(ctx, mesh->GetMeshByIndex(i), uid);
                }

                break;
            }
            case ComponentType::Light:
            {
                ImGui::SameLine(); ImGui::Text("[Light Component]");
                if (ImGui::Button("Delete this Light"))
                {
                    if (sc->GetOwner())
                        sc->GetOwner()->DeleteFromTopScene();
                    else
                        sc->GetParent()->DelChild(sc);
                }

                LightComponentPtr light = std::dynamic_pointer_cast<LightComponent>(sc);
                LightType         lightType = light->GetLightType();

                float3 baseColor = light->GetColor().ToFloat3();
                if (ImGui::ColorEdit3("Base Color", baseColor.data()))
                    light->SetColor(Color(baseColor[0] * 255, baseColor[1] * 255, baseColor[2] * 255));

                if (lightType == LightType::Directional || lightType == LightType::Spot || lightType == LightType::Point)
                {
                    {
                        float intensity = light->GetIntensityOrigin();
                        if (ImGui::DragFloat("Intensity", &intensity, 0, 0.0, 2.0))
                            light->SetIntensity(intensity);
                    }

                    float3 direction = light->GetDirectionOrigin();
                    if (ImGui::InputFloat3("Direction", direction.data()))
                    {
                        if (Math::LengthSq(direction) > 0.000001)
                            light->SetDirection(direction);
                    }

                    if (lightType == LightType::Spot || lightType == LightType::Point)
                    {
                        float3 lightPos = light->GetLightPos();
                        if (ImGui::InputFloat3("Light Pos", lightPos.data()))
                            light->SetLightPos(lightPos);

                        if (ctx->GetLightingMode() == LightingMode::PBR)
                        {
                            float falloffRadius = light->GetFalloffRadius();
                            if (ImGui::InputFloat("Falloff Radius", &falloffRadius))
                                light->SetFalloffRadius(falloffRadius);
                        }

                        if (lightType == LightType::Spot)
                        {
                            SpotLightComponentPtr light_spot = std::dynamic_pointer_cast<SpotLightComponent>(light);

                            float2 inoutcutoff = light_spot->GetInOutCutoff();
                            if (ImGui::InputFloat2("Falloff Radius", inoutcutoff.data()))
                            {
                                light_spot->SetInOutCutoff(inoutcutoff);
                            }
                        }
                    }
                }
                break;
            }
            case ComponentType::Camera:
            {
                ImGui::SameLine(); ImGui::Text("[Camera Component]");
                if (ctx->GetLightingMode() == LightingMode::PBR)
                {
                    CameraComponent* cc = ctx->SceneManagerInstance().GetActiveCamera();
                    if (cc)
                    {
                        float v1, v2, v3;
                        v1 = cc->GetAperture();
                        if (ImGui::SliderFloat("Aperture", &v1, MIN_APERTURE, MAX_APERTURE / 2))
                            cc->SetAperture(v1);

                        v2 = cc->GetShutterSpeed();
                        if (ImGui::SliderFloat("ShutterSpeed", &v2, MIN_SHUTTER_SPEED, MAX_SHUTTER_SPEED / 600))
                            cc->SetShutterSpeed(v2);

                        v3 = cc->GetSensitivity();
                        if (ImGui::SliderFloat("Sensitivity", &v3, MIN_SENSITIVITY, MAX_SENSITIVITY / 100))
                            cc->SetSensitivity(v3);
                    }
                }
                break;
            }
            case ComponentType::Scene:
                ImGui::SameLine(); ImGui::Text("[Scene Component]");
                break;
            case ComponentType::SkyBox:
                ImGui::SameLine(); ImGui::Text("[SkyBox Component]");
                break;
            case ComponentType::Unknown:
            
            case ComponentType::Image:
            case ComponentType::Animation:
            default:
            {
                break;
            }
        }

        for (uint32_t i=0; i<sc->NumChildren(); i++)
        {
            SceneRecursion(ctx, sc->ChildByIndex(i), uid);
        }

        ImGui::TreePop();
    }
    ImGui::PopID();
}

void IMGUI_ShowSceneManager(Context* ctx)
{
    SceneManager & sm = ctx->SceneManagerInstance();
    if (ImGui::CollapsingHeader("Scene Manager" UID, ImGuiTreeNodeFlags_DefaultOpen))
    {
        int uid = 10000;
        for (uint32_t i=0; i<sm.GetRootComponent()->NumChildren(); i++)
        {
            SceneRecursion(ctx, sm.GetRootComponent()->ChildByIndex(i), uid);
        }
    }
}

MeshComponentPtr CreateCoordinateAxis(Context* ctx, float thickness)
{
    std::array<std::array<float, 8 * 6>, 3> vertex_xyz;

    for (int axis = 0; axis < 3; axis++)
    {
        for (int line = 0; line < 6; line++)
        {
            vertex_xyz[axis][line * 8 + 3] = 0; // u
            vertex_xyz[axis][line * 8 + 4] = 0; // v
            vertex_xyz[axis][line * 8 + 5] = 0; // v
            vertex_xyz[axis][line * 8 + 6] = 0; // v
            vertex_xyz[axis][line * 8 + 7] = 0; // v

            if (line < 3)
            {
                vertex_xyz[axis][line * 8 + axis] = 0;
                vertex_xyz[axis][line * 8 + 5 + axis] = -1;
            }
            else
            {
                vertex_xyz[axis][line * 8 + axis] = 9999;
                vertex_xyz[axis][line * 8 + 5 + axis] = 1;
            }

            if (line % 3 == 0)
                vertex_xyz[axis][line * 8 + ((axis + 1) % 3)] = thickness;
            else
                vertex_xyz[axis][line * 8 + ((axis + 1) % 3)] = -thickness;

            if (line % 3 == 0)
                vertex_xyz[axis][line * 8 + ((axis + 2) % 3)] = 0;
            else if (line % 3 == 1)
                vertex_xyz[axis][line * 8 + ((axis + 2) % 3)] = -thickness;
            else
                vertex_xyz[axis][line * 8 + ((axis + 2) % 3)] = thickness;
        }
    }

    std::vector<uint16_t> indics = {
        0, 1, 2, 3, 4, 5,
        0, 1, 3, 3, 1, 4,
        1, 2, 4, 2, 4, 5,
        0, 2, 3, 2, 3, 5,
    };
    RHIRenderBufferData indics_data((uint32_t)indics.size() * sizeof(uint16_t), indics.data());
    RHIRenderBufferPtr indics_buffer = ctx->RHIContextInstance().CreateIndexBuffer(indics_data.m_iDataSize, 0, &indics_data);

    MeshComponentPtr componet = MakeSharedPtr<MeshComponent>(ctx);
    for (int i = 0; i < 3; i++)
    {
        RHIRenderBufferData vertex_data((uint32_t)vertex_xyz[i].size() * sizeof(float), vertex_xyz[i].data());
        RHIRenderBufferPtr vertex_buffer = ctx->RHIContextInstance().CreateVertexBuffer(vertex_data.m_iDataSize, 0, &vertex_data);

        RHIMeshPtr mesh = ctx->RHIContextInstance().CreateMesh();
        mesh->SetIndexBuffer(indics_buffer, IndexBufferType::UInt16);
        mesh->AddVertexStream(vertex_buffer, 0, sizeof(float) * 8, VertexFormat::Float3, VertexElementUsage::Position, 0);
        mesh->AddVertexStream(vertex_buffer, sizeof(float) * 3, sizeof(float) * 8, VertexFormat::Float2, VertexElementUsage::TexCoord, 0);
        mesh->AddVertexStream(vertex_buffer, sizeof(float) * 5, sizeof(float) * 8, VertexFormat::Float3, VertexElementUsage::Normal, 0);
        mesh->SetTopologyType(MeshTopologyType::Triangles);

        MaterialPtr material = MakeSharedPtr<Material>();
        material->albedo_factor = float4(i == 0, i == 1, i == 2, 0);
        mesh->SetMaterial(material);

        componet->AddMesh(mesh);
    }
    return componet;
}

#endif

SEEK_NAMESPACE_END
