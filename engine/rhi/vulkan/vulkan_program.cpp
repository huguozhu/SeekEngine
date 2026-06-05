#include "kernel/context.h"
#include "rhi/vulkan/vulkan_program.h"
#include "rhi/vulkan/vulkan_context.h"
#include "rhi/vulkan/vulkan_shader.h"
#include "rhi/vulkan/vulkan_mesh.h"
#include "rhi/vulkan/vulkan_render_state.h"
#include "utils/log.h"

#define SEEK_MACRO_FILE_UID 78     //

SEEK_NAMESPACE_BEGIN

VkProgram::VkProgram(Context* context)
    : RHIProgram(context)
{
}

VkProgram::~VkProgram()
{
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    VkDevice device = vkCtx->GetVkDevice();

    if (m_vkPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, m_vkPipelineLayout, nullptr);
        m_vkPipelineLayout = VK_NULL_HANDLE;
    }
    if (m_vkDescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_vkDescriptorSetLayout, nullptr);
        m_vkDescriptorSetLayout = VK_NULL_HANDLE;
    }

    // 清理缓存的 pipelines
    for (auto& [key, pipeline] : m_GraphicsPipelineCache)
    {
        vkDestroyPipeline(device, pipeline, nullptr);
    }
    m_GraphicsPipelineCache.clear();

    if (m_vkComputePipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, m_vkComputePipeline, nullptr);
        m_vkComputePipeline = VK_NULL_HANDLE;
    }
}

SResult VkProgram::Build(VkDevice device)
{
    // 收集所有 stage 的 descriptor bindings，合并后创建 DescriptorSetLayout
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    // 从 VkShader 获取每个 stage 的 bindings
    for (int i = 0; i < static_cast<int>(ShaderType::Num); i++)
    {
        VkShader* shader = static_cast<VkShader*>(m_vShaders[i]);
        if (!shader) continue;

        for (const auto& binding : shader->GetDescriptorBindings())
        {
            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.binding = binding.binding;
            layoutBinding.descriptorType = binding.descriptorType;
            layoutBinding.descriptorCount = binding.count;
            layoutBinding.stageFlags = binding.stageFlags;
            bindings.push_back(layoutBinding);
        }
    }

    // 创建 DescriptorSetLayout
    if (!bindings.empty())
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_vkDescriptorSetLayout) != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create DescriptorSetLayout");
            return ERR_SYSTEM_ERROR;
        }
    }

    // 创建 PipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = m_vkDescriptorSetLayout ? 1 : 0;
    pipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout) != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create PipelineLayout");
        return ERR_SYSTEM_ERROR;
    }

    return S_Success;
}

VkPipeline VkProgram::GetOrCreatePipeline(VkWindow* window,
    const VkPipelineVertexInputStateCreateInfo* vertexInput, VkPipelineCache pipelineCache)
{
    return GetOrCreatePipeline(window, vertexInput, pipelineCache, RenderStateDesc::Default3D());
}

VkPipeline VkProgram::GetOrCreatePipeline(VkWindow* window,
    const VkPipelineVertexInputStateCreateInfo* vertexInput, VkPipelineCache pipelineCache,
    const RenderStateDesc& renderStateDesc)
{
    if (!window) return VK_NULL_HANDLE;

    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    VkDevice device = vkCtx->GetVkDevice();

    // 构建 pipeline key（用于缓存）
    uint64_t key = 0;
    if (vertexInput)
    {
        key ^= (uint64_t)vertexInput->vertexAttributeDescriptionCount << 32;
        key ^= (uint64_t)vertexInput->vertexBindingDescriptionCount;
    }
    key ^= (uint64_t)(uintptr_t)this;
    key ^= renderStateDesc.Hash();

    auto it = m_GraphicsPipelineCache.find(key);
    if (it != m_GraphicsPipelineCache.end())
        return it->second;

    // Ensure Build has been called
    if (m_vkPipelineLayout == VK_NULL_HANDLE)
        Build(device);

    VkRenderState tempRS(m_pContext, renderStateDesc);
    VkPipeline pipeline = tempRS.CreateGraphicsPipeline(
        device, pipelineCache,
        window->GetVkRenderPass(), 0,
        m_vkPipelineLayout,
        vertexInput,
        window->GetSampleCount(),
        window->GetColorFormat(),
        window->GetDepthFormat(),
        vkCtx->UseDynamicRendering());

    if (pipeline != VK_NULL_HANDLE)
        m_GraphicsPipelineCache[key] = pipeline;

    return pipeline;
}

VkPipeline VkProgram::GetOrCreateComputePipeline(VkPipelineCache pipelineCache)
{
    if (m_vkComputePipeline != VK_NULL_HANDLE)
        return m_vkComputePipeline;

    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
    VkDevice device = vkCtx->GetVkDevice();

    if (m_vkPipelineLayout == VK_NULL_HANDLE)
        Build(device);

    VkShader* computeShader = static_cast<VkShader*>(m_vShaders[static_cast<int>(ShaderType::Compute)]);
    if (!computeShader || computeShader->GetVkShaderModule() == VK_NULL_HANDLE)
    {
        LOG_ERROR("Compute shader not set for program");
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo stageInfo = {};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = computeShader->GetVkShaderModule();
    stageInfo.pName = computeShader->GetEntryPoint().c_str();

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = m_vkPipelineLayout;

    vkCreateComputePipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &m_vkComputePipeline);

    return m_vkComputePipeline;
}

void VkProgram::BindDescriptorSets(VkCommandBuffer cmdBuf, VkPipelineBindPoint bindPoint)
{
    // 简化实现：分配并绑定 descriptor set
    VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());

    if (m_vkDescriptorSetLayout == VK_NULL_HANDLE)
        return;

    // 分配新的 descriptor set
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vkCtx->GetVkDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_vkDescriptorSetLayout;

    VkDescriptorSet ds;
    if (vkAllocateDescriptorSets(vkCtx->GetVkDevice(), &allocInfo, &ds) != VK_SUCCESS)
    {
        LOG_WARNING("Failed to allocate descriptor set");
        return;
    }

    vkCmdBindDescriptorSets(cmdBuf, bindPoint, m_vkPipelineLayout, 0, 1, &ds, 0, nullptr);
}

SEEK_NAMESPACE_END




