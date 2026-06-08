#include "rhi/vulkan/vulkan_predeclare.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_translate.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_render_view.h"
#include "rhi/base/viewport.h"
#include "utils/log.h"

#include <windows.h>

#define SEEK_MACRO_FILE_UID 72     //

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkWindow
// ============================================================================
VkWindow::VkWindow(Context* context)
    : RHIFrameBuffer(context)
{
}

VkWindow::~VkWindow()
{
    DestroySwapchainResources();
    if (m_vkSurface != VK_NULL_HANDLE && m_pVkContext)
    {
        vkDestroySurfaceKHR(m_pVkContext->GetVkInstance(), m_vkSurface, nullptr);
        m_vkSurface = VK_NULL_HANDLE;
    }
}

VkFramebuffer VkWindow::GetVkFramebuffer() const
{
    if (m_uCurrentImageIndex < m_vSwapchainFramebuffers.size())
        return m_vSwapchainFramebuffers[m_uCurrentImageIndex];
    return VK_NULL_HANDLE;
}

SResult VkWindow::Create(std::string const& name, void* native_wnd, VkContext* context)
{
    m_pVkContext = context;

    if (SEEK_CHECKFAILED(CreateSurface(native_wnd)))
        return ERR_SYSTEM_ERROR;

    if (SEEK_CHECKFAILED(CreateSwapchain()))
        return ERR_SYSTEM_ERROR;

    // 为每个 swapchain image 创建 RTV 并附加第一个到 framebuffer
    if (!m_vSwapchainTextures.empty())
    {
        m_vSwapchainRtvs.resize(m_uSwapchainImageCount);
        for (uint32_t i = 0; i < m_uSwapchainImageCount; i++)
        {
            m_vSwapchainRtvs[i] = MakeSharedPtr<VkTexture2DCubeRtv>(
                m_pContext, m_vSwapchainTextures[i], 0, 1, 0);
        }
        // 附加第一个 swapchain image 的 RTV
        this->AttachTargetView(Attachment::Color0, m_vSwapchainRtvs[0]);
    }

    if (SEEK_CHECKFAILED(CreateRenderPass()))
        return ERR_SYSTEM_ERROR;

    if (SEEK_CHECKFAILED(CreateImGuiOverlayRenderPass()))
        return ERR_SYSTEM_ERROR;

    if (SEEK_CHECKFAILED(CreateDepthBuffer()))
        return ERR_SYSTEM_ERROR;

    if (SEEK_CHECKFAILED(CreateFramebuffers()))
        return ERR_SYSTEM_ERROR;

    // 创建 per-swapchain-image 的 render complete 信号量（避免不同 image 复用冲突）
    m_pVkContext->CreatePerImageRenderCompleteSemaphores(m_uSwapchainImageCount);

    LOG_INFO("VkWindow created: %dx%d", m_vkSwapchainExtent.width, m_vkSwapchainExtent.height);
    return S_Success;
}

