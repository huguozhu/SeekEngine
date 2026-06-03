#include "kernel/context.h"
#include "rhi/vulkan/vulkan_gpu_buffer.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_translate.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 73     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

VkContext* VkGpuBuffer::GetVkContext() const { return static_cast<VkContext*>(&m_pContext->RHIContextInstance()); }
VkDevice VkGpuBuffer::GetVkDevice() const { return GetVkContext()->GetVkDevice(); }
VmaAllocator VkGpuBuffer::GetVmaAllocator() const { return GetVkContext()->GetVmaAllocator(); }

// ============================================================================
// VkGpuBuffer
// ============================================================================
VkGpuBuffer::VkGpuBuffer(Context* context, uint32_t size, ResourceFlags flags, GpuBufferType_Vk type, uint32_t structureStride)
    : RHIGpuBuffer(context, size, flags, structureStride), m_eType(type)
{
}

VkGpuBuffer::~VkGpuBuffer()
{
    if (m_pMappedData && m_vmaAllocation != VK_NULL_HANDLE)
    {
        vmaUnmapMemory(GetVmaAllocator(), m_vmaAllocation);
        m_pMappedData = nullptr;
    }
    if (m_vkBuffer != VK_NULL_HANDLE && m_vmaAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(GetVmaAllocator(), m_vkBuffer, m_vmaAllocation);
        m_vkBuffer = VK_NULL_HANDLE;
        m_vmaAllocation = VK_NULL_HANDLE;
    }
}

SResult VkGpuBuffer::Create(RHIGpuBufferData* pData)
{
    VkBufferUsageFlags usage = VkTranslate::GetVkBufferUsageFlags(m_iFlags);
    VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    VmaAllocationCreateFlags allocFlags = 0;

    // 根据 buffer 类型调整使用方式
    switch (m_eType)
    {
    case GpuBufferType_Vk::COMMON_BUFFER:
        if (m_iFlags & RESOURCE_FLAG_CPU_WRITE)
            memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if (m_iFlags & RESOURCE_FLAG_CPU_READ)
            memUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    case GpuBufferType_Vk::VERTEX_BUFFER:
        usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case GpuBufferType_Vk::INDEX_BUFFER:
        usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case GpuBufferType_Vk::CONSTANT_BUFFER:
        usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        break;
    default:
        break;
    }

    if (m_iFlags & RESOURCE_FLAG_DRAW_INDIRECT_ARGS)
        usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_vkBufferSize > 0 ? m_vkBufferSize : m_iSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memUsage;
    allocInfo.flags = allocFlags;

    VkResult result = vmaCreateBuffer(GetVmaAllocator(), &bufferInfo, &allocInfo,
        &m_vkBuffer, &m_vmaAllocation, nullptr);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("vmaCreateBuffer failed: %d", result);
        return ERR_SYSTEM_ERROR;
    }

    m_vkBufferSize = m_iSize;

    // 如果需要 CPU 访问，映射内存
    if (memUsage == VMA_MEMORY_USAGE_CPU_TO_GPU || memUsage == VMA_MEMORY_USAGE_CPU_ONLY)
    {
        m_bIsMapped = true;
    }

    // 上传初始数据
    if (pData && pData->m_pData && pData->m_iDataSize > 0)
    {
        if (m_bIsMapped)
        {
            void* mapped;
            vmaMapMemory(GetVmaAllocator(), m_vmaAllocation, &mapped);
            memcpy(mapped, pData->m_pData, std::min((uint32_t)pData->m_iDataSize, m_iSize));
            vmaUnmapMemory(GetVmaAllocator(), m_vmaAllocation);
        }
        else
        {
            // 通过 staging buffer 上传
            VkBuffer stagingBuffer;
            VmaAllocation stagingAlloc;
            VmaAllocationCreateInfo stagingAllocInfo = {};
            stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

            VkBufferCreateInfo stagingInfo = {};
            stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingInfo.size = pData->m_iDataSize;
            stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            if (vmaCreateBuffer(GetVmaAllocator(), &stagingInfo, &stagingAllocInfo,
                &stagingBuffer, &stagingAlloc, nullptr) == VK_SUCCESS)
            {
                void* mapped;
                vmaMapMemory(GetVmaAllocator(), stagingAlloc, &mapped);
                memcpy(mapped, pData->m_pData, pData->m_iDataSize);
                vmaUnmapMemory(GetVmaAllocator(), stagingAlloc);

                VkContext* vkCtx = GetVkContext();
                VkCommandBuffer cmdBuf = vkCtx->BeginSingleTimeCommands();

                VkBufferCopy copyRegion = {};
                copyRegion.size = pData->m_iDataSize;
                vkCmdCopyBuffer(cmdBuf, stagingBuffer, m_vkBuffer, 1, &copyRegion);

                vkCtx->EndSingleTimeCommands(cmdBuf);

                vmaDestroyBuffer(GetVmaAllocator(), stagingBuffer, stagingAlloc);
            }
        }
    }

    return S_Success;
}

