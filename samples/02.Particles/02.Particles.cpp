#include "02.Particles.h"
#include "components/particle_component.h"
#include "math/color.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!

void Particles::CreateWaterMarkEntity()
{
    WaterMarkComponentPtr pWatermark = MakeSharedPtr<WaterMarkComponent>(m_pContext.get());
    m_pContext->SceneManagerInstance().AddSprite2DComponent(pWatermark);

    RHITexturePtr watermark_tex = nullptr;
    std::string tex_path = FullPath("02.Particles/email_watermark.png");
    BitmapBufferPtr bitmap = ImageDecodeFromFile(tex_path, ImageType::PNG);
    if (bitmap)
    {
        RHITexture::Desc desc;
        desc.width = bitmap->Width();
        desc.height = bitmap->Height();
        desc.type = TextureType::Tex2D;
        desc.format = bitmap->Format();
        desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_CPU_WRITE;
        if (1)
        {
            uint32_t w = bitmap->Width();
            uint32_t h = bitmap->Height();
            uint8_t* data = bitmap->Data();
            uint8_t v = 255;
            for (uint32_t j = 0; j < h; j++)
            {
                for (uint32_t i = 0; i < w; i++)
                {
                    uint8_t* src = data + j * w * 4 + i * 4;
                    if (src[0] >= v && src[1] >= v && src[2] >= v)
                        src[3] = 0;
                }
            }
        }
        watermark_tex = m_pContext->RHIContextInstance().CreateTexture2D(desc, bitmap);
    }
    if (watermark_tex)
    {
        pWatermark->SetWaterMarkTex(watermark_tex);
        WaterMarkDesc desc = { 0 };
        desc.src_width = watermark_tex->Width();
        desc.src_height = watermark_tex->Height();
        desc.radian = Math::PI / 6;
        desc.offset_x = watermark_tex->Width() * 1.5;
        desc.offset_y = watermark_tex->Height() * 5.0;
        desc.watermark_type = WaterMarkType_Single;
        desc.watermark_type = WaterMarkType_Repeat;        
        pWatermark->SetWaterMarkDesc(desc);
    }
}
void Particles::CreateParticleEntities()
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
            for (auto it = pot.MemberBegin(); it<pot.MemberEnd(); ++it)
            {
                params[i].positions_over_time[j].first = (float)atof(it->name.GetString());
                params[i].positions_over_time[j].second = float3(
                    it->value[0].GetFloat(),
                    it->value[1].GetFloat(),
                    it->value[2].GetFloat());
                ++j;
            }
        }
        else
            params[i].positions_over_time.push_back(std::make_pair(0.0f, float3()));
        
        if (node.HasMember("emit_direction_type"))
            params[i].emit_direction_type = (EmitDirectionType)node["emit_direction_type"].GetUint();

        params[i].direction = float3(
            node["direction"][0].GetFloat(),
            node["direction"][1].GetFloat(),
            node["direction"][2].GetFloat());

        params[i].direction_spread_percent = node["direction_spread_percent"].GetFloat();
        params[i].emit_shape_type = (EmitShapeType)node["emit_shape_type"].GetUint();
        params[i].sphere_radius = node["sphere_radius"].GetFloat();

        params[i].box_size = float3(
            node["box_size"][0].GetFloat(),
            node["box_size"][1].GetFloat(),
            node["box_size"][2].GetFloat());
        
        params[i].particles_per_sec = node["particles_per_sec"].GetFloat();
        params[i].min_init_speed = node["min_init_speed"].GetFloat();
        params[i].max_init_speed = node["max_init_speed"].GetFloat();
        params[i].min_life_time = node["min_life_time"].GetFloat();
        params[i].max_life_time = node["max_life_time"].GetFloat();

        params[i].gravity = float3(
            node["gravity"][0].GetFloat(),
            node["gravity"][1].GetFloat(),
            node["gravity"][2].GetFloat());

        params[i].wind = float3(
            node["wind"][0].GetFloat(),
            node["wind"][1].GetFloat(),
            node["wind"][2].GetFloat());


        if (node.HasMember("particle_size"))
        {
            params[i].particle_size_over_life.push_back(std::make_pair(0.0f,
                float2(
                    node["particle_size"][0].GetFloat(),
                    node["particle_size"][1].GetFloat()
                )
            ));
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
                params[i].particle_size_over_life[j].second = float2(
                    it->value[0].GetFloat(),
                    it->value[1].GetFloat()
                );
                ++j;
            }
        }
        else
            params[i].particle_size_over_life.push_back(std::make_pair(0.0f, float2(1, 1)));
        

        if (node.HasMember("particle_color"))
        {
            params[i].particle_color_over_life.push_back(std::make_pair(0.0f, 
            float4(
                node["particle_color"][0].GetFloat(),
                node["particle_color"][1].GetFloat(),
                node["particle_color"][2].GetFloat(),
                node["particle_color"][3].GetFloat()
            )
            ));
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
                    it->value[0].GetFloat(),
                    it->value[1].GetFloat(),
                    it->value[2].GetFloat(),
                    it->value[3].GetFloat()
                );
                ++j;
            }
        }
        else
            params[i].particle_color_over_life.push_back( std::make_pair(0.0f, float4(1.0, 1.0, 1.0, 1.0)));

        params[i].tex_rows_cols = uint2(
            node["tex_rows"].GetUint(),
            node["tex_cols"].GetUint());

        params[i].tex_frames_per_sec = node["tex_frames_per_sec"].GetFloat();
        params[i].tex_time_sampling_type = (TexTimeSamplingType)node["tex_time_sampling_type"].GetUint();

        std::string path = FullPath(std::string(node["tex_path"].GetString()));
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


        EntityPtr pParticleEntity = MakeSharedPtr<Entity>(m_pContext.get(), "ParticleEntity");
        ParticleComponentPtr pParticle = MakeSharedPtr<ParticleComponent>(m_pContext.get(), params[i]);
        pParticle->Play();
        m_pParticleComponentList.push_back(pParticle);
        pParticleEntity->AddSceneComponent(pParticle);

        pParticleEntity->AddToTopScene();
        m_ParticleList.push_back(pParticleEntity);
    }
}
SResult Particles::OnCreate()
{
    RHIContext& rc = m_pContext->RHIContextInstance();
    Viewport const& vp = rc.GetScreenRHIFrameBuffer()->GetViewport();
    float w = vp.width;
    float h = vp.height;

    m_pCameraEntity = MakeSharedPtr<Entity>(m_pContext.get(), "CameraEntity");
    CameraComponentPtr pCam = MakeSharedPtr<CameraComponent>(m_pContext.get());
    pCam->ProjPerspectiveParams(Math::PI / 4, w / h, 0.1f, 1000.0f);
    pCam->SetLookAt(float3(0, 0, -5), float3(0, 0, 0), float3(0, 1, 0));
    m_pCameraEntity->AddSceneComponent(pCam);
    m_pCameraEntity->AddToTopScene();

    m_CameraController.SetCamera(pCam.get());

    CreateParticleEntities();
    CreateWaterMarkEntity();
    return S_Success;
}

SResult Particles::OnUpdate()
{
    //m_CameraController.Update(m_pContext->GetDeltaTime());
    //return m_pContext->Update();

    m_CameraController.Update(m_pContext->GetDeltaTime());
    SEEK_RETIF_FAIL(m_pContext->Tick());
    SEEK_RETIF_FAIL(m_pContext->BeginRender());
    SEEK_RETIF_FAIL(m_pContext->RenderFrame());
    IMGUI_Begin();
    IMGUI_Rendering();
    SEEK_RETIF_FAIL(m_pContext->EndRender());
    return S_Success;
}


#ifndef DVF_PLATFORM_ANDROID
int main()
{
    Particles theApp;
    return APP_RUN(&theApp);
}
#endif

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
