#include "thread/thread_manager.h"
#include "thread/thread.h"

#define SEEK_MACRO_FILE_UID 79     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static SResult RenderThread_Entry(Context* context, Thread* render_thread, void* user_data)
{
    if (!context || !render_thread)
        return ERR_INVALID_INIT;

    Context* pContext = (Context*)user_data;
    pContext->ApiSemWait();
    // add to do: call real GPU command


    render_thread->GetSemaphore().Post();
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

Thread* ThreadManager::GetRenderThread()
{
    if (!m_pRenderThread)
        return nullptr;
    return m_pRenderThread.get();
}
SEEK_NAMESPACE_END