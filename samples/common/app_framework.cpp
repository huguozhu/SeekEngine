#include "app_framework.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <Shlwapi.h>
#include <wincodec.h>

#include <d3d11.h>
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_framebuffer.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


int APP_RUN(AppFramework* app)
{
    SResult ret = app->Run();
    return ret;
}

SResult AppFramework::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.debug = false;
    info.device = device;
    info.native_wnd = native_wnd;    
    info.lighting_mode = LightingMode::Phong;
    info.preferred_adapter = 0;
    info.HDR = false;

    m_pContext = MakeSharedPtr<Context>();
    SEEK_RETIF_FAIL(m_pContext->Init(info));

    
    return S_Success;
}
void AppFramework::IMGUI_Begin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void AppFramework::IMGUI_Rendering()
{

    D3D11RHIContext* rc_d3d = static_cast<D3D11RHIContext*>(&m_pContext->RHIContextInstance());
    D3D11RHIFrameBuffer* fb = static_cast<D3D11RHIFrameBuffer*>(rc_d3d->GetFinalRHIFrameBuffer().get());
    ID3D11RenderTargetView* view = fb->GetRenderTargetView();

    ImGui::Render();
    rc_d3d->GetD3D11DeviceContext()->OMSetRenderTargets(1, &view, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
    this->InitContext(NULL, (void*)wnd);
    IMGUI_Init();

    if (!m_bInit)
    {
        SEEK_RETIF_FAIL(this->OnCreate());
        m_bInit = true;
    }

    D3D11RHIContext* rc_d3d = static_cast<D3D11RHIContext*>(&m_pContext->RHIContextInstance());
    ImGui_ImplWin32_Init(wnd);
    ImGui_ImplDX11_Init(rc_d3d->GetD3D11Device(), rc_d3d->GetD3D11DeviceContext());

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
EntityPtr AppFramework::CreateEntityFromFile(std::string filePath)
{
    m_pGLTFLoader = MakeSharedPtr<glTF2_Loader>(m_pContext.get());
    SceneComponentPtr sceneComponent = nullptr;
    std::vector<AnimationComponentPtr> animComponents;
    SResult ret = m_pGLTFLoader->LoadSceneFromFile(filePath, sceneComponent, animComponents);
    if (SEEK_CHECKFAILED(ret) && !sceneComponent) return nullptr;

    EntityPtr mesh_entity = MakeSharedPtr<Entity>(m_pContext.get());
    mesh_entity->SetName("Mesh_Entity");
    mesh_entity->AddSceneComponent(sceneComponent);
    for (size_t i = 0; i < animComponents.size(); i++)
    {
        if (animComponents[i])
        {
            mesh_entity->AddComponent(animComponents[i]);
            animComponents[i]->Play();
        }
    }
    return mesh_entity;
}
#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
