#pragma once
#include "kernel/kernel.h"
#include "thread/semaphore.h"
#include "rhi/base/rhi_context.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/viewport.h"
#include "resource/resource_mgr.h"


SEEK_NAMESPACE_BEGIN

enum class RendererType : uint32_t
{
    Unknown,
    Forward,
    Deferred,
};

enum class RHIType : uint32_t
{
    Unknown,
    D3D11,
    D3D12,      // not supported fully
    Vulkan,     // not supported fully
    Metal,      // not supported yet
    GLES,       // not supported yet
};

enum class LightingMode : uint32_t
{
    Phong,
    PBR,
};
static const float PBR_INTENSITY_COEFF = 120000.0f;

enum class AntiAliasingMode : uint32_t
{
    None,
    TAA,
    FXAA,
};
enum class FPSLimitType : uint32_t
{
    NoLImit,
    FPS_30,
    FPS_60,
    FPS_120,
};
enum class GlobalIlluminationMode : uint32_t
{
    None,
    RSM,
    LPV,      // Light Propagation Volumes Global Illumination(Used in Cry Engine3)
    //VXGI,     // Voxel-based Global Illumination
    //SSGI,     // Screen Space Global Illumination
};
struct RenderInitInfo
{
    uint32_t                enable_debug = false;
    uint32_t                enable_profile = false;
    uint32_t                enable_transparent = false;
    uint32_t                enable_ambient_occlusion = false;
    uint32_t                enable_capture = false;
    uint32_t                HDR = false;
    RHIType                 rhi_type = RHIType::D3D11;
    uint32_t                num_samples = 1;
    int32_t                 preferred_adapter = 0;
    LightingMode            lighting_mode = LightingMode::Phong;
    RendererType            renderer_type = RendererType::Forward;

    AntiAliasingMode        anti_aliasing_mode = AntiAliasingMode::None;
    FPSLimitType            fps_limit_type = FPSLimitType::FPS_60;
    GlobalIlluminationMode  gi_mode = GlobalIlluminationMode::None;

};

class Context
{
public:
    Context(const RenderInitInfo& init_info);
    ~Context();

    SResult                 Init(void* device, void* native_wnd);
    void                    Uninit();
    void                    SetViewport(Viewport vp);
    Viewport                GetViewport() const { return m_sViewport; }
    
    SResult                 Tick();
    SResult                 Update();
    SResult                 BeginRender();
    SResult                 RenderFrame();   // Called by Rendering Thread 
    SResult                 EndRender();

    void                    SetClearColor(float4 color) { m_fClearColor = color; }
    float4                  GetClearColor() const { return m_fClearColor; }

    bool                    EnableProfile()             const { return m_InitInfo.enable_profile; }
    bool                    EnableDebug()               const { return m_InitInfo.enable_debug; }
    bool                    EnableTransparent()         const { return m_InitInfo.enable_transparent; }
    bool                    EnableAmbientOcclusion()    const { return m_InitInfo.enable_ambient_occlusion;}  
    int32_t                 GetPreferredAdapter()       const { return m_InitInfo.preferred_adapter; }
    uint32_t                GetNumSamples()             const { return m_InitInfo.num_samples; }
    bool                    IsHDR()                     const { return m_InitInfo.HDR; }
    RHIType                 GetRHIType()                const { return m_InitInfo.rhi_type; }
    LightingMode            GetLightingMode()           const { return m_InitInfo.lighting_mode; }
	RendererType            GetRendererType()           const { return m_InitInfo.renderer_type; }
    AntiAliasingMode        GetAntiAliasingMode()       const { return m_InitInfo.anti_aliasing_mode; }
	GlobalIlluminationMode  GetGlobalIlluminationMode() const { return m_InitInfo.gi_mode; }

    uint32_t                GetFrameCount()             const { return m_FrameCount; }
    double                  GetCurTime()                const { return m_dCurTime; }
    double                  GetDeltaTime()              const { return m_dDeltaTime; }

    // For edit/debug mode
    void                SetLightingMode(LightingMode lightMode) { m_InitInfo.lighting_mode = lightMode; }
    void                SetAntiAliasingMode(AntiAliasingMode mode) { m_InitInfo.anti_aliasing_mode = mode; }
    void                SetHDR(bool b) { m_InitInfo.HDR = b; }
    void                SetEnableTransparent(bool b) { m_InitInfo.enable_transparent = b; }
    void                SetEnableAmbientOcclusion(bool b) { m_InitInfo.enable_ambient_occlusion = b; }
    void                SetFpsLimitType(FPSLimitType b);

    RHIContext&         RHIContextInstance() { return *m_pRHIContext; }
    SceneManager&       SceneManagerInstance() { return *m_pSceneManager;}
    SceneRenderer&      SceneRendererInstance() { return *m_pSceneRenderer; }
    ResourceManager&    ResourceManagerInstance() { return *m_pResourceManager; }
    Effect&             EffectInstance() { return *(m_pEffect.get()); }
    
    const RHITexturePtr& GetIBLDiffuseTexture() { return m_pIBLDiffuseTex; }
    const RHITexturePtr& GetIBLSpecularTexture() { return m_pIBLSpecularTex; }
    const RHITexturePtr& GetIBLBrdfTex() { return m_pIBLBrdfTex; }
    bool HasPrecomputedIBL() { return m_pIBLDiffuseTex && m_pIBLSpecularTex && m_pIBLBrdfTex; }

private:
    SResult InitRHIContext();

private:
    RenderInitInfo              m_InitInfo{};

    RHIContextPtrUnique         m_pRHIContext;
    SceneManagerPtrUnique       m_pSceneManager;
    SceneRendererPtrUnique      m_pSceneRenderer;
    ResourceManagerPtrUnique    m_pResourceManager;
    EffectPtrUnique             m_pEffect;

    uint32_t                    m_FrameCount = 0;

    // Time
    TimerPtrUnique              m_pTimer = MakeUniquePtr<Timer>();
    double                      m_dCurTime = 0.0;
    double                      m_dDeltaTime = 0.0;


    float4                      m_fClearColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    Viewport                    m_sViewport;
    bool                        m_bViewportChanged = false;

    RHITexturePtr               m_pIBLDiffuseTex = nullptr;
    RHITexturePtr               m_pIBLSpecularTex = nullptr;
    RHITexturePtr               m_pIBLBrdfTex = nullptr;

    float                       m_fMinFrameTime = 0.0;

};

SEEK_NAMESPACE_END