#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_context.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include "utils/dll_loader.h"

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator
#include "vk_mem_alloc.h"

SEEK_NAMESPACE_BEGIN

class VkWindow;
class VkProgram;

// ============================================================================
//
class VkContext : public RHIContext
{
public:
    VkContext(Context* context);
    virtual ~VkContext();

    //
    VkInstance       GetVkInstance()       const { return m_vkInstance; }
    VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; }
    VkDevice         GetVkDevice()         const { return m_vkDevice; }
    VkQueue          GetVkGraphicsQueue()  const { return m_vkGraphicsQueue; }
    VkQueue          GetVkComputeQueue()   const { return m_vkComputeQueue; }
    uint32_t         GetGraphicsQueueFamily() const { return m_uGraphicsQueueFamily; }
    VmaAllocator     GetVmaAllocator()     const { return m_vmaAllocator; }
    VkCommandPool    GetVkCommandPool()    const { return m_vkCommandPool; }
    VkDescriptorPool GetVkDescriptorPool() const { return m_vkDescriptorPool; }
    VkCommandBuffer  GetCurrentCommandBuffer() const { return m_perFrame[m_uCurrentFrame].commandBuffer; }
    bool             UseDynamicRendering()   const { return m_bUseDynamicRendering; }
    uint32_t         GetCurrentSwapchainImageIndex() const { return m_uCurrentSwapchainImageIndex; }
    bool             IsFrameRecording()      const { return m_bFrameRecording; }

protected:
    //
    VkInstance              m_vkInstance = VK_NULL_HANDLE;
    VkPhysicalDevice        m_vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice                m_vkDevice = VK_NULL_HANDLE;
    VkQueue                 m_vkGraphicsQueue = VK_NULL_HANDLE;
    VkQueue                 m_vkComputeQueue = VK_NULL_HANDLE;
    uint32_t                m_uGraphicsQueueFamily = 0;
    uint32_t                m_uComputeQueueFamily = 0;
    uint32_t                m_uPresentQueueFamily = 0;
    VmaAllocator            m_vmaAllocator = VK_NULL_HANDLE;
    VkCommandPool           m_vkCommandPool = VK_NULL_HANDLE;
    VkPipelineCache         m_vkPipelineCache = VK_NULL_HANDLE;
    VkDescriptorPool        m_vkDescriptorPool = VK_NULL_HANDLE;

    VkWindow*               m_pCurrentVkFrameBuffer = nullptr;
    bool                     m_bFrameRecording = false;  // BeginFrame 后为 true，EndFrame 后为 false

    VkDebugUtilsMessengerEXT m_vkDebugMessenger = VK_NULL_HANDLE;
    bool                     m_bEnableDebug = false;

    bool m_bUseDynamicRendering = false;

