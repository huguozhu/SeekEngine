#include "kernel/context.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_texture.h"
#include "rhi/vulkan/vulkan_gpu_buffer.h"
#include "rhi/vulkan/vulkan_framebuffer.h"
#include "rhi/vulkan/vulkan_program.h"
#include "rhi/vulkan/vulkan_shader.h"
#include "rhi/vulkan/vulkan_mesh.h"
#include "rhi/vulkan/vulkan_render_state.h"
#include "rhi/vulkan/vulkan_render_view.h"
#include "rhi/vulkan/vulkan_fence.h"
#include "rhi/vulkan/vulkan_query.h"
#include "rhi/vulkan/vulkan_translate.h"

#include "rhi/base/rhi_mesh.h"
#include "rhi/base/rhi_program.h"
#include "utils/log.h"

#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#define SEEK_MACRO_FILE_UID 70     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

// ============================================================================
// 闈欐€佸彉閲?鈥?Vulkan 鍔ㄦ€佸姞杞?// ============================================================================
static DllLoader s_vulkan("vulkan-1.dll");

// 璋冭瘯鍥炶皟
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOG_ERROR("[Vulkan] %s", pCallbackData->pMessage);
    } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_WARNING("[Vulkan] %s", pCallbackData->pMessage);
    }
    return VK_FALSE;
}

// ============================================================================
// VkContext 鏋勯€?鏋愭瀯
// ============================================================================
VkContext::VkContext(Context* context)
    : RHIContext(context)
{
}

VkContext::~VkContext()
{
    Uninit();
}

