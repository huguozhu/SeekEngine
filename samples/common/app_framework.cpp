#include "app_framework.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <Shlwapi.h>
#include <wincodec.h>

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!

#define DEFAULT_WND_WIDTH 1280
#define DEFAULT_WND_HEIGHT 720

int APP_RUN(AppFramework* app)
{
    SResult ret = app->Run();
    return ret;
}


SResult AppFramework::InitContext(int width, int height, void* device, void* native_wnd)
{
    m_pContext = MakeSharedPtr<Context>();

    RenderInitInfo info;
    info.device = device;
    info.native_wnd = native_wnd;    
    SEEK_RETIF_FAIL(m_pContext->Init(info));
    return S_Success;
}

SResult AppFramework::RenderFrame()
{
    return S_Success;
}

IWICImagingFactory* g_pIWICFactory;
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ACTIVATE:
        break;

    case WM_PAINT:
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND InitWindow(std::string const& name, uint32_t width, uint32_t height)
{
    uint32_t w = width;
    uint32_t h = height;
    HINSTANCE hInst = GetModuleHandle(nullptr);

    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hIcon = nullptr;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = name.c_str();
    RegisterClassExA(&wc);

    uint32_t style = WS_OVERLAPPEDWINDOW;

    RECT rc = { 0, 0, (LONG)w, (LONG)h };
    AdjustWindowRect(&rc, style, false);
    HWND wnd = CreateWindowA(name.c_str(), name.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
        w, h, nullptr, nullptr, hInst, nullptr);

    ShowWindow(wnd, SW_SHOWNORMAL);
    UpdateWindow(wnd);
    return wnd;
}

SResult AppFramework::Run()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pIWICFactory));

    HWND wnd = InitWindow(m_szName, DEFAULT_WND_WIDTH, DEFAULT_WND_HEIGHT);
    this->InitContext(DEFAULT_WND_WIDTH, DEFAULT_WND_HEIGHT, NULL, (void*)wnd);

    if (!m_bInit)
    {
        SEEK_RETIF_FAIL(this->OnCreate());
        m_bInit = true;
    }
    m_pContext->SetViewport(Viewport(0, 0, DEFAULT_WND_WIDTH, DEFAULT_WND_HEIGHT));

    bool get_msg = false;
    MSG  msg;
    ::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
    while (WM_QUIT != msg.message)
    {
        get_msg = (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0);

        if (get_msg)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            SEEK_RETIF_FAIL(this->OnUpdate());
        }
    }

    this->OnDestroy();
    DestroyWindow(wnd);
    CoUninitialize();
    return S_Success;
}
std::string AppFramework::FullPath(std::string relativePath)
{
#if defined(SEEK_PLATFORM_WINDOWS)
    return std::string{ SEEK_SAMPLES_DIR }.append("/") + relativePath;
#elif defined(SEEK_PLATFORM_MAC)
    return std::string{ SEEK_SAMPLES_DIR }.append("/") + relativePath;
#elif defined(SEEK_PLATFORM_IOS)
    CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    char resourcePath[PATH_MAX];
    if (CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8*)resourcePath, PATH_MAX))
    {
        if (resourceURL != NULL)
        {
            CFRelease(resourceURL);
        }
    }
    return std::string(resourcePath) + "/" + relativePath;
#elif defined(SEEK_PLATFORM_LINUX)
    return std::string{ SEEK_SAMPLES_DIR }.append("/") + relativePath;
#elif defined(SEEK_PLATFORM_ANDROID)
    return std::string{ "/sdcard/seek/" } + relativePath;
#else
    return relativePath;
#endif
}

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
