#pragma once
#include "kernel/kernel.h"
#include "thread/semaphore.h"
#include "rhi/rhi_context.h"
#include "scene_manager/scene_manager.h"


SEEK_NAMESPACE_BEGIN


enum class RHIType
{
    Unknown,
    D3D12,
    Vulkan,
};

struct RenderInitInfo
{
    bool                    debug = false;
    bool                    profile = false;
    bool                    multi_thread = true;
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
    SResult             Update();

    SResult             BeginFrame();
    SResult             RenderFrame();    
    SResult             EndFrame();

    bool                IsMultiThreaded() { return m_InitInfo.multi_thread; }

    RHIContext&         RHIContextInstance() { return *m_pRHIContext; }
    SceneManager&       SceneManagerInstance() { return *m_pSceneManager;}


private:
    bool                ApiSemWait(int32_t msecs = -1);

private:
    RenderInitInfo              m_InitInfo{};
    Semaphore                   m_ApiSemaphore;
    ThreadManagerPtrUnique      m_pThreadManager;
    RHIContextPtrUnique         m_pRHIContext;
    SceneManagerPtrUnique       m_pSceneManager;


};

SEEK_NAMESPACE_END