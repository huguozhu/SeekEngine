#include "kernel/context.h"

#include "thread/thread.h"
#include "thread/thread_manager.h"



#include "rhi/rhi_context.h"
#include "utils/log.h"


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
        m_pRHIContext->Init();
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
    }

    m_ApiSemaphore.Post();

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
SResult Context::BeginRenderFrame()
{
    m_pRendererCommandManager->ExecPreCommands();
    return S_Success;
}
// Called by MainThread
SResult Context::RenderFrame()
{
    Thread* pRenderThread = m_pThreadManager->GetRenderThread();
    this->RenderSemWait();

    this->ApiSemWait();
    {

    }
    return S_Success;
}
SResult Context::EndRenderFrame()
{
    m_pRendererCommandManager->ExecPostCommands();
    return S_Success;
}
void Context::ApiSemWait()
{
    if (m_InitInfo.multi_thread)
    {
        m_ApiSemaphore.Wait();
    }
}
void Context::ApiSemPost()
{
    if (m_InitInfo.multi_thread)
    {
        m_ApiSemaphore.Post();
    }
}
void Context::RenderSemWait()
{
    if (m_InitInfo.multi_thread)
    {
        m_pThreadManager->GetRenderThread()->GetSemaphore().Wait();
    }
}
void Context::RenderSemPost()
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