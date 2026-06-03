#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_fence.h"
#include <vulkan/vulkan.h>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkFence — Vulkan 围栏（使用 Timeline Semaphore）
// ============================================================================
class VkFence : public RHIFence
{
public:
    VkFence(Context* context);
    ~VkFence() override;

    uint64_t Signal() override;
    void     Wait(uint64_t value) override;
    bool     IsCompleted(uint64_t value) override;

    bool Create(VkDevice device);

private:
    VkDevice      m_vkDevice = VK_NULL_HANDLE;
    VkSemaphore   m_vkTimelineSemaphore = VK_NULL_HANDLE;
    uint64_t      m_uCounter = 0;
};

SEEK_NAMESPACE_END