// ============================================================================
// Vulkan Instance 鍒涘缓
// ============================================================================
bool VkContext::CreateVulkanInstance()
{
    // 妫€鏌?Vulkan API 鐗堟湰
    uint32_t apiVersion = VK_API_VERSION_1_3;
    {
        uint32_t supportedVersion;
        vkEnumerateInstanceVersion(&supportedVersion);
        if (supportedVersion < VK_API_VERSION_1_3)
        {
            LOG_INFO("Vulkan 1.3 not available, falling back to 1.2");
            apiVersion = VK_API_VERSION_1_2;
            m_bUseDynamicRendering = false;
        }
        else
        {
            m_bUseDynamicRendering = true;
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "SeekEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "SeekEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = apiVersion;

    // 鏀堕泦闇€瑕佺殑鎵╁睍
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    if (m_bEnableDebug)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    // Vulkan 1.2+ portability enumeration 鍦ㄦ煇浜涘钩鍙帮紙濡?MoltenVK锛夐渶瑕?    // 涓嶅仛寮哄埗瑕佹眰锛屽厛鏌ユ槸鍚︽敮鎸?    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Validation layers
    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    if (m_bEnableDebug)
    {
        // 妫€鏌?validation layer 鏄惁鍙敤
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        bool layerFound = false;
        for (const auto& layer : availableLayers)
        {
            if (strcmp(layer.layerName, validationLayer) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (layerFound)
        {
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayer;
        }
        else
        {
            LOG_WARNING("Validation layer not available, debug mode will be limited");
        }
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateInstance failed: %d", result);
        return false;
    }

    // 璁剧疆璋冭瘯鍥炶皟
    if (m_bEnableDebug)
    {
        SetupValidationLayers();
    }

    LOG_INFO("Vulkan instance created (API %d.%d.%d)",
        VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));
    return true;
}

void VkContext::SetupValidationLayers()
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT");
    if (!func) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    func(m_vkInstance, &createInfo, nullptr, &m_vkDebugMessenger);
}

// ============================================================================
// Vulkan Device 鍒涘缓锛堥€夋嫨鐗╃悊璁惧銆佸垱寤洪€昏緫璁惧锛?// ============================================================================
bool VkContext::CreateVulkanDevice()
{
    // 鏋氫妇鐗╃悊璁惧
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        LOG_ERROR("No Vulkan physical devices found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

    // 鏍规嵁 m_pContext->GetPreferredAdapter() 閫夋嫨璁惧
    int32_t preferredIndex = m_pContext->GetPreferredAdapter();
    if (preferredIndex < 0 || preferredIndex >= static_cast<int32_t>(deviceCount))
        preferredIndex = 0;

    m_vkPhysicalDevice = devices[preferredIndex];

    // 鑾峰彇璁惧灞炴€?    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &deviceProps);
    LOG_INFO("Selected Vulkan GPU: %s", deviceProps.deviceName);

    // 鏌ユ壘闃熷垪鏃?    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    m_uGraphicsQueueFamily = UINT32_MAX;
    m_uComputeQueueFamily = UINT32_MAX;
    m_uPresentQueueFamily = UINT32_MAX;

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            if (m_uGraphicsQueueFamily == UINT32_MAX)
                m_uGraphicsQueueFamily = i;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            // 浼樺厛閫夋嫨浠?compute 鐨勯槦鍒楁棌
            if (m_uComputeQueueFamily == UINT32_MAX || !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                m_uComputeQueueFamily = i;
        }
    }

    // 濡傛灉娌℃壘鍒扮嫭绔嬬殑 compute 闃熷垪锛屽鐢?graphics 闃熷垪
    if (m_uComputeQueueFamily == UINT32_MAX)
        m_uComputeQueueFamily = m_uGraphicsQueueFamily;

    // Present 鏀寔锛堝欢杩熸鏌?鈥?闇€瑕?surface锛屽湪 AttachNativeWindow 涓鐞嗭級

    if (m_uGraphicsQueueFamily == UINT32_MAX)
    {
        LOG_ERROR("No graphics queue family found");
        return false;
    }

    // 鍒涘缓閫昏緫璁惧
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo gfxQueueInfo = {};
    gfxQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    gfxQueueInfo.queueFamilyIndex = m_uGraphicsQueueFamily;
    gfxQueueInfo.queueCount = 1;
    gfxQueueInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(gfxQueueInfo);

    // 濡傛灉 compute 闃熷垪鏃忎笌 graphics 涓嶅悓锛屽崟鐙垱寤?    if (m_uComputeQueueFamily != m_uGraphicsQueueFamily)
    {
        VkDeviceQueueCreateInfo compQueueInfo = {};
        compQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        compQueueInfo.queueFamilyIndex = m_uComputeQueueFamily;
        compQueueInfo.queueCount = 1;
        compQueueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(compQueueInfo);
    }

    // 璁惧鎵╁睍
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    if (m_bUseDynamicRendering)
    {
        deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    }

    // Vulkan 1.2/1.3 鐗规€?    VkPhysicalDeviceVulkan12Features features12 = {};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.timelineSemaphore = VK_TRUE;

    VkPhysicalDeviceVulkan13Features features13 = {};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = m_bUseDynamicRendering ? VK_TRUE : VK_FALSE;
    features13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceFeatures2 deviceFeatures = {};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.features.samplerAnisotropy = VK_TRUE;
    deviceFeatures.features.fillModeNonSolid = VK_TRUE;
    deviceFeatures.features.wideLines = VK_TRUE;
    deviceFeatures.features.multiDrawIndirect = VK_TRUE;
    deviceFeatures.features.textureCompressionBC = VK_TRUE;
    deviceFeatures.pNext = &features12;
    features12.pNext = &features13;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &deviceFeatures;

    VkResult result = vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vkCreateDevice failed: %d", result);
        return false;
    }

    // 鑾峰彇闃熷垪鍙ユ焺
    vkGetDeviceQueue(m_vkDevice, m_uGraphicsQueueFamily, 0, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, m_uComputeQueueFamily, 0, &m_vkComputeQueue);

    // 鍒涘缓 VMA Allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_vkPhysicalDevice;
    allocatorInfo.device = m_vkDevice;
    allocatorInfo.instance = m_vkInstance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    result = vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vmaCreateAllocator failed: %d", result);
        return false;
    }

    LOG_INFO("Vulkan device created successfully");
    return true;
}

