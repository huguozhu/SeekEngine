#pragma once
#include "seek_engine.h"

USING_NAMESPACE_SEEK

class AppFramework
{
public:
    AppFramework(std::string const& name)
        :m_szName(name)
    {}
    virtual ~AppFramework()
    {}
    SResult Run();

    virtual SResult         OnCreate() = 0;
    virtual SResult         OnUpdate() = 0;
    virtual SResult         OnDestroy() { return S_Success; }
    virtual SResult         RenderFrame();

    virtual SResult         InitContext(int width, int height, void* device = nullptr, void* native_wnd = nullptr);


private:
    std::string     m_szName;
    ContextPtr      m_pContext = nullptr;
    bool            m_bInit = false;

};

int APP_RUN(AppFramework* app);