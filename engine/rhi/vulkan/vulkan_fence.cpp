#include "kernel/context.h"
#include "rhi/vulkan/vulkan_fence.h"
#include "rhi/vulkan/vulkan_context.h"
#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

VkFence::VkFence(Context* context)
    : RHIFence(context)
{
}

VkFence::~VkFence()
{
    if (m_vkTimelineSemaphore != VK_NULL_HANDLE && m_vkDevice != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(m_vkDevice, m_vkTimelineSemaphore, nullptr);
        m_vkTimelineSemaphore = VK_NULL_HANDLE;
    }
}

bool VkFence::Create(VkDevice device)
{
    m_vkDevice = device;

    VkSemaphoreTypeCreateInfo typeInfo = {};
    typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeInfo.initialValue = 0;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = &typeInfo;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_vkTimelineSemaphore) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create timeline semaphore");
        return false;
    }

    return true;
}

uint64_t VkFence::Signal()
{
    m_uCounter++;
    uint64_t signalValue = m_uCounter;

    VkTimelineSemaphoreSubmitInfo timelineInfo = {};
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.signalSemaphoreValueCount = 1;
    timelineInfo.pSignalSemaphoreValues = &signalValue;

    VkSemaphoreSignalInfo signalInfo = {};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    signalInfo.semaphore = m_vkTimelineSemaphore;
    signalInfo.value = signalValue;

    vkSignalSemaphore(m_vkDevice, &signalInfo);

    return signalValue;
}

void VkFence::Wait(uint64_t value)
{
    VkSemaphoreWaitInfo waitInfo = {};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &m_vkTimelineSemaphore;
    waitInfo.pValues = &value;

    vkWaitSemaphores(m_vkDevice, &waitInfo, UINT64_MAX);
}

bool VkFence::IsCompleted(uint64_t value)
{
    uint64_t currentValue;
    vkGetSemaphoreCounterValue(m_vkDevice, m_vkTimelineSemaphore, &currentValue);
    return currentValue >= value;
}

SEEK_NAMESPACE_END