// RHI 工厂方法
public:
    SResult             Init() override;
    void                Uninit() override;
    SResult             CheckCapabilitySetSupport() override;
    SResult             AttachNativeWindow(std::string const& name, void* native_wnd = nullptr) override;
    RHIMeshPtr          CreateMesh() override;
    RHIShaderPtr        CreateShader(ShaderType type, std::string const& name, std::string const& entry_func_name, std::string const& code) override;

    RHITexturePtr       CreateTexture2D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas = {}) override;
    RHITexturePtr       CreateTexture3D(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_datas = {}) override;
    RHITexturePtr       CreateTextureCube(const RHITexture::Desc& tex_desc, std::span<BitmapBufferPtr> init_data = {}) override;

    RHIGpuBufferPtr  CreateGpuBuffer(uint32_t size, ResourceFlags flags, uint32_t structure_stride, RHIGpuBufferData* pData = nullptr) override;
    RHIGpuBufferPtr  CreateConstantBuffer(uint32_t size, ResourceFlags flags, RHIGpuBufferData* pData = nullptr) override;
    RHIGpuBufferPtr  CreateVertexBuffer(uint32_t size, RHIGpuBufferData* pData) override;
    RHIGpuBufferPtr  CreateIndexBuffer(uint32_t size, RHIGpuBufferData* pData) override;

    RHIShaderResourceViewPtr CreateBufferSrv(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems) override;
    RHIUnorderedAccessViewPtr CreateBufferUav(RHIGpuBufferPtr const& buffer, PixelFormat format, uint32_t first_elem, uint32_t num_elems) override;

    RHIRenderTargetViewPtr Create2DRenderTargetView(RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0) override;
    RHIRenderTargetViewPtr Create2DRenderTargetView(RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level) override;
    RHIRenderTargetViewPtr Create3DRenderTargetView(RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level) override;
    RHIDepthStencilViewPtr Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0) override;
    RHIDepthStencilViewPtr Create2DDepthStencilView(RHITexturePtr const& tex_2d, uint32_t array_index, CubeFaceType face, uint32_t mip_level) override;

    RHIFrameBufferPtr   CreateRHIFrameBuffer() override;
    RHIProgramPtr       CreateRHIProgram() override;
    RHITimeQueryPtr     CreateRHITimeQuery() override;
    RHIFencePtr         CreateFence() override;

    //
    SResult             BeginFrame() override;
    SResult             EndFrame() override;

    SResult             BeginRenderPass(const RenderPassInfo& renderPassInfo) override;
    SResult             Render(RHIProgram* program, RHIMeshPtr const& mesh) override;
    SResult             EndRenderPass() override;

    void                BeginComputePass(const ComputePassInfo& computePassInfo) override;
    SResult             Dispatch(RHIProgram* program, uint32_t x, uint32_t y, uint32_t z) override;
    SResult             DispatchIndirect(RHIProgram* program, RHIGpuBufferPtr indirectBuf) override;
    SResult             DrawIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIGpuBufferPtr indirectBuf, MeshTopologyType type) override;
    SResult             DrawIndexedIndirect(RHIProgram* program, RHIRenderStatePtr rs, RHIMeshPtr const& mesh, RHIGpuBufferPtr indirectBuf, uint32_t argsOffset = 0) override;
    SResult             DrawInstanced(RHIProgram* program, RHIRenderStatePtr rs, MeshTopologyType type, uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) override;
    void                EndComputePass() override;

    //
    SResult             SyncTexture(RHITexturePtr tex) override { return S_Success; }
    SResult             CopyTexture(RHITexturePtr tex_src, RHITexturePtr tex_dst) override;
    SResult             CopyTextureRegion(RHITexturePtr tex_src, RHITexturePtr tex_dst, int32_t dst_x = 0, int32_t dst_y = 0, int32_t dst_z = 0) override;

    void                BeginCapture() override {}
    void                EndCapture() override {}

    //
    void                BindConstantBuffer(ShaderType stage, uint32_t binding, const RHIGpuBuffer* cbuffer, const char* name) override;
    void                BindRHISrv(ShaderType stage, uint32_t binding, const RHIShaderResourceView* srv, const char* name) override;
    void                BindRHIUav(ShaderType stage, uint32_t binding, const RHIUnorderedAccessView* uav, const char* name) override;
    void                BindTexture(ShaderType stage, uint32_t binding, const RHITexture* texture, const char* name) override;
    void                BindRWTexture(ShaderType stage, uint32_t binding, const RHITexture* rw_texture, const char* name) override;
    void                BindSampler(ShaderType stage, uint32_t binding, const RHISampler* sampler, const char* name) override;

    void                BindRHIProgram(RHIProgram* program) override;

    // 辅助方法（public 供 VkGpuBuffer/VkTexture 等调用）
    VkCommandBuffer     BeginSingleTimeCommands();
    void                EndSingleTimeCommands(VkCommandBuffer cmdBuf);
    // 按 swapchain image 数量创建 render complete 信号量（VkWindow 创建 swapchain 后调用）
    void                CreatePerImageRenderCompleteSemaphores(uint32_t imageCount);

protected:
    friend class Context;
    RHIRenderStatePtr   CreateRenderState(RenderStateDesc const& desc) override;
    RHISamplerPtr       CreateSampler(SamplerDesc const& desc) override;

    bool                CreateVulkanInstance();
    bool                CreateVulkanDevice();
    void                SetupValidationLayers();
    SResult             WaitForCommandBuffer(VkCommandBuffer cmdBuf);

    //
    struct PerFrameResources
    {
        VkCommandPool       commandPool = VK_NULL_HANDLE;
        VkCommandBuffer     commandBuffer = VK_NULL_HANDLE;
        VkFence             fence = VK_NULL_HANDLE;      //
        VkSemaphore         imageAcquiredSemaphore = VK_NULL_HANDLE;
        // renderCompleteSemaphore 改为按 swapchain image 索引分配（见 m_vRenderCompleteSemaphores）
    };
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    PerFrameResources m_perFrame[MAX_FRAMES_IN_FLIGHT];
    uint32_t m_uCurrentFrame = 0;

    // 按 swapchain image 索引的信号量（避免不同 image 复用同一信号量导致 presentation engine 冲突）
    std::vector<VkSemaphore> m_vRenderCompleteSemaphores;

    //
    uint32_t m_uCurrentSwapchainImageIndex = 0;

    // Descriptor binding state for current draw call
    struct BindingEntry {
        VkDescriptorType type;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
        VkDeviceSize range = VK_WHOLE_SIZE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        const RHIGpuBuffer* cbuffer = nullptr;
    };
    static constexpr uint32_t MAX_BINDINGS = 16;
    BindingEntry m_Bindings[SHADER_STAGE_COUNT][MAX_BINDINGS] = {};
    bool m_BindingsUsed[SHADER_STAGE_COUNT][MAX_BINDINGS] = {};
    VkProgram* m_pActiveProgram = nullptr;

    void ResetBindingState();
    void FlushBindings(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint);
    uint32_t m_uDescriptorSetCounter = 0;
};