// ============================================================================
// Init / Uninit
// ============================================================================
SResult VkContext::Init()
{
    m_bEnableDebug = m_pContext->EnableDebug();
    s_vulkan.Load();

    if (!CreateVulkanInstance())
        return ERR_SYSTEM_ERROR;

    if (!CreateVulkanDevice())
        return ERR_SYSTEM_ERROR;

    // 鍒涘缓鍛戒护姹?    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_uGraphicsQueueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_vkCommandPool) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create command pool");
        return ERR_SYSTEM_ERROR;
    }

    // 鍒涘缓 Pipeline Cache
    VkPipelineCacheCreateInfo cacheInfo = {};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(m_vkDevice, &cacheInfo, nullptr, &m_vkPipelineCache);

    // 鍒涘缓鍏ㄥ眬 descriptor pool
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
        { VK_DESCRIPTOR_TYPE_SAMPLER,                256 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          256 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          256 },
    };
    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    descriptorPoolInfo.pPoolSizes = poolSizes;
    descriptorPoolInfo.maxSets = 4096;
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(m_vkDevice, &descriptorPoolInfo, nullptr, &m_vkDescriptorPool) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create descriptor pool");
        return ERR_SYSTEM_ERROR;
    }

    // 鍒涘缓 per-frame 璧勬簮
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = m_uGraphicsQueueFamily;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_vkDevice, &cmdPoolInfo, nullptr, &m_perFrame[i].commandPool) != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create per-frame command pool %u", i);
            return ERR_SYSTEM_ERROR;
        }

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_perFrame[i].commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &m_perFrame[i].commandBuffer) != VK_SUCCESS)
        {
            LOG_ERROR("Failed to allocate per-frame command buffer %u", i);
            return ERR_SYSTEM_ERROR;
        }

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &m_perFrame[i].fence);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_perFrame[i].imageAcquiredSemaphore);
        vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_perFrame[i].renderCompleteSemaphore);
    }

    CheckCapabilitySetSupport();
    CreateCommonMesh();
    LOG_INFO("VkContext initialized successfully");
    return S_Success;
}

