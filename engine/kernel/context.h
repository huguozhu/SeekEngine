#pragma once
#include "kernel/kernel.h"
#include "thread/semaphore.h"
#include "rhi/rhi_context.h"
#include "scene_manager/scene_manager.h"
#include "rhi/viewport.h"
#include "effect/frame.h"
#include "effect/command_buffer.h"


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

class Context
{
public:
    Context();
    ~Context();

    SResult             Init(const RenderInitInfo& init_info);
    void                Uninit();
    void                SetViewport(Viewport vp);
    


    void                ExecuteRendererCommands(CommandBuffer& cb ) {}

    SResult             Update();
    SResult             BeginRenderFrame();
    SResult             RenderFrame();    
    SResult             EndRenderFrame();

    bool                IsMultiThreaded() { return m_InitInfo.multi_thread; }
    bool                IsDebug() { return m_InitInfo.debug; }
    int32_t             GetPreferredAdapter()   const { return m_InitInfo.preferred_adapter; }
    uint32_t            GetNumSamples()         const { return m_InitInfo.num_samples; }
    RHIType             GetRHIType()            const { return m_InitInfo.rhi_type; }

    RHIContext&         RHIContextInstance() { return *m_pRHIContext; }
    SceneManager&       SceneManagerInstance() { return *m_pSceneManager;}

    void                ApiSemWait();
    void                ApiSemPost();
    void                RenderSemWait();
    void                RenderSemPost();
    

private:
    void                SwapFrame();

    RenderInitInfo              m_InitInfo{};
    Semaphore                   m_ApiSemaphore;

    FramePtr                    m_Frames[2] = { nullptr };
    Frame*                      m_pFrameToRender = nullptr;
    Frame*                      m_pFrameToSubmit = nullptr;


    ThreadManagerPtrUnique      m_pThreadManager;
    RHIContextPtrUnique         m_pRHIContext;
    SceneManagerPtrUnique       m_pSceneManager;

    Viewport                    m_viewport;
    bool                        m_bViewportChanged = false;
    

};

SEEK_NAMESPACE_END