// ============================================================================
//
// ============================================================================
class VkWindow : public RHIFrameBuffer
{
public:
    VkWindow(Context* context);
    ~VkWindow() override;

    SResult Create(std::string const& name, void* native_wnd, VkContext* context);

    VkRenderPass     GetVkRenderPass()      const { return m_vkRenderPass; }
    VkRenderPass     GetImGuiOverlayRenderPass() const { return m_vkImGuiOverlayRenderPass; }
    VkFramebuffer    GetVkFramebuffer()     const;
    VkFormat         GetColorFormat()       const { return m_vkColorFormat; }
    VkFormat         GetDepthFormat()       const { return m_vkDepthFormat; }
    VkSampleCountFlagBits GetSampleCount()  const { return m_vkSampleCount; }
    uint32_t         GetSwapchainImageCount() const { return m_uSwapchainImageCount; }
    VkExtent2D       GetExtent()            const { return m_vkSwapchainExtent; }
    const std::vector<VkImage>&     GetSwapchainImages()    const { return m_vSwapchainImages; }
    const std::vector<VkImageView>& GetSwapchainImageViews() const { return m_vSwapchainImageViews; }
    VkImage          GetDepthImage()        const { return m_vkDepthImage; }
    VkImageView      GetDepthImageView()    const { return m_vkDepthImageView; }

    SResult          AcquireNextImage(VkSemaphore semaphore, uint32_t& outImageIndex);
    SResult          Present(VkSemaphore waitSemaphore);

protected:
    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult SwapBuffers() override;

private:
    SResult CreateSurface(void* native_wnd);
    SResult CreateSwapchain();
    SResult CreateRenderPass();
    SResult CreateImGuiOverlayRenderPass();  // ImGui 覆盖层专用（LOAD_OP_LOAD 保留场景内容）
    SResult CreateFramebuffers();
    SResult CreateDepthBuffer();
    void    DestroySwapchainResources();
    SResult RecreateSwapchain();

    VkContext*          m_pVkContext = nullptr;
    HWND                m_hWnd = nullptr;

    VkSurfaceKHR        m_vkSurface = VK_NULL_HANDLE;
    VkSwapchainKHR      m_vkSwapchain = VK_NULL_HANDLE;
    VkRenderPass        m_vkRenderPass = VK_NULL_HANDLE;
    VkRenderPass        m_vkImGuiOverlayRenderPass = VK_NULL_HANDLE;  // ImGui 覆盖层专用（LOAD_OP_LOAD）

    uint32_t            m_uSwapchainImageCount = 0;
    VkFormat            m_vkColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat            m_vkDepthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    VkSampleCountFlagBits m_vkSampleCount = VK_SAMPLE_COUNT_1_BIT;
    VkExtent2D          m_vkSwapchainExtent = { 0, 0 };

    //
    std::vector<VkImage>        m_vSwapchainImages;
    std::vector<VkImageView>    m_vSwapchainImageViews;
    std::vector<VkFramebuffer>  m_vSwapchainFramebuffers;
    std::vector<RHITexturePtr>  m_vSwapchainTextures;       // swapchain image 纹理封装
    std::vector<RHIRenderTargetViewPtr> m_vSwapchainRtvs;   // 每个 swapchain image 的 RTV

    //
    VkImage         m_vkDepthImage = VK_NULL_HANDLE;
    VmaAllocation   m_vmaDepthAllocation = VK_NULL_HANDLE;
    VkImageView     m_vkDepthImageView = VK_NULL_HANDLE;

    uint32_t m_uCurrentImageIndex = 0;
    uint32_t m_uCurrentFrameIndex = 0;
};

SEEK_NAMESPACE_END





