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
// VkProgram 鈥?Vulkan 鐫€鑹插櫒绋嬪簭锛圖escriptorSetLayout + PipelineLayout 绠＄悊锛?// ============================================================================
class VkProgram : public RHIProgram
{
public:
    VkProgram(Context* context);
    ~VkProgram() override;

    // 鍒涘缓 DescriptorSetLayout 鍜?PipelineLayout
    SResult Build(VkDevice device);

    // 鑾峰彇鎴栧垱寤?Graphics Pipeline
    VkPipeline GetOrCreatePipeline(VkWindow* window, const VkPipelineVertexInputStateCreateInfo* vertexInput, VkPipelineCache pipelineCache);
    VkPipeline GetOrCreatePipeline(VkWindow* window, const VkPipelineVertexInputStateCreateInfo* vertexInput, VkPipelineCache pipelineCache, const RenderStateDesc& renderStateDesc);
    VkPipeline GetOrCreateComputePipeline(VkPipelineCache pipelineCache);

    // 缁戝畾 Descriptor Sets
    void BindDescriptorSets(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint);

    VkPipelineLayout         GetVkPipelineLayout() const { return m_vkPipelineLayout; }
    VkDescriptorSetLayout    GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }
    const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_vkDescriptorSets; }

    // 鏇存柊 descriptor set锛堢敱 ShaderParamAssignHelper 璋冪敤锛?    void UploadDescriptorSets(VkDevice device, VkDescriptorPool pool);

protected:
    VkPipelineLayout        m_vkPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout   m_vkDescriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_vkDescriptorSets;

    // 鍚堝苟鎵€鏈?stage 鐨?descriptor bindings
    struct MergedBinding
    {
        uint32_t            binding;
        VkDescriptorType    descriptorType;
        uint32_t            count;
        VkShaderStageFlags  stageFlags;
    };
    std::vector<MergedBinding> m_MergedBindings;

    // Graphics pipeline 缂撳瓨
    std::unordered_map<uint64_t, VkPipeline> m_GraphicsPipelineCache;
    VkPipeline m_vkComputePipeline = VK_NULL_HANDLE;

    VkDescriptorSet m_vkCurrentDescriptorSet = VK_NULL_HANDLE;
};

using VkProgramPtr = std::shared_ptr<VkProgram>;

SEEK_NAMESPACE_END

