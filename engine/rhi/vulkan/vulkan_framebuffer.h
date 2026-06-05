#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>

SEEK_NAMESPACE_BEGIN

class VkContext;

// ============================================================================
// VkFrameBuffer — Vulkan 离屏 FrameBuffer（非窗口）
// ============================================================================
class VkFrameBuffer : public RHIFrameBuffer
{
public:
    VkFrameBuffer(Context* context);
    ~VkFrameBuffer() override;

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult SwapBuffers() override { return S_Success; }

    VkRenderPass  GetVkRenderPass()  const { return m_vkRenderPass; }
    VkFramebuffer GetVkFramebuffer() const { return m_vkFramebuffer; }
    VkFormat      GetColorFormat()   const { return m_vkColorFormat; }
    VkFormat      GetDepthFormat()   const { return m_vkDepthFormat; }
    VkSampleCountFlagBits GetSampleCount() const { return m_vkSampleCount; }
    uint32_t      GetWidth()         const { return m_uWidth; }
    uint32_t      GetHeight()        const { return m_uHeight; }

    // Build render pass and framebuffer from attached views
    SResult Build();

    void Destroy();

protected:
    VkRenderPass  m_vkRenderPass = VK_NULL_HANDLE;
    VkFramebuffer m_vkFramebuffer = VK_NULL_HANDLE;
    VkFormat      m_vkColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat      m_vkDepthFormat = VK_FORMAT_D32_SFLOAT;
    VkSampleCountFlagBits m_vkSampleCount = VK_SAMPLE_COUNT_1_BIT;
    uint32_t m_uWidth = 0;
    uint32_t m_uHeight = 0;
};

using VkFrameBufferPtr = std::shared_ptr<VkFrameBuffer>;

SEEK_NAMESPACE_END
