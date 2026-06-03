#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_query.h"
#include <vulkan/vulkan.h>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkTimeQuery — Vulkan GPU 时间查询
// ============================================================================
class VkTimeQuery : public RHITimeQuery
{
public:
    VkTimeQuery(Context* context);
    ~VkTimeQuery() override;

    double TimeElapsedInMS() override { return (double)m_fElapsedMs; }

    void Begin() override { m_bQueryStarted = true; }
    void End() override { m_bQueryEnded = true; }

    bool Create(VkDevice device, VkPhysicalDevice physicalDevice);
    void RecordBegin(VkCommandBuffer cmdBuf);
    void RecordEnd(VkCommandBuffer cmdBuf);
    void Resolve(VkDevice device);

private:
    VkQueryPool m_vkQueryPool = VK_NULL_HANDLE;
    float       m_fElapsedMs = 0.0f;
    float       m_fTimestampPeriod = 0.0f;
    bool        m_bQueryStarted = false;
    bool        m_bQueryEnded = false;
};

SEEK_NAMESPACE_END

