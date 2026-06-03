#include "kernel/context.h"
#include "rhi/vulkan/vulkan_query.h"
#include "rhi/vulkan/vulkan_context.h"
#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

VkTimeQuery::VkTimeQuery(Context* context)
    : RHITimeQuery(context)
{
}

VkTimeQuery::~VkTimeQuery()
{
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    if (m_vkQueryPool != VK_NULL_HANDLE && vkCtx->GetVkDevice() != VK_NULL_HANDLE)
    {
        vkDestroyQueryPool(vkCtx->GetVkDevice(), m_vkQueryPool, nullptr);
        m_vkQueryPool = VK_NULL_HANDLE;
    }
}

bool VkTimeQuery::Create(VkDevice device, VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    m_fTimestampPeriod = props.limits.timestampPeriod / 1e6f; // 纳秒 → 毫秒

    VkQueryPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    poolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    poolInfo.queryCount = 2;

    if (vkCreateQueryPool(device, &poolInfo, nullptr, &m_vkQueryPool) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create timestamp query pool");
        return false;
    }

    return true;
}

void VkTimeQuery::RecordBegin(VkCommandBuffer cmdBuf)
{
    vkCmdResetQueryPool(cmdBuf, m_vkQueryPool, 0, 2);
    vkCmdWriteTimestamp(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, m_vkQueryPool, 0);
}

void VkTimeQuery::RecordEnd(VkCommandBuffer cmdBuf)
{
    vkCmdWriteTimestamp(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, m_vkQueryPool, 1);
}

void VkTimeQuery::Resolve(VkDevice device)
{
    uint64_t timestamps[2];
    if (vkGetQueryPoolResults(device, m_vkQueryPool, 0, 2, sizeof(timestamps),
        timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT) == VK_SUCCESS)
    {
        m_fElapsedMs = (timestamps[1] - timestamps[0]) * m_fTimestampPeriod;
    }
}

SEEK_NAMESPACE_END