void VkContext::Uninit()
{
    // 绛夊緟璁惧绌洪棽
    if (m_vkDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_vkDevice);
    }

    // 娓呯悊 per-frame 璧勬簮
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (m_perFrame[i].fence != VK_NULL_HANDLE)
            vkDestroyFence(m_vkDevice, m_perFrame[i].fence, nullptr);
        if (m_perFrame[i].imageAcquiredSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(m_vkDevice, m_perFrame[i].imageAcquiredSemaphore, nullptr);
        if (m_perFrame[i].renderCompleteSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(m_vkDevice, m_perFrame[i].renderCompleteSemaphore, nullptr);
        if (m_perFrame[i].commandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_vkDevice, m_perFrame[i].commandPool, nullptr);
    }

    // 娓呯悊缂撳瓨
    m_Samplers.clear();
    m_RenderStates.clear();

    // 娓呯悊鍏叡璧勬簮
    if (m_vkDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
    if (m_vkPipelineCache != VK_NULL_HANDLE)
        vkDestroyPipelineCache(m_vkDevice, m_vkPipelineCache, nullptr);
    if (m_vkCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, nullptr);

    // 娓呯悊 VMA
    if (m_vmaAllocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(m_vmaAllocator);

    // 娓呯悊璁惧
    if (m_vkDevice != VK_NULL_HANDLE)
        vkDestroyDevice(m_vkDevice, nullptr);

    // 娓呯悊璋冭瘯
    if (m_vkDebugMessenger != VK_NULL_HANDLE && m_vkInstance != VK_NULL_HANDLE)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
        if (func)
            func(m_vkInstance, m_vkDebugMessenger, nullptr);
    }

    if (m_vkInstance != VK_NULL_HANDLE)
        vkDestroyInstance(m_vkInstance, nullptr);
}

// ============================================================================
// 鑳藉姏妫€娴?// ============================================================================
SResult VkContext::CheckCapabilitySetSupport()
{
    CapabilitySet& cap = m_CapabilitySet;

    // MSAA 閲囨牱鏁版敮鎸佹娴?    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &props);

    VkSampleCountFlags supportedSamples =
        props.limits.framebufferColorSampleCounts &
        props.limits.framebufferDepthSampleCounts;

    for (int i = 0; i <= CAP_MAX_TEXTURE_SAMPLE_COUNT; i++)
    {
        cap.TextureSampleCountSupport[i] = (supportedSamples & (1 << (i - 1))) != 0 || i <= 1;
    }

    // 鏈€澶?RenderTarget 鏁?    cap.maxRenderTargetCount = static_cast<uint8_t>(
        std::min(props.limits.maxColorAttachments, 8u));

    // 绾圭悊鏍煎紡鏀寔妫€娴?    for (uint32_t fmt = 0; fmt < to_underlying(PixelFormat::Num); fmt++)
    {
        VkFormat vkFmt = VkTranslate::PixelFormatToVkFormat(static_cast<PixelFormat>(fmt));
        if (vkFmt == VK_FORMAT_UNDEFINED)
            continue;

        VkFormatProperties fmtProps;
        vkGetPhysicalDeviceFormatProperties(m_vkPhysicalDevice, vkFmt, &fmtProps);

        cap.TextureSupport[fmt][to_underlying(TextureFormatSupportType::Filtering)] =
            (fmtProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) != 0;
        cap.TextureSupport[fmt][to_underlying(TextureFormatSupportType::Write)] =
            (fmtProps.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
        cap.TextureSupport[fmt][to_underlying(TextureFormatSupportType::RenderTarget)] =
            (fmtProps.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
        cap.TextureSupport[fmt][to_underlying(TextureFormatSupportType::MSAA)] =
            (fmtProps.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
    }

    return S_Success;
}

// ============================================================================
// AttachNativeWindow 鈥?鍒涘缓 VkWindow
// ============================================================================
SResult VkContext::AttachNativeWindow(std::string const& name, void* native_wnd)
{
    auto window = MakeUniquePtr<VkWindow>(m_pContext);
    SResult ret = window->Create(name, native_wnd, this);
    if (SEEK_CHECKFAILED(ret))
        return ret;

    m_pScreenRHIFrameBuffer = std::move(window);
    BindRHIFrameBuffer(m_pScreenRHIFrameBuffer);
    SetFinalRHIFrameBuffer(m_pScreenRHIFrameBuffer);

    return S_Success;
}

// ============================================================================
// 杈呭姪鍑芥暟 鈥?鍗曟鍛戒护鎻愪氦
// ============================================================================
VkCommandBuffer VkContext::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vkCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuf;
    vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &cmdBuf);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    return cmdBuf;
}

void VkContext::EndSingleTimeCommands(VkCommandBuffer cmdBuf)
{
    vkEndCommandBuffer(cmdBuf);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;

    vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vkGraphicsQueue);

    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &cmdBuf);
}

SResult VkContext::WaitForCommandBuffer(VkCommandBuffer cmdBuf)
{
    // 鍚屾绛夊緟锛堢畝鍖栧疄鐜帮紝鐢熶骇鐜搴旂敤 fence 寮傛绛夊緟锛?    vkEndCommandBuffer(cmdBuf);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vkGraphicsQueue);
    return S_Success;
}

