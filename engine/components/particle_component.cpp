#include "components/particle_component.h"
#include "components/camera_component.h"
#include "scene_manager/scene_manager.h"
#include "rhi/mesh.h"
#include "rhi/texture.h"
#include "effect/scene_renderer.h"
#include "effect/effect.h"
#include "effect/technique.h"
#include "kernel/context.h"
#include "rhi/render_buffer.h"
#include "kernel/kernel.h"

#define SEEK_MACRO_FILE_UID 100    // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN
#include "shader/shared/ParticleCommon.h"
#define PARTICLE_DEBUG 0   
static const uint32_t DEBUG_MAX_PARTICLES = 1024; 
static const uint32_t GRADIENT_SAMPLES = 32;
static uint32_t s_ComponentIndex = 0;

enum class TexTimeSamplingType : uint32_t
{
    Play_Once           = Tex_Time_Sampling_Type_Play_Once,
    Play_Loop           = Tex_Time_Sampling_Type_Play_Loop,
    Random_Still_Frame  = Tex_Time_Sampling_Type_Random_Still_Frame,
    Random_Loop         = Tex_Time_Sampling_Type_Random_Loop,
};
enum class EmitShapeType : uint32_t
{
    Sphere = Emit_Shape_Type_Sphere,
    Box = Emit_Shape_Type_Box,
};

int to2power(int i) {
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i++;
    return i;
}

/******************************************************************************
 * ParticleComponent
 ******************************************************************************/
