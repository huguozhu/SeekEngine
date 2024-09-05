#pragma once
#include "kernel/kernel.h"


SEEK_NAMESPACE_BEGIN





class Frame
{
public:
    Frame(Context* context);
    ~Frame();

private:
    Context*    m_pContext = nullptr;
};


SEEK_NAMESPACE_END