// ============================================================================
// 娓叉煋寰幆
// ============================================================================
SResult VkContext::BeginFrame()
{
    PerFrameResources& frame = m_perFrame[m_uCurrentFrame];

    // 绛夊緟涓婁竴甯у畬鎴?    vkWaitForFences(m_vkDevice, 1, &frame.fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_vkDevice, 1, &frame.fence);

    // 鑾峰彇 swapchain image
    if (m_pCurrentVkFrameBuffer)
    {
        SResult ret = m_pCurrentVkFrameBuffer->AcquireNextImage(
            frame.imageAcquiredSemaphore, m_uCurrentSwapchainImageIndex);
        if (SEEK_CHECKFAILED(ret))
            return ret;
    }

    // 寮€濮嬪綍鍒跺懡浠ょ紦鍐?    vkResetCommandBuffer(frame.commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(frame.commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to begin command buffer");
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

SResult VkContext::EndFrame()
{
    PerFrameResources& frame = m_perFrame[m_uCurrentFrame];

    vkEndCommandBuffer(frame.commandBuffer);

    // 鎻愪氦
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAcquiredSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderCompleteSemaphore;

    if (vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, frame.fence) != VK_SUCCESS)
    {
        LOG_ERROR("Queue submit failed");
        return ERR_SYSTEM_ERROR;
    }

    // Present
    if (m_pCurrentVkFrameBuffer)
    {
        m_pCurrentVkFrameBuffer->Present(frame.renderCompleteSemaphore);
    }

    m_uCurrentFrame = (m_uCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return S_Success;
}

SResult VkContext::BeginRenderPass(const RenderPassInfo& renderPassInfo)
{
    ResetBindingState();

    if (renderPassInfo.fb)
    {
        BindRHIFrameBuffer(RHIFrameBufferPtr(renderPassInfo.fb, [](RHIFrameBuffer*){}));
    }

    RHIFrameBuffer* fb = GetCurRHIFrameBuffer().get();
    VkWindow* window = dynamic_cast<VkWindow*>(fb);
    if (!window)
    {
        LOG_ERROR("FrameBuffer is not a VkWindow");
        return ERR_SYSTEM_ERROR;
    }
    m_pCurrentVkFrameBuffer = window;

    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;

    if (m_bUseDynamicRendering)
    {
        // Vulkan 1.3 dynamic rendering
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = window->GetSwapchainImageViews()[m_uCurrentSwapchainImageIndex];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        float4 clearColor = m_pContext->GetClearColor();
        colorAttachment.clearValue.color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };

        VkRenderingAttachmentInfo depthAttachment = {};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = window->GetDepthImageView();
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = { {0, 0}, window->GetExtent() };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRendering(cmdBuf, &renderingInfo);
    }
    else
    {
        // 浼犵粺 render pass
        VkRenderPassBeginInfo rpBegin = {};
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.renderPass = window->GetVkRenderPass();
        rpBegin.framebuffer = window->GetVkFramebuffer();
        rpBegin.renderArea = { {0, 0}, window->GetExtent() };

        VkClearValue clearValues[2];
        float4 clearColor = m_pContext->GetClearColor();
        clearValues[0].color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
        clearValues[1].depthStencil = { 1.0f, 0 };

        rpBegin.clearValueCount = 2;
        rpBegin.pClearValues = clearValues;

        vkCmdBeginRenderPass(cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    }

    // 璁剧疆 viewport + scissor
    Viewport const& vp = fb->GetViewport();
    VkViewport viewport = {};
    viewport.x = vp.x;
    viewport.y = vp.y + vp.height;
    viewport.width = vp.width;
    viewport.height = -vp.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { (int32_t)vp.x, (int32_t)vp.y };
    scissor.extent = { (uint32_t)vp.width, (uint32_t)vp.height };
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    return S_Success;
}

SResult VkContext::Render(RHIProgram* program, RHIMeshPtr const& mesh)
{
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;
    VkProgram* vkProgram = static_cast<VkProgram*>(program);
    VkMesh* vkMesh = static_cast<VkMesh*>(mesh.get());

    if (!vkProgram || !vkMesh)
        return ERR_SYSTEM_ERROR;

    // 缁戝畾绠＄嚎
    VkPipeline pipeline = vkProgram->GetOrCreatePipeline(m_pCurrentVkFrameBuffer, vkMesh->GetVertexInputState(), m_vkPipelineCache);
    if (pipeline == VK_NULL_HANDLE)
        return ERR_SYSTEM_ERROR;

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // 缁戝畾椤剁偣缂撳啿
    VkDeviceSize offsets[] = { 0 };
    for (uint32_t i = 0; i < vkMesh->GetVertexBufferCount(); i++)
    {
        VkBuffer vb = vkMesh->GetVertexBuffer(i);
        if (vb != VK_NULL_HANDLE)
            vkCmdBindVertexBuffers(cmdBuf, i, 1, &vb, offsets);
    }

    // 缁戝畾绱㈠紩缂撳啿
    VkBuffer ib = vkMesh->GetIndexBuffer();
    VkIndexType ibType = vkMesh->GetIndexType();
    if (ib != VK_NULL_HANDLE)
        vkCmdBindIndexBuffer(cmdBuf, ib, 0, ibType);

    // Flush descriptor bindings collected during Commit phase
    FlushBindings(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);

    // 缁樺埗
    if (ib != VK_NULL_HANDLE)
    {
        vkCmdDrawIndexed(cmdBuf, vkMesh->GetIndexCount(), vkMesh->GetInstanceCount(), 0, 0, 0);
    }
    else
    {
        vkCmdDraw(cmdBuf, vkMesh->GetVertexCount(), vkMesh->GetInstanceCount(), 0, 0);
    }

    return S_Success;
}

SResult VkContext::EndRenderPass()
{
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;

    if (m_bUseDynamicRendering)
    {
        vkCmdEndRendering(cmdBuf);
    }
    else
    {
        vkCmdEndRenderPass(cmdBuf);
    }

    return S_Success;
}

void VkContext::BeginComputePass(const ComputePassInfo& computePassInfo)
{
    // 璁＄畻閫氶亾璧峰 鈥?Vulkan 涓€氬父鍦ㄥ悓涓€涓?render pass 澶栨墽琛?}

SResult VkContext::Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z)
{
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;
    VkProgram* vkProgram = static_cast<VkProgram*>(program);
    if (!vkProgram) return ERR_SYSTEM_ERROR;

    VkPipeline pipeline = vkProgram->GetOrCreateComputePipeline(m_vkPipelineCache);
    if (pipeline == VK_NULL_HANDLE)
        return ERR_SYSTEM_ERROR;

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkProgram->BindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE);
    vkCmdDispatch(cmdBuf, x, y, z);

    return S_Success;
}

