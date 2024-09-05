#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN


class Mutex
{
public:
    Mutex();
    ~Mutex();

    void Lock();
    void Unlock();

private:
    uint8_t     m_Data[64] = {0};
};


class MutexScope
{
public:
    MutexScope(Mutex& m);
    ~MutexScope();

private:
    Mutex& m_Mutex;
};

SEEK_NAMESPACE_END