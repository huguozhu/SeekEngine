#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_program.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

SEEK_NAMESPACE_BEGIN

class VkWindow;
class VkRenderState;

// ============================================================================
// VkProgram — Vulkan 着色器程序（DescriptorSetLayout + PipelineLayout 管理）
// ============================================================================
class VkProgram : public RHIProgram
{
public:
    VkProgram(Context* context);
    ~VkProgram() override;

    // 创建 DescriptorSetLayout 和 PipelineLayout
    SResult Build(VkDevice device);

    // 获取或创建 Graphics Pipeline
    VkPipeline GetOrCreatePipeline(VkWindow* window, const VkPipelineVertexInputStateCreateInfo* vertexInput, VkPipelineCache pipelineCache);
    VkPipeline GetOrCreatePipeline(VkWindow* window, const VkPipelineVertexInputStateCreateInfo* vertexInput, VkPipelineCache pipelineCache, const RenderStateDesc& renderStateDesc);
    VkPipeline GetOrCreateComputePipeline(VkPipelineCache pipelineCache);

    // 绑定 Descriptor Sets
    void BindDescriptorSets(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint);

    VkPipelineLayout         GetVkPipelineLayout() const { return m_vkPipelineLayout; }
    VkDescriptorSetLayout    GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }
    const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_vkDescriptorSets; }

    // 更新 descriptor set（由 ShaderParamAssignHelper 调用）
    void UploadDescriptorSets(VkDevice device, VkDescriptorPool pool);

protected:
    VkPipelineLayout        m_vkPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout   m_vkDescriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_vkDescriptorSets;

    // 合并所有 stage 的 descriptor bindings
    struct MergedBinding
    {
        uint32_t            binding;
        VkDescriptorType    descriptorType;
        uint32_t            count;
        VkShaderStageFlags  stageFlags;
    };
    std::vector<MergedBinding> m_MergedBindings;

    // Graphics pipeline 缓存
    std::unordered_map<uint64_t, VkPipeline> m_GraphicsPipelineCache;
    VkPipeline m_vkComputePipeline = VK_NULL_HANDLE;

    VkDescriptorSet m_vkCurrentDescriptorSet = VK_NULL_HANDLE;
};

using VkProgramPtr = std::shared_ptr<VkProgram>;

SEEK_NAMESPACE_END