SResult VkContext::DispatchIndirect(RHIProgram* program, RHIGpuBufferPtr indirectBuf)
{
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;
    VkProgram* vkProgram = static_cast<VkProgram*>(program);
    if (!vkProgram || !indirectBuf) return ERR_SYSTEM_ERROR;

    VkPipeline pipeline = vkProgram->GetOrCreateComputePipeline(m_vkPipelineCache);
    if (pipeline == VK_NULL_HANDLE) return ERR_SYSTEM_ERROR;

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkProgram->BindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE);

    VkBuffer buf = static_cast<VkGpuBuffer*>(indirectBuf.get())->GetVkBuffer();
    vkCmdDispatchIndirect(cmdBuf, buf, 0);
    return S_Success;
}

void VkContext::EndComputePass()
{
}

SResult VkContext::DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIGpuBufferPtr indirectBuf, MeshTopologyType type)
{
    // 闈炵储寮曢棿鎺ョ粯鍒?鈥?Vulkan 鐗堟湰
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;
    VkProgram* vkProgram = static_cast<VkProgram*>(program);
    if (!vkProgram || !indirectBuf) return ERR_SYSTEM_ERROR;

    VkPipeline pipeline = vkProgram->GetOrCreatePipeline(m_pCurrentVkFrameBuffer, nullptr, m_vkPipelineCache);
    if (pipeline == VK_NULL_HANDLE) return ERR_SYSTEM_ERROR;

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkProgram->BindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);

    VkBuffer buf = static_cast<VkGpuBuffer*>(indirectBuf.get())->GetVkBuffer();
    vkCmdDrawIndirect(cmdBuf, buf, 0, 1, 0);
    return S_Success;
}

SResult VkContext::DrawIndexedIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIMeshPtr const& mesh, RHIGpuBufferPtr indirectBuf, uint32_t argsOffset)
{
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;
    VkProgram* vkProgram = static_cast<VkProgram*>(program);
    VkMesh* vkMesh = static_cast<VkMesh*>(mesh.get());
    if (!vkProgram || !vkMesh || !indirectBuf) return ERR_NOT_IMPLEMENTED;

    VkPipeline pipeline = vkProgram->GetOrCreatePipeline(m_pCurrentVkFrameBuffer, vkMesh->GetVertexInputState(), m_vkPipelineCache);
    if (pipeline == VK_NULL_HANDLE) return ERR_SYSTEM_ERROR;

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDeviceSize offsets[] = { 0 };
    for (uint32_t i = 0; i < vkMesh->GetVertexBufferCount(); i++)
    {
        VkBuffer vb = vkMesh->GetVertexBuffer(i);
        if (vb != VK_NULL_HANDLE) vkCmdBindVertexBuffers(cmdBuf, i, 1, &vb, offsets);
    }

    VkBuffer ib = vkMesh->GetIndexBuffer();
    if (ib != VK_NULL_HANDLE)
        vkCmdBindIndexBuffer(cmdBuf, ib, 0, vkMesh->GetIndexType());

    vkProgram->BindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);

    VkBuffer buf = static_cast<VkGpuBuffer*>(indirectBuf.get())->GetVkBuffer();
    vkCmdDrawIndexedIndirect(cmdBuf, buf, argsOffset, 1, 0);
    return S_Success;
}