ParticleComponent::ParticleComponent(Context* context, const ParticleSystemParam& param)
    :SceneComponent(context)
{
    m_eComponentType = ComponentType::ParticleSystem;
    m_szName = "ParticleComponent";
    m_Param = param;
    m_iMaxParticles = uint32_t(m_Param.particles_per_sec * m_Param.max_life_time + 15) / 16 * 16;
    m_eState = ParticleState::Stopped;
    m_szName = std::string("Particle_") + std::to_string(s_ComponentIndex++);
    this->Init();
}
ParticleComponent::~ParticleComponent()
{
}
SResult ParticleComponent::Init()
{
    DVF_RETIF_FAIL(this->InitShaders());
    DVF_RETIF_FAIL(this->InitTextures());
    DVF_RETIF_FAIL(this->InitResource());
    DVF_RETIF_FAIL(this->InitParticles());
    return S_Success;
}
SResult ParticleComponent::InitShaders()
{
    static const std::string Tech_Init                  = "ParticleInit";
    static const std::string Tech_Tick_Begin            = "ParticleTickBegin";
    static const std::string Tech_Emit                  = "ParticleEmit";
    static const std::string Tech_Simulate              = "ParticleSimulate";
    static const std::string Tech_Culling               = "ParticleCulling";
    static const std::string Tech_PreSort               = "ParticlePreSort";
    static const std::string Tech_BitonicSort           = "ParticleBitonicSortCS";
    static const std::string Tech_SortMatrixTranspose   = "ParticleSortMatrixTransposeCS";
    static const std::string Tech_Render                = "ParticleRender";
    struct ParticleTechInfo
    {
        const std::string tech_name;
        const char* vertex_shader_name;
        const char* pixel_shader_name;
        const char* compute_shader_name;
    };
    ParticleTechInfo particle_techs[] = {        
        {Tech_Init,                 nullptr, nullptr, "ParticleInitCS"},
        {Tech_Tick_Begin,           nullptr, nullptr, "ParticleTickBeginCS"},
        {Tech_Emit,                 nullptr, nullptr, "ParticleEmitCS"},
        {Tech_Simulate,             nullptr, nullptr, "ParticleSimulateCS"},
        {Tech_Culling,              nullptr, nullptr, "ParticleCullingCS"},
        {Tech_PreSort,              nullptr, nullptr, "ParticlePreSortCS"},
        {Tech_BitonicSort,          nullptr, nullptr, "ParticleBitonicSortCS"},
        {Tech_SortMatrixTranspose,  nullptr, nullptr, "ParticleSortMatrixTransposeCS"},
        {Tech_Render,               "ParticleRenderVS", "ParticleRenderPS", nullptr},
    };
    
    Effect& effect = m_pContext->EffectInstance();
    int tech_count = sizeof(particle_techs) / sizeof(ParticleTechInfo);
    for (size_t i = 0; i < tech_count; ++i)
    {
        const std::string tech_name = particle_techs[i].tech_name;
        if (!effect.GetTechnique(tech_name))
        {
            DVF_RETIF_FAIL(effect.LoadTechnique(tech_name, &RenderStateDesc::Particle(),
                particle_techs[i].vertex_shader_name,
                particle_techs[i].pixel_shader_name,
                particle_techs[i].compute_shader_name));
        }
    }
    m_pTechParticleInit                 = effect.GetTechnique(Tech_Init);
    m_pTechParticleTickBegin            = effect.GetTechnique(Tech_Tick_Begin);
    m_pTechParticleEmit                 = effect.GetTechnique(Tech_Emit);
    m_pTechParticleSimulate             = effect.GetTechnique(Tech_Simulate);
    m_pTechParticleCulling              = effect.GetTechnique(Tech_Culling);
    m_pTechParticlePreSort              = effect.GetTechnique(Tech_PreSort);
    m_pTechParticleBitonicSort          = effect.GetTechnique(Tech_BitonicSort);
    m_pTechParticleSortMatrixTranspose  = effect.GetTechnique(Tech_SortMatrixTranspose);
    
    m_pTechParticleRender       = effect.GetTechnique(Tech_Render, { {"HAS_TEX", "1" } });
    m_pTechParticleRenderNoTex  = effect.GetTechnique(Tech_Render, { {"HAS_TEX", "0" } });
    return S_Success;
}
SResult ParticleComponent::InitTextures()
{
    Texture::Desc desc_color{ TextureType::Tex2D, GRADIENT_SAMPLES, 1, 1, 1, 1, 
        PixelFormat::R8G8B8A8_UNORM, RESOURCE_FLAG_CPU_WRITE };
    m_pTexParticleColorOverLife = m_pContext->RenderContextInstance().CreateTexture2D(desc_color);
    this->UpdateTexture_ColorOverLife();

    Texture::Desc desc_size{ TextureType::Tex2D, GRADIENT_SAMPLES, 1, 1, 1, 1,
        PixelFormat::R32G32F, RESOURCE_FLAG_CPU_WRITE };
    m_pTexParticleSizeOverLife = m_pContext->RenderContextInstance().CreateTexture2D(desc_size);
    this->UpdateTexture_SizeOverLife();
    return S_Success;
}
SResult ParticleComponent::UpdateTexture_ColorOverLife()
{
    float delta = 1.0f / float(GRADIENT_SAMPLES);
    float x = 0.0f;

    std::vector<Color> samples;
    for (uint32_t i = 0; i < GRADIENT_SAMPLES; ++i)
    {
        ColorOverLife::iterator it = m_Param.particle_color_over_life.begin();
        float4 v;
        float x_0 = it->first;
        float4 color_0 = it->second;
        float x_1 = 0;
        float4 color_1;
        for (; it != m_Param.particle_color_over_life.end() - 1; it++)
        {
            ColorOverLife::iterator it_next = it + 1;
            x_1 = it_next->first;
            color_1 = it_next->second;
            if (x < x_1)
                break;
            x_0 = x_1;
            color_0 = color_1;
        }
        if (it == m_Param.particle_color_over_life.end() - 1)
            v = color_0;
        else
        {
            float lerp = (x - x_0) / (x_1 - x_0);
            v = color_0 + (color_1 - color_0) * lerp;
        }
        samples.push_back(Color(v));
        x += delta;
    }
    BitmapBufferPtr bitmap = MakeSharedPtr<BitmapBuffer>(GRADIENT_SAMPLES, 1, PixelFormat::R8G8B8A8_UNORM, (uint8_t*)&samples[0]);
    m_pTexParticleColorOverLife->Update({ bitmap });
    return S_Success;
}
SResult ParticleComponent::UpdateTexture_SizeOverLife()
{
    float delta = 1.0f / float(GRADIENT_SAMPLES);
    float x = 0.0f;

    std::vector<float2> samples;
    for (uint32_t i = 0; i < GRADIENT_SAMPLES; ++i)
    {
        SizeOverLife::iterator it = m_Param.particle_size_over_life.begin();
        float2 v;
        float x_0 = it->first;
        float2 size_0 = it->second;
        float x_1 = 0;
        float2 size_1;
        for (; it != m_Param.particle_size_over_life.end() - 1; it++)
        {
            SizeOverLife::iterator it_next = it + 1;
            x_1 = it_next->first;
            size_1 = it_next->second;
            if (x < x_1)
                break;
            x_0 = x_1;
            size_0 = size_1;
        }
        if (it == m_Param.particle_size_over_life.end() - 1)
            v = size_0;
        else
        {
            float lerp = (x - x_0) / (x_1 - x_0);
            v = size_0 + (size_1 - size_0) * lerp;
        }
        samples.push_back(v);
        x += delta;
    }
    BitmapBufferPtr bitmap = MakeSharedPtr<BitmapBuffer>(GRADIENT_SAMPLES, 1, PixelFormat::R32G32F, (uint8_t*)&samples[0]);
    m_pTexParticleSizeOverLife->Update({ bitmap });
    return S_Success;
}
SResult ParticleComponent::InitResource()
{
    RenderContext& rc = m_pContext->RenderContextInstance();

    m_pParticleInitParam            = rc.CreateConstantBuffer(sizeof(uint), RESOURCE_FLAG_CPU_WRITE);
    m_pParticleTickBeginParam       = rc.CreateConstantBuffer(sizeof(uint32_t),             RESOURCE_FLAG_CPU_WRITE);
    m_pParticleEmitParam            = rc.CreateConstantBuffer(sizeof(GpuEmitParam),         RESOURCE_FLAG_CPU_WRITE);    
    m_pParticleSimulateParam        = rc.CreateConstantBuffer(sizeof(GpuSimulateParam),     RESOURCE_FLAG_CPU_WRITE);
    m_pParticleCullingParam         = rc.CreateConstantBuffer(sizeof(GpuCullingParam),      RESOURCE_FLAG_CPU_WRITE);
    m_pParticleSortParam            = rc.CreateConstantBuffer(sizeof(GpuSortParam),         RESOURCE_FLAG_CPU_WRITE);
    m_pParticleRenderParam          = rc.CreateConstantBuffer(sizeof(GpuRenderParam),       RESOURCE_FLAG_CPU_WRITE);    
    m_pParticleAliveIndicesParam    = rc.CreateConstantBuffer(sizeof(uint2),                RESOURCE_FLAG_CPU_WRITE);
    m_pParticleVertices             = rc.CreateConstantBuffer(sizeof(float4) * 6,           RESOURCE_FLAG_CPU_WRITE);    
    m_pRandomFloats                 = rc.CreateConstantBuffer(sizeof(float) * RANDOM_FLOAT_NUM, RESOURCE_FLAG_CPU_WRITE);

    float4 particle_vertices[6] = {
        float4(-1.0, -1.0, 0.0, 0.0),   // 0
        float4( 1.0, -1.0, 1.0, 0.0),   // 1
        float4(-1.0,  1.0, 0.0, 1.0),   // 2
        float4(-1.0,  1.0, 0.0, 1.0),   // 3
        float4( 1.0, -1.0, 1.0, 0.0),   // 4
        float4( 1.0,  1.0, 1.0, 1.0)    // 5
    };
    m_pParticleVertices->Update(&particle_vertices, sizeof(float4) * 6);
   
    uint32_t max_count = m_iMaxParticles;
    m_pParticleDeadIndices = rc.CreateRWStructuredBuffer(max_count * sizeof(uint),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(uint));
    m_pParticleAliveIndices[0] = rc.CreateRWStructuredBuffer(max_count * sizeof(uint),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(uint));
    m_pParticleAliveIndices[1] = rc.CreateRWStructuredBuffer(max_count * sizeof(uint),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(uint));
    uint32_t sort_capacity = to2power(max_count);
    m_pParticleSortIndices = rc.CreateRWStructuredBuffer(sort_capacity * sizeof(SortInfo),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(SortInfo));
    m_pParticleSortTempIndices = rc.CreateRWStructuredBuffer(sort_capacity * sizeof(SortInfo),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(SortInfo));
    m_pParticleDatas = rc.CreateRWStructuredBuffer(max_count * sizeof(Particle),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(Particle));
    m_pParticleCounters = rc.CreateRWStructuredBuffer(sizeof(ParticleCounters),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_COPY_BACK | RESOURCE_FLAG_GPU_WRITE,
        sizeof(ParticleCounters));

    m_pParticleDrawIndirectArgs = rc.CreateRWStructuredBuffer(sizeof(ParticleDrawArgs),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_DRAW_INDIRECT_ARGS,
        sizeof(ParticleDrawArgs));
    m_pParticleDispatchEmitIndirectArgs = rc.CreateRWStructuredBuffer(sizeof(DispatchArgs),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_DRAW_INDIRECT_ARGS,
        sizeof(DispatchArgs));
    m_pParticleDispatchSimulateIndirectArgs = rc.CreateRWStructuredBuffer(sizeof(DispatchArgs),
        RESOURCE_FLAG_SHADER_RESOURCE | RESOURCE_FLAG_GPU_WRITE | RESOURCE_FLAG_DRAW_INDIRECT_ARGS,
        sizeof(DispatchArgs));

    // Techs set Params
    m_pTechParticleInit->SetParam("CInitParam", m_pParticleInitParam);
    m_pTechParticleInit->SetParam("dead_indices", m_pParticleDeadIndices);
    m_pTechParticleInit->SetParam("particle_counters", m_pParticleCounters);

    m_pTechParticleTickBegin->SetParam("CAliveIndices", m_pParticleAliveIndicesParam);
    m_pTechParticleTickBegin->SetParam("CTickBeginParam", m_pParticleTickBeginParam);
    m_pTechParticleTickBegin->SetParam("particle_counters", m_pParticleCounters);
    m_pTechParticleTickBegin->SetParam("draw_args", m_pParticleDrawIndirectArgs);
    m_pTechParticleTickBegin->SetParam("emit_dispatch_args", m_pParticleDispatchEmitIndirectArgs);
    m_pTechParticleTickBegin->SetParam("simulate_dispatch_args", m_pParticleDispatchSimulateIndirectArgs);

    m_pTechParticleEmit->SetParam("CAliveIndices", m_pParticleAliveIndicesParam);
    m_pTechParticleEmit->SetParam("particle_counters", m_pParticleCounters);
    m_pTechParticleEmit->SetParam("dead_indices", m_pParticleDeadIndices);
    m_pTechParticleEmit->SetParam("CRandom_Floats", m_pRandomFloats);
    m_pTechParticleEmit->SetParam("particle_datas", m_pParticleDatas);
    m_pTechParticleEmit->SetParam("CEmitParam", m_pParticleEmitParam);

    m_pTechParticleSimulate->SetParam("particle_counters", m_pParticleCounters);
    m_pTechParticleSimulate->SetParam("dead_indices", m_pParticleDeadIndices);
    m_pTechParticleSimulate->SetParam("CAliveIndices", m_pParticleAliveIndicesParam);
    m_pTechParticleSimulate->SetParam("CSimulateParam", m_pParticleSimulateParam);
    m_pTechParticleSimulate->SetParam("particle_datas", m_pParticleDatas);
    m_pTechParticleSimulate->SetParam("CRandom_Floats", m_pRandomFloats);

    m_pTechParticleCulling->SetParam("CCullingParam", m_pParticleCullingParam);
    m_pTechParticleCulling->SetParam("sort_indices", m_pParticleSortIndices);
    m_pTechParticleCulling->SetParam("particle_datas", m_pParticleDatas);
    m_pTechParticleCulling->SetParam("particle_counters", m_pParticleCounters);
    m_pTechParticleCulling->SetParam("draw_args", m_pParticleDrawIndirectArgs);
    m_pTechParticleCulling->SetParam("size_over_life_tex", m_pTexParticleSizeOverLife);

    m_pTechParticlePreSort->SetParam("particle_counters", m_pParticleCounters);
    m_pTechParticlePreSort->SetParam("sort_indices", m_pParticleSortIndices);

    m_pTechParticleBitonicSort->SetParam("particle_counters", m_pParticleCounters);
    m_pTechParticleBitonicSort->SetParam("sort_indices", m_pParticleSortIndices);
    m_pTechParticleBitonicSort->SetParam("CSortParam", m_pParticleSortParam);

    m_pTechParticleSortMatrixTranspose->SetParam("CSortParam", m_pParticleSortParam);

    m_pTechParticleRenderNoTex->SetParam("CParticleRenderParam", m_pParticleRenderParam);
    m_pTechParticleRenderNoTex->SetParam("particle_datas", m_pParticleDatas);
    m_pTechParticleRenderNoTex->SetParam("sort_indices", m_pParticleSortIndices);
    m_pTechParticleRenderNoTex->SetParam("CParticleVertices", m_pParticleVertices);
    m_pTechParticleRenderNoTex->SetParam("color_over_life_tex", m_pTexParticleColorOverLife);
    m_pTechParticleRenderNoTex->SetParam("size_over_life_tex", m_pTexParticleSizeOverLife);

    m_pTechParticleRender->SetParam("CParticleRenderParam", m_pParticleRenderParam);
    m_pTechParticleRender->SetParam("particle_datas", m_pParticleDatas);
    m_pTechParticleRender->SetParam("sort_indices", m_pParticleSortIndices);
    m_pTechParticleRender->SetParam("CParticleVertices", m_pParticleVertices);
    m_pTechParticleRender->SetParam("particle_tex", m_Param.particle_tex);
    m_pTechParticleRender->SetParam("color_over_life_tex", m_pTexParticleColorOverLife);
    m_pTechParticleRender->SetParam("size_over_life_tex", m_pTexParticleSizeOverLife);
    return S_Success;
}
SResult ParticleComponent::InitParticles()
{
    m_pParticleInitParam->Update(&m_iMaxParticles, sizeof(uint));
    m_pTechParticleInit->Dispatch((m_iMaxParticles + PARTICLE_CS_X_SIZE - 1) / PARTICLE_CS_X_SIZE, 1, 1);
    return S_Success;
}
SResult ParticleComponent::TickBegin(float delta_time)
{
    float interval_time_emit = m_fElapsed - m_fLastEmitTime;
    uint32_t emit_count = 0;
    float min_emit_time = 1.0f / m_Param.particles_per_sec;
    if (interval_time_emit > min_emit_time)
    {
        emit_count = (uint32_t)(interval_time_emit * m_Param.particles_per_sec);
        m_fLastEmitTime += emit_count * min_emit_time;
    }
    m_pParticleTickBeginParam->Update(&emit_count, sizeof(emit_count));    
    m_pTechParticleTickBegin->Dispatch(1, 1, 1);
    return S_Success;
}
SResult ParticleComponent::EmitParticles()
{
    GpuEmitParam param;
    this->FillGpuEmitParam(&param);
    m_pParticleEmitParam->Update(&param, sizeof(GpuEmitParam));   
    m_pTechParticleEmit->SetParam("alive_pre_simulate_indices", m_pParticleAliveIndices[m_iPreSimIndex]);
    m_pTechParticleEmit->DispatchIndirect(m_pParticleDispatchEmitIndirectArgs);
#if PARTICLE_DEBUG
    {
        ParticleCounters counters = { 0 };
        BufferPtr buf1 = MakeSharedPtr<Buffer>(m_pParticleCounters->GetSize(), (uint8_t*)&counters);
        m_pParticleCounters->CopyBack(buf1);

        Particle datas[DEBUG_MAX_PARTICLES] = { 0.f };
        BufferPtr buf2 = MakeSharedPtr<Buffer>(m_pParticleDatas->GetSize(), (uint8_t*)datas);
        m_pParticleDatas->CopyBack(buf2);

        uint32_t pre_indices[DEBUG_MAX_PARTICLES] = { 0 };
        BufferPtr buf3 = MakeSharedPtr<Buffer>(m_pParticleAliveIndices[m_iPreSimIndex]->GetSize(), (uint8_t*)pre_indices);
        m_pParticleAliveIndices[m_iPreSimIndex]->CopyBack(buf3);

        uint32_t post_indices[DEBUG_MAX_PARTICLES] = { 0 };
        BufferPtr buf4 = MakeSharedPtr<Buffer>(m_pParticleAliveIndices[m_iPostSimIndex]->GetSize(), (uint8_t*)post_indices);
        m_pParticleAliveIndices[m_iPostSimIndex]->CopyBack(buf4);

        uint32_t dead_indices[DEBUG_MAX_PARTICLES] = { 0 };
        BufferPtr buf5 = MakeSharedPtr<Buffer>(m_pParticleDeadIndices->GetSize(), (uint8_t*)dead_indices);
        m_pParticleDeadIndices->CopyBack(buf5);

        buf2 = buf2;
    }
#endif
    return S_Success;
}
SResult ParticleComponent::SimulateParticles(float delta_time)
{   
    GpuSimulateParam param = { 0 };
    this->FillSimulateParam(&param, delta_time);
    m_pParticleSimulateParam->Update(&param, sizeof(GpuSimulateParam));    
    m_pTechParticleSimulate->SetParam("alive_pre_simulate_indices", m_pParticleAliveIndices[m_iPreSimIndex]);
    m_pTechParticleSimulate->SetParam("alive_post_simulate_indices", m_pParticleAliveIndices[m_iPostSimIndex]);    
    m_pTechParticleSimulate->DispatchIndirect(m_pParticleDispatchSimulateIndirectArgs);
    return S_Success;
}
SResult ParticleComponent::CullingParticles()
{
    CameraComponent* pCam = m_pContext->SceneManagerInstance().GetActiveCamera();
    if (!pCam)
        return DVF_ERR_INVALID_INIT;
    float4x4 const& view = pCam->GetViewMatrix().Transpose();
    float4x4 const& proj = pCam->GetProjMatrix().Transpose();
    GpuCullingParam param{ view, proj };
    m_pParticleCullingParam->Update(&param, sizeof(GpuCullingParam));
    m_pTechParticleCulling->SetParam("alive_post_simulate_indices", m_pParticleAliveIndices[m_iPostSimIndex]);
    m_pTechParticleCulling->DispatchIndirect(m_pParticleDispatchSimulateIndirectArgs);
    return S_Success;
}
SResult ParticleComponent::PreSortParticles()
{
    ParticleCounters counters = { 0 };
    BufferPtr buf1 = MakeSharedPtr<Buffer>(m_pParticleCounters->GetSize(), (uint8_t*)&counters);
    m_pParticleCounters->CopyBack(buf1);
    m_pTechParticlePreSort->Dispatch((counters.render_count + BITONIC_BLOCK_SIZE - 1) / BITONIC_BLOCK_SIZE, 1, 1);
    return S_Success;
}
SResult ParticleComponent::SortParticles()
{
    ParticleCounters counters = { 0 };
    BufferPtr buf1 = MakeSharedPtr<Buffer>(m_pParticleCounters->GetSize(), (uint8_t*)&counters);
    m_pParticleCounters->CopyBack(buf1);

    // sort level <= BITONIC_BLOCK_SIZE
    uint size = to2power(counters.render_count);
    for (uint32_t level = 2; level <= size && level <= BITONIC_BLOCK_SIZE; level *= 2)
    {
        GpuSortParam param{ level, level, 0, 0};
        m_pParticleSortParam->Update(&param, sizeof(GpuSortParam));        

        uint32_t dispatch_x = (counters.render_count + BITONIC_BLOCK_SIZE - 1) / BITONIC_BLOCK_SIZE;
        m_pTechParticleBitonicSort->Dispatch(dispatch_x, 1, 1);
    }

    uint32_t matrix_width = 2;
    uint32_t matrix_height = 2;
    while (matrix_width * matrix_width < size)
        matrix_width *= 2;
    matrix_height = size / matrix_width;

    // sort level > BITONIC_BLOCK_SIZE
    for (uint32_t level = BITONIC_BLOCK_SIZE * 2; level <= size; level *= 2)
    {
        GpuSortParam param;
        if (level == size)
            param = { level/matrix_width, level,                matrix_width, matrix_height };
        else
            param = { level/matrix_width, level/ matrix_width,  matrix_width, matrix_height };
        m_pParticleSortParam->Update(&param, sizeof(GpuSortParam));

        // Step1:        
        m_pTechParticleSortMatrixTranspose->SetParam("sort_data_input", m_pParticleSortIndices);
        m_pTechParticleSortMatrixTranspose->SetParam("sort_data_output", m_pParticleSortTempIndices);
        m_pTechParticleSortMatrixTranspose->Dispatch(matrix_width / TRANSPOSE_BLOCK_SIZE,
            matrix_height / TRANSPOSE_BLOCK_SIZE, 1);
        
        // Step2: sort temp datas
        m_pTechParticleBitonicSort->SetParam("sort_indices", m_pParticleSortTempIndices);
        m_pTechParticleBitonicSort->Dispatch(size / BITONIC_BLOCK_SIZE, 1, 1);

        // Step3: transpose datas to SortIndices
        param = { matrix_width, level, matrix_width, matrix_height };
        m_pParticleSortParam->Update(&param, sizeof(GpuSortParam));
        m_pTechParticleSortMatrixTranspose->SetParam("sort_data_input", m_pParticleSortTempIndices);
        m_pTechParticleSortMatrixTranspose->SetParam("sort_data_output", m_pParticleSortIndices);
        m_pTechParticleSortMatrixTranspose->Dispatch(matrix_width / TRANSPOSE_BLOCK_SIZE,
            matrix_height / TRANSPOSE_BLOCK_SIZE, 1);

        // Step4: sort 
        m_pTechParticleBitonicSort->SetParam("sort_indices", m_pParticleSortIndices);
        m_pTechParticleBitonicSort->Dispatch(size / BITONIC_BLOCK_SIZE, 1, 1); 
    }
    return S_Success;
}

