#pragma once
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class Tutorial
{
public:
    Tutorial();
    virtual ~Tutorial();
    SResult Run();


private:
    ContextPtr    m_pContext;
};