SResult VkWindow::CreateSurface(void* native_wnd)
{
    m_hWnd = static_cast<HWND>(native_wnd);
    if (!m_hWnd)
    {
        LOG_ERROR("Invalid native window handle");
        return ERR_INVALID_ARG;
    }

    VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = m_hWnd;

    if (vkCreateWin32SurfaceKHR(m_pVkContext->GetVkInstance(), &surfaceInfo, nullptr, &m_vkSurface) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create Win32 surface");
        return ERR_SYSTEM_ERROR;
    }

    //
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_pVkContext->GetVkPhysicalDevice(),
        m_pVkContext->GetGraphicsQueueFamily(), m_vkSurface, &presentSupport);
    if (!presentSupport)
    {
        LOG_ERROR("Surface does not support presentation");
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

SResult VkWindow::CreateSwapchain()
{
    VkPhysicalDevice physicalDevice = m_pVkContext->GetVkPhysicalDevice();

    //
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_vkSurface, &capabilities);

    // 查询支持的表面格式
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_vkSurface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_vkSurface, &formatCount, surfaceFormats.data());

    //
    m_vkColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    for (const auto& fmt : surfaceFormats)
    {
        if (fmt.format == VK_FORMAT_R8G8B8A8_UNORM || fmt.format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            m_vkColorFormat = fmt.format;
            colorSpace = fmt.colorSpace;
            break;
        }
    }

    //
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_vkSurface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_vkSurface, &presentModeCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; //
    for (const auto& mode : presentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            break;
        }
    }

    //
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        m_vkSwapchainExtent = capabilities.currentExtent;
    }
    else
    {
        // 从窗口获取大小
        RECT rect;
        GetClientRect(m_hWnd, &rect);
        m_vkSwapchainExtent.width = rect.right - rect.left;
        m_vkSwapchainExtent.height = rect.bottom - rect.top;
    }

    m_vkSwapchainExtent.width = Math::Clamp(m_vkSwapchainExtent.width,
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    m_vkSwapchainExtent.height = Math::Clamp(m_vkSwapchainExtent.height,
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = m_vkSurface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = m_vkColorFormat;
    swapchainInfo.imageColorSpace = colorSpace;
    swapchainInfo.imageExtent = m_vkSwapchainExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_pVkContext->GetVkDevice(), &swapchainInfo, nullptr, &m_vkSwapchain) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create swapchain");
        return ERR_SYSTEM_ERROR;
    }

    //
    vkGetSwapchainImagesKHR(m_pVkContext->GetVkDevice(), m_vkSwapchain, &m_uSwapchainImageCount, nullptr);
    m_vSwapchainImages.resize(m_uSwapchainImageCount);
    vkGetSwapchainImagesKHR(m_pVkContext->GetVkDevice(), m_vkSwapchain, &m_uSwapchainImageCount, m_vSwapchainImages.data());

    //
    m_vSwapchainImageViews.resize(m_uSwapchainImageCount);
    for (uint32_t i = 0; i < m_uSwapchainImageCount; i++)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_vSwapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_vkColorFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(m_pVkContext->GetVkDevice(), &viewInfo, nullptr, &m_vSwapchainImageViews[i]);
    }

    // 为每个 swapchain image 创建纹理封装
    m_vSwapchainTextures.resize(m_uSwapchainImageCount);
    for (uint32_t i = 0; i < m_uSwapchainImageCount; i++)
    {
        m_vSwapchainTextures[i] = MakeSharedPtr<VkTexture2D>(
            m_pContext, m_vSwapchainImages[i], m_vkColorFormat,
            m_vkSwapchainExtent.width, m_vkSwapchainExtent.height, 1);
    }

    // Set viewport
    m_stViewport.left = 0;
    m_stViewport.top = 0;
    m_stViewport.width = static_cast<float>(m_vkSwapchainExtent.width);
    m_stViewport.height = static_cast<float>(m_vkSwapchainExtent.height);

    return S_Success;
}

