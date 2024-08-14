#pragma once
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class AppFramework
{
public:
    AppFramework()
    {}
    virtual ~AppFramework()
    {}
    SResult Run();

    virtual SResult         OnCreate() = 0;
    virtual SResult         OnUpdate() = 0;
    virtual SResult         OnDestroy() { return S_Success; }

    virtual SResult         InitContext(int width, int height, void* device = nullptr, void* native_wnd = nullptr);



private:
    std::string     m_szName;
    Context*        m_pContext;

};

int APP_RUN(AppFramework* app);