SResult VkContext::DrawInstanced(RHIProgram* program, RHIRenderStatePtr rs, MeshTopologyType type,
    uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;
    VkProgram* vkProgram = static_cast<VkProgram*>(program);
    if (!vkProgram) return ERR_SYSTEM_ERROR;

    VkPipeline pipeline = vkProgram->GetOrCreatePipeline(m_pCurrentVkFrameBuffer, nullptr, m_vkPipelineCache);
    if (pipeline == VK_NULL_HANDLE) return ERR_SYSTEM_ERROR;

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkProgram->BindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);
    vkCmdDraw(cmdBuf, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    return S_Success;
}

// ============================================================================
// 绾圭悊鎿嶄綔
// ============================================================================
SResult VkContext::CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst)
{
    return CopyTextureRegion(tex_src, tex_dst, 0, 0, 0);
}

SResult VkContext::CopyTextureRegion(RHITexturePtr tex_src, RHITexturePtr tex_dst, int32_t dst_x, int32_t dst_y, int32_t dst_z)
{
    // 绠€鍖栧疄鐜?鈥?閫氳繃 command buffer 杩涜 blit 鎴?copy
    VkCommandBuffer cmdBuf = m_perFrame[m_uCurrentFrame].commandBuffer;

    VkTexture2D* srcTex = static_cast<VkTexture2D*>(tex_src.get());
    VkTexture2D* dstTex = static_cast<VkTexture2D*>(tex_dst.get());
    if (!srcTex || !dstTex) return ERR_NOT_IMPLEMENTED;

    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.extent.width = static_cast<uint32_t>(srcTex->Descriptor().width);
    copyRegion.extent.height = static_cast<uint32_t>(srcTex->Descriptor().height);
    copyRegion.extent.depth = 1;
    copyRegion.dstOffset = { dst_x, dst_y, dst_z };

    vkCmdCopyImage(cmdBuf,
        srcTex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstTex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copyRegion);

    return S_Success;
}

// ============================================================================
// 璧勬簮缁戝畾 鈥?寤惰繜鏀堕泦妯″紡
// ============================================================================
void VkContext::BindRHIProgram(RHIProgram* program)
{
    m_pActiveProgram = static_cast<VkProgram*>(program);
}

void VkContext::BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIGpuBuffer* cbuffer, const char* name)
{
    if (!cbuffer || binding >= MAX_BINDINGS) return;
    uint32_t s = (uint32_t)stage;
    const VkGpuBuffer* vkBuf = static_cast<const VkGpuBuffer*>(cbuffer);
    m_Bindings[s][binding].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    m_Bindings[s][binding].buffer = vkBuf->GetVkBuffer();
    m_Bindings[s][binding].range = vkBuf->GetSize();
    m_Bindings[s][binding].cbuffer = cbuffer;
    m_BindingsUsed[s][binding] = true;
}

void VkContext::BindRHISrv(ShaderType stage, uint32_t binding, const RHIShaderResourceView* srv, const char* name)
{
    if (!srv || binding >= MAX_BINDINGS) return;
    uint32_t s = (uint32_t)stage;
    m_Bindings[s][binding].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    m_BindingsUsed[s][binding] = true;
}

void VkContext::BindRHIUav(ShaderType stage, uint32_t binding, const RHIUnorderedAccessView* uav, const char* name)
{
    if (!uav || binding >= MAX_BINDINGS) return;
    uint32_t s = (uint32_t)stage;
    m_Bindings[s][binding].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    m_BindingsUsed[s][binding] = true;
}

