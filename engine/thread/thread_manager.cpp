#include "thread/thread_manager.h"
#include "thread/thread.h"
#include "utils/timer.h"

#define SEEK_MACRO_FILE_UID 79     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

static SResult RenderThread_Entry(Context* context, Thread* render_thread, void* user_data)
{
    while (1)
    {
        if (!context || !render_thread)
            return ERR_INVALID_INIT;

        LOG_RECORD_FUNCTION();
        Context* pContext = (Context*)user_data;
        pContext->RenderThreadSemWait();

        if (1)
        {
            static uint32_t RenderThread_Index = 0;
            double var = Timer::CurrentTimeSinceEpoch_S();
            LOG_INFO("RengeringThread Index:  %6d:    time = %20f ", RenderThread_Index++, var);
            ::_sleep(10);
        }

        // add to do: call real GPU command
        pContext->RendererCommandManagerInstance().ExecPreRenderCommands();
        pContext->RenderFrame();
        pContext->RendererCommandManagerInstance().ExecPostRenderCommands();
        RHIFrameBufferPtr final_fb = pContext->RHIContextInstance().GetFinalRHIFrameBuffer();
        if (final_fb)
        {
            SEEK_RETIF_FAIL(final_fb->SwapBuffers());
        }
        pContext->MainThreadSemPost();
    }
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
        m_pRenderThread->Init(RenderThread_Entry, m_pContext, 0, "RenderThread");
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