SResult VkGpuBuffer::Update(RHIGpuBufferData* pData)
{
    if (!pData || !pData->m_pData || pData->m_iDataSize == 0)
        return ERR_INVALID_ARG;

    if (m_bIsMapped && m_vmaAllocation != VK_NULL_HANDLE)
    {
        void* mapped;
        vmaMapMemory(GetVmaAllocator(), m_vmaAllocation, &mapped);
        memcpy(mapped, pData->m_pData, std::min((uint32_t)pData->m_iDataSize, m_iSize));
        vmaUnmapMemory(GetVmaAllocator(), m_vmaAllocation);
        return S_Success;
    }

    // 否则通过 staging buffer
    VkContext* vkCtx = GetVkContext();
    VkCommandBuffer cmdBuf = vkCtx->BeginSingleTimeCommands();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationCreateInfo stagingAllocInfo = {};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VkBufferCreateInfo stagingInfo = {};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = pData->m_iDataSize;
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    vmaCreateBuffer(GetVmaAllocator(), &stagingInfo, &stagingAllocInfo,
        &stagingBuffer, &stagingAlloc, nullptr);

    void* mapped;
    vmaMapMemory(GetVmaAllocator(), stagingAlloc, &mapped);
    memcpy(mapped, pData->m_pData, pData->m_iDataSize);
    vmaUnmapMemory(GetVmaAllocator(), stagingAlloc);

    VkBufferCopy copyRegion = {};
    copyRegion.size = pData->m_iDataSize;
    vkCmdCopyBuffer(cmdBuf, stagingBuffer, m_vkBuffer, 1, &copyRegion);

    vkCtx->EndSingleTimeCommands(cmdBuf);

    vmaDestroyBuffer(GetVmaAllocator(), stagingBuffer, stagingAlloc);

    return S_Success;
}

SResult VkGpuBuffer::CopyBack(BufferPtr buffer, int start, int length)
{
    // GPU → CPU readback（简化实现）
    return ERR_NOT_IMPLEMENTED;
}

// ============================================================================
// VkConstantBuffer
// ============================================================================
VkConstantBuffer::VkConstantBuffer(Context* context, uint32_t size, ResourceFlags flags)
    : VkGpuBuffer(context, size, flags, GpuBufferType_Vk::CONSTANT_BUFFER, 0)
{
    m_vkBufferSize = size;
}

// ============================================================================
// VkVertexBuffer
// ============================================================================
VkVertexBuffer::VkVertexBuffer(Context* context, uint32_t size)
    : VkGpuBuffer(context, size, RESOURCE_FLAG_NONE, GpuBufferType_Vk::VERTEX_BUFFER, 0)
{
    m_vkBufferSize = size;
}

// ============================================================================
// VkIndexBuffer
// ============================================================================
VkIndexBuffer::VkIndexBuffer(Context* context, uint32_t size)
    : VkGpuBuffer(context, size, RESOURCE_FLAG_NONE, GpuBufferType_Vk::INDEX_BUFFER, 0)
{
    m_vkBufferSize = size;
}

SEEK_NAMESPACE_END

