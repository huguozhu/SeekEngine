#include "thread/thread_manager.h"
#include "thread/thread.h"

SEEK_NAMESPACE_BEGIN


static SResult RenderThread_Entry(Context* context, Thread* thread, void* user_data)
{
    Context* pContext = (Context*)user_data;
    pContext->BeginFrame();
    pContext->Render();
    pContext->EndFrame();
    return S_Success;
}


ThreadManager::ThreadManager(Context* context)
    :m_pContext(context)
{
    m_pRenderThread = MakeSharedPtr<Thread>(m_pContext);
}


ThreadManager::~ThreadManager()
{

}

SResult ThreadManager::Init()
{
    if (m_pRenderThread)
    {
        m_pRenderThread->Init(RenderThread_Entry, nullptr);
    }

    return S_Success;
}

SEEK_NAMESPACE_END