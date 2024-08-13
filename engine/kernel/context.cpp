#include "kernel/context.h"

#include "thread/thread.h"
#include "thread/thread_manager.h"

#include "rhi/rhi_context.h"

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
    return S_Success;
}
void Context::Uninit()
{
    m_pThreadManager.reset();
}

SResult Context::Update()
{
    return S_Success;
}
SResult Context::BeginFrame()
{
    return S_Success;
}
SResult Context::RenderFrame()
{
    return S_Success;
}
SResult Context::EndFrame()
{
    return S_Success;
}

bool Context::ApiSemWait(int32_t msecs)
{
    bool ok = m_ApiSemaphore.Wait(msecs);
    if (ok)
    {

        return true;
    }
    return false;
}
SEEK_NAMESPACE_END