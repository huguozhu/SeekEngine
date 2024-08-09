#pragma once
#include "kernel/kernel.h"



SEEK_NAMESPACE_BEGIN


class ThreadManager
{
public:
    ThreadManager(Context* context);
    ~ThreadManager();

    SResult Init();


private:
    Context*    m_pContext = nullptr;
    ThreadPtr   m_pRenderThread = nullptr;
};

SEEK_NAMESPACE_END