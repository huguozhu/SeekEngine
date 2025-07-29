/*************************************************************************************************
**
**      Copyright (C) 2024. All rights reserved.
**
**      Name                   : particle_component.h
**
**      Brief                  : particle component
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2024-05-16  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "components/mesh_component.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/material.h"
#include "rhi/base/rhi_mesh.h"
#include <list>
#include <mutex>

SEEK_NAMESPACE_BEGIN
enum class EmitDirectionType : uint32_t;
enum class EmitShapeType : uint32_t;
enum class TexTimeSamplingType : uint32_t;

enum {
    /** The Particle emitter lives forever. */
    DURATION_INFINITY = -1,
};


typedef std::vector<std::pair<float, float3>> PosOverTime;
typedef std::vector<std::pair<float, float2>>  SizeOverLife;
typedef std::vector<std::pair<float, float4>>  ColorOverLife;
struct ParticleSystemParam
{
    // Main Params
    /** How many seconds the emitter will run. -1 means 'forever' */
    float           duration = -1.0;

    // Emitter
    PosOverTime         positions_over_time;

    EmitDirectionType   emit_direction_type = (EmitDirectionType)1;
    float3              direction = float3(0, 0, 0);
    float               direction_spread_percent = 50;
    EmitShapeType       emit_shape_type = EmitShapeType(0);
    float               sphere_radius = 1.0;
    float3              box_size = float3(1, 1, 0);
    float               particles_per_sec = 300.f;

    // particle params
    float               min_init_speed = 1.0f;
    float               max_init_speed = 1.0f;
    float               min_life_time = 0.1f;
    float               max_life_time = 0.1f;
    uint2               tex_rows_cols = uint2(1, 1);
    float               tex_frames_per_sec = 1;
    TexTimeSamplingType tex_time_sampling_type = TexTimeSamplingType(0);
    RHITexturePtr          particle_tex = nullptr;
    SizeOverLife        particle_size_over_life;
    ColorOverLife       particle_color_over_life;

    // physics
    float3              gravity = float3(0, -10.0, 0);
    float3              wind = float3(0, 0, 0);

};

enum class ParticleMsgType
{
    Start,
    Pause,
    Resume,
    Stop,
};
typedef void (*ParticleCallback)(std::string const& anim_name, ParticleMsgType msg_type, void* user_data);
enum ParticleState : uint32_t
{
    Stopped,
    Playing,
    Pause,
};

class ParticleComponent : public SceneComponent
{
public:
    ParticleComponent(Context* context, const ParticleSystemParam& param);
    virtual ~ParticleComponent();

    SResult             Render();
    virtual SResult     Tick(float delta_time) override;

    SResult Play();
    SResult Pause();
    SResult Resume();
    SResult Stop();
    void RegisterParticleCallback(ParticleCallback cb, void* user_data);

private:
    SResult Init();
    SResult InitShaders();
    SResult InitTextures();
    SResult InitResource();
    SResult InitParticles();

    SResult UpdateTexture_ColorOverLife();
    SResult UpdateTexture_SizeOverLife();

    SResult Tick_GPU(float delta_time);
    SResult TickBegin(float delta_time);
    SResult EmitParticles();
    SResult SimulateParticles(float delta_time);
    SResult CullingParticles();
    SResult SortParticles();

    float3 GetCurEmitPos();
    SResult FillGpuEmitParam(void* to_gpu_param);             // GpuParticleEmitParam*
    SResult FillSimulateParam(void* param, float delta_time); // ParticleSimulateParam*

    void SelectDebugInfo();
    void GpuSyncFence();
    
protected:
    RHIGpuBufferPtr m_pParticleAliveIndicesParam = nullptr;
    RHIGpuBufferPtr m_pParticleInitParam = nullptr;
    RHIGpuBufferPtr m_pParticleTickBeginParam = nullptr;
    RHIGpuBufferPtr m_pParticleEmitParam = nullptr;
    RHIGpuBufferPtr m_pParticleSimulateParam = nullptr;
    RHIGpuBufferPtr m_pParticleCullingParam = nullptr;
    RHIGpuBufferPtr m_pParticleSortParam = nullptr;
    RHIGpuBufferPtr m_pParticleRenderParam = nullptr;
    RHIGpuBufferPtr m_pParticleVertices = nullptr;
    RHIGpuBufferPtr m_pRandomFloats = nullptr;

    RHIGpuBufferPtr m_pParticleCounters = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleCountersUav = nullptr;

    RHIGpuBufferPtr m_pParticleDeadIndices = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleDeadIndicesUav = nullptr;

    RHIGpuBufferPtr m_pParticleAliveIndices[2] = { nullptr };
    RHIUnorderedAccessViewPtr m_pParticleAliveIndicesUav[2] = { nullptr };

    RHIGpuBufferPtr m_pParticleSortIndices = nullptr;
    RHIShaderResourceViewPtr m_pParticleSortIndicesSrv = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleSortIndicesUav = nullptr;

    RHIGpuBufferPtr m_pParticleSortTempIndices = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleSortTempIndicesUav = nullptr;

    RHIGpuBufferPtr m_pParticleDatas = nullptr;
    RHIShaderResourceViewPtr m_pParticleDatasSrv = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleDatasUav = nullptr;
    

    RHIGpuBufferPtr m_pParticleDrawIndirectArgs = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleDrawIndirectArgsUav = nullptr;

    RHIGpuBufferPtr m_pParticleDispatchEmitIndirectArgs = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleDispatchEmitIndirectArgsUav = nullptr;

    RHIGpuBufferPtr m_pParticleDispatchSimulateIndirectArgs = nullptr;
    RHIUnorderedAccessViewPtr m_pParticleDispatchSimulateIndirectArgsUav = nullptr;

    Technique*      m_pTechParticleInit = nullptr;
    Technique*      m_pTechParticleTickBegin = nullptr;
    Technique*      m_pTechParticleEmit = nullptr;
    Technique*      m_pTechParticleSimulate = nullptr;
    Technique*      m_pTechParticleCulling = nullptr;
    Technique*      m_pTechParticlePreSort = nullptr;
    Technique*      m_pTechParticleBitonicSort = nullptr;
    Technique*      m_pTechParticleSortMatrixTranspose = nullptr;
    Technique*      m_pTechParticleRender = nullptr;
    Technique*      m_pTechParticleRenderNoTex = nullptr;

    RHITexturePtr   m_pTexParticleColorOverLife = nullptr;
    RHITexturePtr   m_pTexParticleSizeOverLife = nullptr;

    ParticleSystemParam    m_Param;

    ParticleState       m_eState;
    ParticleCallback    m_pCallback = nullptr;
    void*               m_pCallbackData = nullptr;
    
    //! time elapsed since the start of the particle(in seconds)
    float           m_fElapsed = 0.0;
    float           m_fLastEmitTime = 0.0;

    uint32_t        m_iMaxParticles = 0;
    uint32_t        m_iPreSimIndex = 0;
    uint32_t        m_iPostSimIndex = 1;

    bool            m_bToCallInitParticles = false;
    bool            m_bInit = false;
    float           m_fTickDeltaTime = 0.0f;
    RHIFencePtr     m_pFence = nullptr;

};

SEEK_NAMESPACE_END
