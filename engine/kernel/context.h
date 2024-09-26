#pragma once
#include "kernel/kernel.h"
#include "thread/semaphore.h"
#include "rhi/rhi_context.h"
#include "scene_manager/scene_manager.h"
#include "rhi/viewport.h"
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

struct RenderInitInfo
{
    bool                    debug = false;
    bool                    profile = false;
    bool                    multi_thread = true;
    bool                    immediate_rendering_mode = false;
    RHIType                 rhi_type = RHIType::Unknown;
    uint32_t                num_samples = 1;
    int32_t                 preferred_adapter = -1;
    void*                   native_wnd = nullptr;
    void*                   device = nullptr;
};

class CommandBuffer;
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

    bool                IsMultiThreaded() { return m_InitInfo.multi_thread; }
    bool                IsDebug() { return m_InitInfo.debug; }
    int32_t             GetPreferredAdapter()   const { return m_InitInfo.preferred_adapter; }
    uint32_t            GetNumSamples()         const { return m_InitInfo.num_samples; }
    RHIType             GetRHIType()            const { return m_InitInfo.rhi_type; }

    RHIContext&         RHIContextInstance() { return *m_pRHIContext; }
    SceneManager&       SceneManagerInstance() { return *m_pSceneManager;}
    ResourceManager&    ResourceManagerInstance() { return *m_pResourceManager; }
    RendererCommandManager& RendererCommandManagerInstance() { return *m_pRendererCommandManager; }

    void                MainThreadSemWait();
    void                MainThreadSemPost();
    void                RenderThreadSemWait();
    void                RenderThreadSemPost();
    

private:

    RenderInitInfo              m_InitInfo{};
    Semaphore                   m_MainThreadSemaphore;

    ThreadManagerPtrUnique      m_pThreadManager;
    RHIContextPtrUnique         m_pRHIContext;
    SceneManagerPtrUnique       m_pSceneManager;
    ResourceManagerPtrUnique    m_pResourceManager;
    RendererCommandManagerPtrUnique     m_pRendererCommandManager;

    Viewport                    m_viewport;
    bool                        m_bViewportChanged = false;
    

};

SEEK_NAMESPACE_END