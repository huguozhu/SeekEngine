#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_mesh.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>
#include <vector>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// VkMesh — Vulkan 网格
// ============================================================================
class VkMesh : public RHIMesh
{
public:
    VkMesh(Context* context);
    ~VkMesh() override = default;

    // Get vertex input state for pipeline creation
    const VkPipelineVertexInputStateCreateInfo* GetVertexInputState();

    // 顶点/索引缓冲
    uint32_t     GetVertexBufferCount() const;
    VkBuffer     GetVertexBuffer(uint32_t index) const;
    VkBuffer     GetIndexBuffer() const;
    VkIndexType  GetIndexType() const;

    // 绘制参数
    uint32_t GetIndexCount() const;
    uint32_t GetVertexCount() const;
    uint32_t GetInstanceCount() const;

private:
    void BuildVertexInputState();

    VkPipelineVertexInputStateCreateInfo           m_VertexInputState = {};
    std::vector<VkVertexInputBindingDescription>    m_BindingDescriptions;
    std::vector<VkVertexInputAttributeDescription>  m_AttributeDescriptions;
    bool m_bVertexInputDirty = true;
};

using VkMeshPtr = std::shared_ptr<VkMesh>;

SEEK_NAMESPACE_END

