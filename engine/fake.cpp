#include "seek_engine.h"

SEEK_NAMESPACE_BEGIN

void fake()
{
    RenderInitInfo info{};
    Context* pContext = new Context(info);
    
    pContext->Init(nullptr, nullptr);
    delete pContext;
}

SEEK_NAMESPACE_END