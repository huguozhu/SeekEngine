#include "kernel/context.h"

#include "thread/thread.h"
#include "thread/thread_manager.h"

#include "rhi/rhi_context.h"
#include "utils/log.h"

#include "utils/timer.h"

SEEK_NAMESPACE_BEGIN

extern "C"
{
    void MakeD3D11RHIContext(Context* context, RHIContextPtrUnique& out);
    void MakeVulkanRHIContext(Context* context, RHIContextPtrUnique& out);
}

Context::Context()
{

}
Context::~Context()
{

}

SResult Context::Init(const RenderInitInfo& init_info)
{
    if (!m_pThreadManager)
    {
        m_pThreadManager = MakeUniquePtrMacro(ThreadManager, this);
        m_pThreadManager->Init();
    }
    if (!m_pRHIContext)
    {
        MakeD3D11RHIContext(this, m_pRHIContext); 
    }
    if (!m_pSceneManager)
    {
        m_pSceneManager = MakeUniquePtrMacro(SceneManager, this);
    }
    if (!m_pResourceManager)
    {
        m_pResourceManager = MakeUniquePtrMacro(ResourceManager, this);
    }
    if (!m_pRendererCommandManager)
    {
        m_pRendererCommandManager = MakeUniquePtrMacro(RendererCommandManager, this);
        m_pRendererCommandManager->InitRendererInit(init_info.native_wnd);
    }

    m_MainThreadSemaphore.Post();

    return S_Success;
}
void Context::Uninit()
{
    m_pThreadManager.reset();
}
void Context::SetViewport(Viewport vp)
{
    if (m_viewport != vp)
    {
        LOG_INFO("viewport is changed, (%d, %d, %u, %u)->(%d, %d, %u, %u)",
            m_viewport.left, m_viewport.top, m_viewport.width, m_viewport.height,
            vp.left, vp.top, vp.width, vp.height);
        m_viewport = vp;
        m_bViewportChanged = true;
    }
}

SResult Context::Update()
{
    return S_Success;
}

SResult Context::BeginRender()
{
    LOG_RECORD_FUNCTION();
    this->RendererCommandManagerInstance().FinishSubmitCommandBuffer();
    this->RendererCommandManagerInstance().SwapCommandBuffer();
    this->RenderThreadSemPost();
    
    if (0)
    {
        static uint32_t MainThread_Index = 0;
        double var = Timer::CurrentTimeSinceEpoch_S();
        LOG_INFO("MainThread Index:  %6d:  time = %20f", MainThread_Index++, var);
        ::_sleep(10);
    }
    return S_Success;
}
/********** Called by Rendering Thread ***************/
SResult Context::RenderFrame()
{
    return S_Success;
}
SResult Context::EndRender()
{
    LOG_RECORD_FUNCTION();
    this->MainThreadSemWait();
    FrameBufferPtr final_fb = this->RHIContextInstance().GetFinalFrameBuffer();
    if (final_fb)
    {
        SEEK_RETIF_FAIL(final_fb->SwapBuffers());
    }
    return S_Success;
}
void Context::MainThreadSemWait()
{
    if (m_InitInfo.multi_thread)
    {
        m_MainThreadSemaphore.Wait();
    }
}
void Context::MainThreadSemPost()
{
    if (m_InitInfo.multi_thread)
    {
        m_MainThreadSemaphore.Post();
    }
}
void Context::RenderThreadSemWait()
{
    if (m_InitInfo.multi_thread)
    {
        m_pThreadManager->GetRenderThread()->GetSemaphore().Wait();
    }
}
void Context::RenderThreadSemPost()
{
    if (m_InitInfo.multi_thread)
    {
        m_pThreadManager->GetRenderThread()->GetSemaphore().Post();
    }
}
SResult Context::ExecCommandBuffers()
{    
    return S_Success;
}
SEEK_NAMESPACE_END