float3 ParticleComponent::GetCurEmitPos()
{
    float3 v;
    uint32_t c = (uint32_t)m_Param.positions_over_time.size();
    
    if (c == 0) 
        v = float3(0, 0, 0);
    else if (c == 1) 
        v = m_Param.positions_over_time[0].second;
    else
    {
        float full_time = m_Param.positions_over_time.back().first;
        float cur_time = m_fElapsed - uint32_t(m_fElapsed / full_time) * full_time;

        PosOverTime::iterator it = m_Param.positions_over_time.begin();
        float time_0 = it->first;
        float3 pos_0 = it->second;
        float time_1 = 0;
        float3 pos_1 = float3(0, 0, 0);
        for (; it != m_Param.positions_over_time.end() - 1; it++)
        {
            PosOverTime::iterator it_next = it + 1;
            time_1 = it_next->first;
            pos_1 = it_next->second;
            if (cur_time < time_1)
                break;
            time_0 = time_1;
            pos_0 = pos_1;
        }
        if (it == m_Param.positions_over_time.end() - 1)
            v = pos_0;
        else
        {
            float lerp = (cur_time - time_0) / (time_1 - time_0);
            v = pos_0 + (pos_1 - pos_0) * lerp;
        }
    }
    this->SetLocalTranslation(v);
    v = this->GetWorldTransform().GetTranslation();
    return v;
}
SResult ParticleComponent::FillGpuEmitParam(void* to_gpu_param)
{
    if (!to_gpu_param)
        return ERR_INVALID_ARG;
    GpuEmitParam* target = (GpuEmitParam*)to_gpu_param;
    //target->particle_color  = m_Param.particle_color.ToFloat4();
    target->max_particles   = m_iMaxParticles;
    target->position        = this->GetCurEmitPos();
    target->min_init_speed  = m_Param.min_init_speed;
    target->max_init_speed  = m_Param.max_init_speed;
    target->min_life_time   = m_Param.min_life_time;
    target->max_life_time   = m_Param.max_life_time;

    target->emit_direction_type         = (uint)m_Param.emit_direction_type;
    target->direction                   = m_Param.direction;
    target->direction_spread_percent    = (float)m_Param.direction_spread_percent / 100.0f;
    target->emit_shape                  = (int)m_Param.emit_shape_type;
    target->sphere_radius               = m_Param.sphere_radius;
    target->box_size                    = m_Param.box_size;
    target->tex_time_sampling_type      = (uint)m_Param.tex_time_sampling_type;
    target->tex_rows_cols               = m_Param.tex_rows_cols;
    return S_Success;
}
SResult ParticleComponent::FillSimulateParam(void* param, float delta_time)
{
    if (!param)
        return ERR_INVALID_ARG;
    GpuSimulateParam* target = (GpuSimulateParam*)param;
    target->delta_time              = delta_time;
    target->gravity                 = m_Param.gravity;
    target->tex_time_sampling_type  = (uint)m_Param.tex_time_sampling_type;
    target->tex_rows_cols           = m_Param.tex_rows_cols;
    target->tex_frames_per_sec      = m_Param.tex_frames_per_sec;
    return S_Success;
}
SResult ParticleComponent::Render()
{
    if (m_eState == ParticleState::Stopped)
        return S_Success;

    CameraComponent* pCam = m_pContext->SceneManagerInstance().GetActiveCamera();
    if (!pCam)
        return DVF_ERR_INVALID_INIT;
    float4x4 const& view = pCam->GetViewMatrix().Transpose();
    float4x4 const& proj = pCam->GetProjMatrix().Transpose();
    GpuRenderParam param { view, proj, m_Param.tex_rows_cols };
    m_pParticleRenderParam->Update(&param, sizeof(GpuRenderParam));
    Technique* pTech = m_Param.particle_tex ? m_pTechParticleRender : m_pTechParticleRenderNoTex;
    pTech->DrawIndirect(m_pParticleDrawIndirectArgs, MeshTopologyType::Triangles);
    return S_Success;
}
SResult ParticleComponent::Tick(float delta_time)
{
    if (m_eState == ParticleState::Stopped  ||
        m_eState == ParticleState::Pause    )
        return S_Success;

    m_fElapsed += delta_time;
    if (m_Param.duration != DURATION_INFINITY && m_Param.duration <  m_fElapsed)
        this->Stop();

    uint2 indices = { m_iPreSimIndex, m_iPostSimIndex };
    m_pParticleAliveIndicesParam->Update(&indices, sizeof(uint2));
    float random_floats[RANDOM_FLOAT_NUM];
    for (uint32_t i = 0; i < RANDOM_FLOAT_NUM; ++i)
        random_floats[i] = Math::GenerateRandom(0.0, 1.0);
    m_pRandomFloats->Update(&random_floats, sizeof(float) * RANDOM_FLOAT_NUM);
    DVF_RETIF_FAIL(this->TickBegin(delta_time));
    DVF_RETIF_FAIL(this->EmitParticles());
    DVF_RETIF_FAIL(this->SimulateParticles(delta_time));
    DVF_RETIF_FAIL(this->CullingParticles());
    DVF_RETIF_FAIL(this->PreSortParticles());
    if (m_Param.particle_tex)
        DVF_RETIF_FAIL(this->SortParticles());

    m_iPreSimIndex  = 1 - m_iPreSimIndex;
    m_iPostSimIndex = 1 - m_iPostSimIndex;
    return S_Success;
}
SResult ParticleComponent::Play()
{
    if (m_eState != ParticleState::Stopped)
    {
        LOG_ERROR("Invalid call Play() when not Stopped.");
        return ERR_NOT_SUPPORT;
    }

    this->InitParticles();
    m_fElapsed = 0;
    m_fLastEmitTime = 0;

    m_eState = ParticleState::Playing;
    if (m_pCallback)
        m_pCallback(m_szName, ParticleMsgType::Start, m_pCallbackData);
    return S_Success;
}
SResult ParticleComponent::Pause()
{
    if (m_eState != ParticleState::Playing)
    {
        LOG_ERROR("Invalid call Pause() when not Playing.");
        return ERR_NOT_SUPPORT;
    }

    m_eState = ParticleState::Pause;
    if (m_pCallback)
        m_pCallback(m_szName, ParticleMsgType::Pause, m_pCallbackData);
    return S_Success;

}
SResult ParticleComponent::Resume()
{
    if (m_eState != ParticleState::Pause)
    {
        LOG_ERROR("Invalid call Resume() when not Paused.");
        return ERR_NOT_SUPPORT;
    }

    m_eState = ParticleState::Playing;
    if (m_pCallback)
        m_pCallback(m_szName, ParticleMsgType::Resume, m_pCallbackData);
    return S_Success;
}
SResult ParticleComponent::Stop()
{
    if (m_eState != ParticleState::Playing)
    {
        LOG_ERROR("Invalid call Stop() when not Playing.");
        return ERR_NOT_SUPPORT;
    }

    m_fElapsed = 0;
    m_fLastEmitTime = 0;

    m_eState = ParticleState::Stopped;
    if (m_pCallback)
        m_pCallback(m_szName, ParticleMsgType::Stop, m_pCallbackData);
    return S_Success;
}
void ParticleComponent::RegisterParticleCallback(ParticleCallback cb, void* user_data)
{
    m_pCallback = cb;
    m_pCallbackData = user_data;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!