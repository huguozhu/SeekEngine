#pragma once
#include "app_framework.h"
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class Tutorial : public AppFramework
{
public:
    Tutorial();
    virtual ~Tutorial();   


private:
    ContextPtr    m_pContext;
};

