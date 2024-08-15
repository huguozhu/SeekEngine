#pragma once
#include "kernel/kernel.h"
#include "kernel/context.h"
#include "thread/semaphore.h"


SEEK_NAMESPACE_BEGIN

typedef SResult(*ThreadFn)(Context* context, class Thread* thread, void* _userData);

class Thread
{
public:
    Thread(Context* context);
    ~Thread();

    SResult Init(ThreadFn fn, void* user_data = NULL, uint32_t stack_size = 0, const char* thread_name = NULL);
    void    ShutDown();
    bool    IsRunning() const { return m_bRunning; }
    uint32_t GetExitCode() const { return m_iExitCode; }

    Semaphore& GetSemaphore() { return m_Semaphore; }

private:
    friend struct ThreadInternal;
    SResult     Entry();

private:
    Context*    m_pContext;
    uint8_t     m_Internal[128];

    ThreadFn    m_pThreadFn = nullptr;
    void*       m_pUserData = nullptr;

    Semaphore   m_Semaphore;
    uint32_t    m_iStackSize = 0;
    uint32_t    m_iExitCode = 0;
    bool        m_bRunning = false;
};

SEEK_NAMESPACE_END