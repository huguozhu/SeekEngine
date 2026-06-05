#include "rhi/vulkan/vulkan_mesh.h"
#include "rhi/vulkan/vulkan_gpu_buffer.h"
#include "rhi/vulkan/vulkan_translate.h"

SEEK_NAMESPACE_BEGIN

VkMesh::VkMesh(Context* context)
    : RHIMesh(context)
{
    m_VertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
}

const VkPipelineVertexInputStateCreateInfo* VkMesh::GetVertexInputState()
{
    if (m_bVertexInputDirty)
        BuildVertexInputState();
    return &m_VertexInputState;
}

void VkMesh::BuildVertexInputState()
{
    m_BindingDescriptions.clear();
    m_AttributeDescriptions.clear();

    uint32_t location = 0;
    for (const auto& stream : m_vVertexStreams)
    {
        VkVertexInputBindingDescription binding = {};
        binding.binding = static_cast<uint32_t>(m_BindingDescriptions.size());
        binding.stride = stream.stride;
        binding.inputRate = stream.is_instance_stream ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        m_BindingDescriptions.push_back(binding);

        uint32_t offset = 0;
        for (const auto& layout : stream.layouts)
        {
            VkVertexInputAttributeDescription attr = {};
            attr.location = location++;
            attr.binding = binding.binding;
            attr.format = VkTranslate::VertexFormatToVkFormat(layout.format);
            attr.offset = offset;
            m_AttributeDescriptions.push_back(attr);
            offset += VkTranslate::VertexFormatSize(layout.format);
        }
    }

    m_VertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_BindingDescriptions.size());
    m_VertexInputState.pVertexBindingDescriptions = m_BindingDescriptions.data();
    m_VertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_AttributeDescriptions.size());
    m_VertexInputState.pVertexAttributeDescriptions = m_AttributeDescriptions.data();

    m_bVertexInputDirty = false;
}

uint32_t VkMesh::GetVertexBufferCount() const
{
    return static_cast<uint32_t>(m_vVertexStreams.size());
}

VkBuffer VkMesh::GetVertexBuffer(uint32_t index) const
{
    if (index >= m_vVertexStreams.size())
        return VK_NULL_HANDLE;

    VkGpuBuffer* buf = static_cast<VkGpuBuffer*>(m_vVertexStreams[index].render_buffer.get());
    return buf ? buf->GetVkBuffer() : VK_NULL_HANDLE;
}

VkBuffer VkMesh::GetIndexBuffer() const
{
    VkGpuBuffer* buf = static_cast<VkGpuBuffer*>(m_pIndexBuffer.get());
    return buf ? buf->GetVkBuffer() : VK_NULL_HANDLE;
}

VkIndexType VkMesh::GetIndexType() const
{
    return m_eIndexBufferType == IndexBufferType::UInt32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
}

uint32_t VkMesh::GetIndexCount() const
{
    if (m_pIndexBuffer && m_eIndexBufferType != IndexBufferType::Unknown)
    {
        uint32_t elemSize = (m_eIndexBufferType == IndexBufferType::UInt32) ? 4 : 2;
        return m_pIndexBuffer->GetSize() / elemSize;
    }
    return 0;
}

uint32_t VkMesh::GetVertexCount() const
{
    if (!m_vVertexStreams.empty())
    {
        uint32_t stride = m_vVertexStreams[0].stride;
        if (stride > 0 && m_vVertexStreams[0].render_buffer)
            return m_vVertexStreams[0].render_buffer->GetSize() / stride;
    }
    return 0;
}

uint32_t VkMesh::GetInstanceCount() const
{
    return m_uInstancCount > 0 ? m_uInstancCount : 1;
}

SEEK_NAMESPACE_END


