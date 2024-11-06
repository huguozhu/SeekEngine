#pragma once
#include "kernel/kernel.h"
#include "thread/semaphore.h"
#include "rhi/base/rhi_context.h"
#include "scene_manager/scene_manager.h"
#include "rhi/base/viewport.h"
#include "effect/command_buffer.h"
#include "resource/resource_mgr.h"


SEEK_NAMESPACE_BEGIN


enum class RHIType
{
    Unknown,
    D3D11,
    D3D12,
    Vulkan,
    Metal,
    GLES,
};

enum class LightingMode
{
    Phong,
    PBR,
};
static const float PBR_INTENSITY_COEFF = 120000.0f;

struct RenderInitInfo
{
    bool                    debug = false;
    bool                    profile = false;
    bool                    multi_thread = true;
    bool                    immediate_rendering_mode = false;
    RHIType                 rhi_type = RHIType::D3D11;
    bool                    HDR = false;
    uint32_t                num_samples = 1;
    int32_t                 preferred_adapter = 0;
    LightingMode            lighting_mode = LightingMode::Phong;
    RendererType            renderer_type = RendererType::Forward;
    void*                   native_wnd = nullptr;
    void*                   device = nullptr;
    bool                    enable_transparent = false;
    bool                    enable_ambient_occlusion = false;
    bool                    enable_capture = false;
    //AntiAliasingMode        anti_aliasing_mode = AntiAliasingMode::None;
};

class Context
{
public:
    Context();
    ~Context();

    SResult             Init(const RenderInitInfo& init_info);
    void                Uninit();
    void                SetViewport(Viewport vp);
    

    SResult             Update();
    SResult             BeginRender();
    SResult             RenderFrame();   // Called by Rendering Thread 
    SResult             EndRender();

    void                SetClearColor(float4 color) { m_fClearColor = color; }
    float4              GetClearColor() const { return m_fClearColor; }

    RenderInitInfo&     GetRenderInitInfo()           { return m_InitInfo; }
    bool                IsMultiThreaded()       const { return m_InitInfo.multi_thread; }
    bool                IsDebug()               const { return m_InitInfo.debug; }
    int32_t             GetPreferredAdapter()   const { return m_InitInfo.preferred_adapter; }
    uint32_t            GetNumSamples()         const { return m_InitInfo.num_samples; }
    bool                IsHDR()                 const { return m_InitInfo.HDR; }
    RHIType             GetRHIType()            const { return m_InitInfo.rhi_type; }
    LightingMode        GetLightingMode()       const { return m_InitInfo.lighting_mode; }

    RHIContext&         RHIContextInstance() { return *m_pRHIContext; }
    SceneManager&       SceneManagerInstance() { return *m_pSceneManager;}
    SceneRenderer&      SceneRendererInstance() { return *m_pSceneRenderer; }
    ResourceManager&    ResourceManagerInstance() { return *m_pResourceManager; }
    RendererCommandManager& RendererCommandManagerInstance() { return *m_pRendererCommandManager; }
    Effect&             EffectInstance() { return *(m_pEffect.get()); }

    void                MainThreadSemWait();
    void                MainThreadSemPost();
    void                RenderThreadSemWait();
    void                RenderThreadSemPost();
    
    const RHITexturePtr& GetIBLDiffuseTexture() { return m_pIBLDiffuseTex; }
    const RHITexturePtr& GetIBLSpecularTexture() { return m_pIBLSpecularTex; }
    const RHITexturePtr& GetIBLBrdfTex() { return m_pIBLBrdfTex; }
    bool HasPrecomputedIBL() { return m_pIBLDiffuseTex && m_pIBLSpecularTex && m_pIBLBrdfTex; }

private:
    SResult InitRHIContext();

private:
    RenderInitInfo              m_InitInfo{};
    Semaphore                   m_MainThreadSemaphore;

    ThreadManagerPtrUnique      m_pThreadManager;
    RHIContextPtrUnique         m_pRHIContext;
    SceneManagerPtrUnique       m_pSceneManager;
    SceneRendererPtrUnique      m_pSceneRenderer;
    ResourceManagerPtrUnique    m_pResourceManager;
    RendererCommandManagerPtrUnique     m_pRendererCommandManager;
    EffectPtrUnique             m_pEffect;

    uint32_t                    m_FrameCount = 0;

    // Time
    TimerPtrUnique              m_pTimer = MakeUniquePtr<Timer>();
    double                      m_dCurTime = 0.0;
    double                      m_dDeltaTime = 0.0;


    float4                      m_fClearColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
    Viewport                    m_sViewport;
    bool                        m_bViewportChanged = false;

    RHITexturePtr               m_pIBLDiffuseTex = nullptr;
    RHITexturePtr               m_pIBLSpecularTex = nullptr;
    RHITexturePtr               m_pIBLBrdfTex = nullptr;

};

SEEK_NAMESPACE_END