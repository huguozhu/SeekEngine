#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_gpu_buffer.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

SEEK_NAMESPACE_BEGIN

// Vulkan Buffer type enumeration
enum class GpuBufferType_Vk : uint32_t
{
    COMMON_BUFFER   = 0,
    VERTEX_BUFFER   = 1,
    INDEX_BUFFER    = 2,
    CONSTANT_BUFFER = 3,
};

class VkContext;

class VkGpuBuffer : public RHIGpuBuffer
{
public:
    VkGpuBuffer(Context* context, uint32_t size, ResourceFlags flags, GpuBufferType_Vk type, uint32_t structureStride);
    ~VkGpuBuffer() override;

    SResult Create(RHIGpuBufferData* pData = nullptr) override;
    SResult Update(RHIGpuBufferData* pData) override;
    SResult CopyBack(BufferPtr buffer, int start = 0, int length = -1) override;

    VkBuffer        GetVkBuffer()      const { return m_vkBuffer; }
    VmaAllocation   GetVmaAllocation() const { return m_vmaAllocation; }
    VkDeviceSize    GetVkBufferSize()  const { return m_vkBufferSize; }
    void*           GetMappedData()    const { return m_pMappedData; }

protected:
    VkBuffer         m_vkBuffer = VK_NULL_HANDLE;
    VmaAllocation    m_vmaAllocation = VK_NULL_HANDLE;
    VkDeviceSize     m_vkBufferSize = 0;
    void*            m_pMappedData = nullptr;
    bool             m_bIsMapped = false;
    GpuBufferType_Vk m_eType = GpuBufferType_Vk::COMMON_BUFFER;

    VkContext*   GetVkContext() const;
    VkDevice     GetVkDevice() const;
    VmaAllocator GetVmaAllocator() const;
};

using VkGpuBufferPtr = std::shared_ptr<VkGpuBuffer>;

class VkConstantBuffer : public VkGpuBuffer
{
public:
    VkConstantBuffer(Context* context, uint32_t size, ResourceFlags flags);
};

class VkVertexBuffer : public VkGpuBuffer
{
public:
    VkVertexBuffer(Context* context, uint32_t size);
};

class VkIndexBuffer : public VkGpuBuffer
{
public:
    VkIndexBuffer(Context* context, uint32_t size);
};

SEEK_NAMESPACE_END
