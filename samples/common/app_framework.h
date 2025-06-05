#pragma once
#include "seek_engine.h"
#include "app_imgui.h"

USING_NAMESPACE_SEEK

#define DEFAULT_WND_WIDTH 1280
#define DEFAULT_WND_HEIGHT 720

#define MSAA_NONE 1
#define MSAA_2X 2
#define MSAA_4X 4
#define MSAA_8X 8

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

    virtual SResult         InitContext(void* device = nullptr, void* native_wnd = nullptr);

    virtual void            IMGUI_Begin();
    virtual void            IMGUI_Rendering();

    static std::string      FullPath(std::string relativePath);
    static std::string      Basename(std::string filepath) { return filepath.substr(filepath.rfind("/") + 1); }

    EntityPtr CreateEntityFromFile(std::string filePath);

protected:
    std::string     m_szName;
    ContextPtr      m_pContext = nullptr;
    bool            m_bInit = false;
    LoaderPtr       m_pGLTFLoader = nullptr;

};

int APP_RUN(AppFramework* app);