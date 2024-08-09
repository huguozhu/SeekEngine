#pragma once
#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN


class Semaphore
{
public:
    Semaphore();
    ~Semaphore();

    void Post(uint32_t count = 1);
    bool Wait(uint32_t msecs = 0xFFFFFFFF);

private:
#if defined(SEEK_PLATFORM_WINDOWS)
    void*  m_Handle;
#endif
};

SEEK_NAMESPACE_END