SResult VkWindow::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_vkColorFormat;
    colorAttachment.samples = m_vkSampleCount;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_vkDepthFormat;
    depthAttachment.samples = m_vkSampleCount;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(m_pVkContext->GetVkDevice(), &renderPassInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create render pass");
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

// ImGui 覆盖层专用 RenderPass — 使用 LOAD_OP_LOAD 保留场景渲染内容
SResult VkWindow::CreateImGuiOverlayRenderPass()
{
    // 颜色附件：LOAD_OP_LOAD 保留已有场景内容，仅在上面叠加 ImGui
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_vkColorFormat;
    colorAttachment.samples = m_vkSampleCount;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;          // 保留场景内容
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 深度附件：LOAD_OP_LOAD 保留场景深度（也可用 DONT_CARE，ImGui 不写深度）
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_vkDepthFormat;
    depthAttachment.samples = m_vkSampleCount;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(m_pVkContext->GetVkDevice(), &renderPassInfo, nullptr, &m_vkImGuiOverlayRenderPass) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create ImGui overlay render pass");
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

SResult VkWindow::CreateDepthBuffer()
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = m_vkDepthFormat;
    imageInfo.extent = { m_vkSwapchainExtent.width, m_vkSwapchainExtent.height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = m_vkSampleCount;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(m_pVkContext->GetVmaAllocator(), &imageInfo, &allocInfo,
        &m_vkDepthImage, &m_vmaDepthAllocation, nullptr) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create depth image");
        return ERR_SYSTEM_ERROR;
    }

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_vkDepthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_vkDepthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_pVkContext->GetVkDevice(), &viewInfo, nullptr, &m_vkDepthImageView) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create depth image view");
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

SResult VkWindow::CreateFramebuffers()
{
    m_vSwapchainFramebuffers.resize(m_uSwapchainImageCount);

    for (uint32_t i = 0; i < m_uSwapchainImageCount; i++)
    {
        std::array<VkImageView, 2> attachments = {
            m_vSwapchainImageViews[i],
            m_vkDepthImageView
        };

        VkFramebufferCreateInfo fbInfo = {};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = m_vkRenderPass;
        fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = m_vkSwapchainExtent.width;
        fbInfo.height = m_vkSwapchainExtent.height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(m_pVkContext->GetVkDevice(), &fbInfo, nullptr,
            &m_vSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create framebuffer %u", i);
            return ERR_SYSTEM_ERROR;
        }
    }

    return S_Success;
}

void VkWindow::DestroySwapchainResources()
{
    if (!m_pVkContext || m_pVkContext->GetVkDevice() == VK_NULL_HANDLE) return;
    VkDevice device = m_pVkContext->GetVkDevice();

    // 清理 swapchain RTV（需在纹理前释放，因为 RTV 持有纹理引用）
    m_vSwapchainRtvs.clear();
    m_vSwapchainTextures.clear();

    for (auto& fb : m_vSwapchainFramebuffers)
    {
        if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(device, fb, nullptr);
    }
    m_vSwapchainFramebuffers.clear();

    if (m_vkDepthImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, m_vkDepthImageView, nullptr);
        m_vkDepthImageView = VK_NULL_HANDLE;
    }
    if (m_vkDepthImage != VK_NULL_HANDLE && m_vmaDepthAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_pVkContext->GetVmaAllocator(), m_vkDepthImage, m_vmaDepthAllocation);
        m_vkDepthImage = VK_NULL_HANDLE;
        m_vmaDepthAllocation = VK_NULL_HANDLE;
    }

    for (auto& view : m_vSwapchainImageViews)
    {
        if (view != VK_NULL_HANDLE) vkDestroyImageView(device, view, nullptr);
    }
    m_vSwapchainImageViews.clear();

    if (m_vkRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, m_vkRenderPass, nullptr);
        m_vkRenderPass = VK_NULL_HANDLE;
    }
    if (m_vkImGuiOverlayRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, m_vkImGuiOverlayRenderPass, nullptr);
        m_vkImGuiOverlayRenderPass = VK_NULL_HANDLE;
    }
    if (m_vkSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, m_vkSwapchain, nullptr);
        m_vkSwapchain = VK_NULL_HANDLE;
    }
}

SResult VkWindow::AcquireNextImage(VkSemaphore semaphore, uint32_t& outImageIndex)
{
    VkResult result = vkAcquireNextImageKHR(m_pVkContext->GetVkDevice(), m_vkSwapchain,
        UINT64_MAX, semaphore, VK_NULL_HANDLE, &outImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapchain();
        return result == VK_ERROR_OUT_OF_DATE_KHR ? ERR_SYSTEM_ERROR : S_Success;
    }

    if (result != VK_SUCCESS)
        return ERR_SYSTEM_ERROR;

    m_uCurrentImageIndex = outImageIndex;
    // 更新当前活跃的 swapchain RTV
    if (outImageIndex < m_vSwapchainRtvs.size() && m_vSwapchainRtvs[outImageIndex])
    {
        this->AttachTargetView(Attachment::Color0, m_vSwapchainRtvs[outImageIndex]);
    }
    return S_Success;
}

SResult VkWindow::Present(VkSemaphore waitSemaphore)
{
    VkSwapchainKHR swapchains[] = { m_vkSwapchain };
    VkSemaphore waitSemaphores[] = { waitSemaphore };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // 当 waitSemaphore 为 VK_NULL_HANDLE 时（如 ImGui 渲染后 GPU 已空闲），无需等待
    presentInfo.waitSemaphoreCount = (waitSemaphore != VK_NULL_HANDLE) ? 1 : 0;
    presentInfo.pWaitSemaphores = (waitSemaphore != VK_NULL_HANDLE) ? waitSemaphores : nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &m_uCurrentImageIndex;

    VkResult result = vkQueuePresentKHR(m_pVkContext->GetVkGraphicsQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to present: %d", result);
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

SResult VkWindow::RecreateSwapchain()
{
    VkDevice device = m_pVkContext->GetVkDevice();
    vkDeviceWaitIdle(device);

    DestroySwapchainResources();
    return CreateSwapchain();
}

SResult VkWindow::OnBind()
{
    //
    return S_Success;
}

SResult VkWindow::OnUnbind()
{
    return S_Success;
}

SResult VkWindow::SwapBuffers()
{
    return S_Success;
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     //




