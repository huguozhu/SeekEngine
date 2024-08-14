#include "seek_engine.h"

SEEK_NAMESPACE_BEGIN

void fake()
{

    Context* pContext = new Context();
    RenderInitInfo info{};
    pContext->Init(info);
    delete pContext;
}

SEEK_NAMESPACE_END