void VkContext::BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name)
{
    if (!texture || binding >= MAX_BINDINGS) return;
    uint32_t s = (uint32_t)stage;
    const VkTexture2D* vkTex = static_cast<const VkTexture2D*>(texture);
    m_Bindings[s][binding].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    m_Bindings[s][binding].imageView = vkTex->GetDefaultVkImageView();
    m_Bindings[s][binding].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_BindingsUsed[s][binding] = true;
}

void VkContext::BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name)
{
    if (!rw_texture || binding >= MAX_BINDINGS) return;
    uint32_t s = (uint32_t)stage;
    const VkTexture2D* vkTex = static_cast<const VkTexture2D*>(rw_texture);
    m_Bindings[s][binding].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    m_Bindings[s][binding].imageView = vkTex->GetDefaultVkImageView();
    m_Bindings[s][binding].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_BindingsUsed[s][binding] = true;
}

void VkContext::BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name)
{
    if (!sampler || binding >= MAX_BINDINGS) return;
    uint32_t s = (uint32_t)stage;
    const VkRHISampler* vkSamp = static_cast<const VkRHISampler*>(sampler);
    m_Bindings[s][binding].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    m_Bindings[s][binding].sampler = vkSamp->GetVkSampler();
    m_BindingsUsed[s][binding] = true;
}

void VkContext::ResetBindingState()
{
    memset(m_BindingsUsed, 0, sizeof(m_BindingsUsed));
    m_pActiveProgram = nullptr;
}

void VkContext::FlushBindings(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint)
{
    if (!m_pActiveProgram || !m_pActiveProgram->GetVkDescriptorSetLayout())
        return;

    VkDescriptorSetLayout layout = m_pActiveProgram->GetVkDescriptorSetLayout();
    VkPipelineLayout pipelineLayout = m_pActiveProgram->GetVkPipelineLayout();

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vkDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, &ds) != VK_SUCCESS)
        return;

    VkWriteDescriptorSet writes[MAX_BINDINGS * SHADER_STAGE_COUNT];
    VkDescriptorBufferInfo bufferInfos[MAX_BINDINGS * SHADER_STAGE_COUNT];
    VkDescriptorImageInfo imageInfos[MAX_BINDINGS * SHADER_STAGE_COUNT];
    uint32_t writeCount = 0;

    for (uint32_t s = 0; s < SHADER_STAGE_COUNT; s++)
    {
        for (uint32_t b = 0; b < MAX_BINDINGS; b++)
        {
            if (!m_BindingsUsed[s][b]) continue;

            VkWriteDescriptorSet& write = writes[writeCount];
            write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = ds;
            write.dstBinding = b;
            write.descriptorCount = 1;
            write.descriptorType = m_Bindings[s][b].type;

            if (m_Bindings[s][b].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                m_Bindings[s][b].type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
            {
                VkDescriptorBufferInfo& bi = bufferInfos[writeCount];
                bi.buffer = m_Bindings[s][b].buffer;
                bi.offset = m_Bindings[s][b].offset;
                bi.range = m_Bindings[s][b].range;
                write.pBufferInfo = &bi;
            }
            else
            {
                VkDescriptorImageInfo& ii = imageInfos[writeCount];
                ii.sampler = m_Bindings[s][b].sampler;
                ii.imageView = m_Bindings[s][b].imageView;
                ii.imageLayout = m_Bindings[s][b].imageLayout;
                write.pImageInfo = &ii;
            }
            writeCount++;
        }
    }

    if (writeCount > 0)
        vkUpdateDescriptorSets(m_vkDevice, writeCount, writes, 0, nullptr);

    vkCmdBindDescriptorSets(cmdBuf, bindPoint, pipelineLayout, 0, 1, &ds, 0, nullptr);
}

// ============================================================================
// MakeVulkanContext
// ============================================================================
extern "C" void MakeVulkanContext(Context* context, RHIContextPtrUnique& out)
{
    out = MakeUniquePtr<VkContext>(context);
}

SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!

