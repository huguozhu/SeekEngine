#include "app_framework.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <Shlwapi.h>
#include <wincodec.h>

#include <d3d11.h>
#include "rhi/d3d11/d3d11_predeclare.h"
#include "rhi/d3d11/d3d11_context.h"
#include "rhi/d3d11/d3d11_framebuffer.h"

#include "rhi/vulkan/vulkan_predeclare.h"
#include "rhi/vulkan/vulkan_context.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_vulkan.h"

#define SEEK_MACRO_FILE_UID 46     // this code is auto generated, don't touch it!!!


int APP_RUN(AppFramework* app)
{
    SResult ret = app->Run();
    return ret;
}

SResult AppFramework::InitContext(void* device, void* native_wnd)
{
    RenderInitInfo info;
    info.enable_debug = true;
    info.preferred_adapter = 0;

    m_pContext = MakeSharedPtr<Context>(info);
    SEEK_RETIF_FAIL(m_pContext->Init(device, native_wnd));
    
    return S_Success;
}
void AppFramework::IMGUI_Begin()
{
    if (m_pContext->GetRHIType() == RHIType::Vulkan)
    {
        // 确保 font texture 已构建（swapchain 重建后可能丢失）
        if (!ImGui::GetIO().Fonts->IsBuilt())
        {
            VkContext* rc_vk = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
            VkCommandBuffer cmdBuf = rc_vk->BeginSingleTimeCommands();
            ImGui_ImplVulkan_CreateFontsTexture(cmdBuf);
            rc_vk->EndSingleTimeCommands(cmdBuf);
        }
        ImGui_ImplVulkan_NewFrame();
    }
    else
    {
        ImGui_ImplDX11_NewFrame();
    }
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void AppFramework::IMGUI_Rendering()
{
    ImGui::Render();

    if (m_pContext->GetRHIType() == RHIType::Vulkan)
    {
        VkContext* rc_vk = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
        VkWindow* window = static_cast<VkWindow*>(rc_vk->GetScreenRHIFrameBuffer().get());

        // 使用当前帧的命令缓冲区（EndFrame 已移至 EndRender，此时仍在录制状态）
        VkCommandBuffer cmdBuf = rc_vk->GetCurrentCommandBuffer();
        if (cmdBuf != VK_NULL_HANDLE && window && rc_vk->IsFrameRecording())
        {
            uint32_t imageIndex = rc_vk->GetCurrentSwapchainImageIndex();
            const auto& swapchainImages = window->GetSwapchainImages();
            if (imageIndex < swapchainImages.size())
            {
                // 场景 EndRenderPass 已将 swapchain image 转为 PRESENT 布局，
                // ImGui overlay RenderPass 要求 COLOR_ATTACHMENT_OPTIMAL，此处做布局转换
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.image = swapchainImages[imageIndex];
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                vkCmdPipelineBarrier(cmdBuf,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

            // 获取 ImGui 覆盖层专用 RenderPass（LOAD_OP_LOAD 保留场景内容）
            VkRenderPass overlayRP = window->GetImGuiOverlayRenderPass();
            VkFramebuffer framebuffer = window->GetVkFramebuffer();
            if (overlayRP != VK_NULL_HANDLE && framebuffer != VK_NULL_HANDLE)
            {
                VkRenderPassBeginInfo rpBegin = {};
                rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                rpBegin.renderPass = overlayRP;
                rpBegin.framebuffer = framebuffer;
                rpBegin.renderArea = { {0, 0}, window->GetExtent() };
                rpBegin.clearValueCount = 0;
                rpBegin.pClearValues = nullptr;

                vkCmdBeginRenderPass(cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
                vkCmdEndRenderPass(cmdBuf);
            }
        }
    }
    else
    {
        D3D11Context* rc_d3d = static_cast<D3D11Context*>(&m_pContext->RHIContextInstance());
        D3D11FrameBuffer* fb = static_cast<D3D11FrameBuffer*>(rc_d3d->GetFinalRHIFrameBuffer().get());
        ID3D11RenderTargetView* view = fb->GetRenderTargetView();
        rc_d3d->GetD3D11DeviceContext()->OMSetRenderTargets(1, &view, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}
SResult AppFramework::RenderFrame()
{
    return S_Success;
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


IWICImagingFactory* g_pIWICFactory;
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
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
AppFramework::AppFramework(std::string const& name)
    :m_szName(name)
{
    IMGUI_Init();
}
SResult AppFramework::Run()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pIWICFactory));

    HWND wnd = InitWindow(m_szName, DEFAULT_WND_WIDTH, DEFAULT_WND_HEIGHT);
    this->InitContext(NULL, (void*)wnd);

    if (!m_bInit)
    {
        SEEK_RETIF_FAIL(this->OnCreate());
        m_bInit = true;
    }
        
    ImGui_ImplWin32_Init(wnd);

    if (m_pContext->GetRHIType() == RHIType::Vulkan)
    {
        VkContext* rc_vk = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
        VkWindow* window = static_cast<VkWindow*>(rc_vk->GetScreenRHIFrameBuffer().get());

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = rc_vk->GetVkInstance();
        init_info.PhysicalDevice = rc_vk->GetVkPhysicalDevice();
        init_info.Device = rc_vk->GetVkDevice();
        init_info.QueueFamily = rc_vk->GetGraphicsQueueFamily();
        init_info.Queue = rc_vk->GetVkGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = rc_vk->GetVkDescriptorPool();
        init_info.Subpass = 0;
        init_info.MinImageCount = window->GetSwapchainImageCount();
        init_info.ImageCount = window->GetSwapchainImageCount();
        init_info.MSAASamples = window->GetSampleCount();

        ImGui_ImplVulkan_Init(&init_info, window->GetVkRenderPass());
    }
    else if (m_pContext->GetRHIType() == RHIType::D3D11)
    {
        D3D11Context* rc_d3d = static_cast<D3D11Context*>(&m_pContext->RHIContextInstance());
        ImGui_ImplDX11_Init(rc_d3d->GetD3D11Device(), rc_d3d->GetD3D11DeviceContext());
    }

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

    // 释放 ImGui 资源（字体纹理、Shader、Buffer、状态对象等），避免泄露
    if (m_pContext->GetRHIType() == RHIType::Vulkan)
    {
        ImGui_ImplVulkan_Shutdown();
    }
    else
    {
        ImGui_ImplDX11_Shutdown();
    }
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

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
