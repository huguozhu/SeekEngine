#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_render_state.h"
#include "rhi/vulkan/vulkan_predeclare.h"
#include "rhi/vulkan/vulkan_translate.h"
#include <vulkan/vulkan.h>
#include <unordered_map>

SEEK_NAMESPACE_BEGIN

class VkContext;
class VkWindow;

// ============================================================================
// PipelineKey — VkPipeline 缓存的键
// ============================================================================
struct PipelineKey
{
    VkRenderPass                     renderPass;
    uint32_t                         subpass;
    VkPipelineLayout                 pipelineLayout;
    VkPipelineVertexInputStateCreateInfo vertexInput;
    VkSampleCountFlagBits            samples;
    RenderStateDesc const*           desc;
    VkFormat                         colorFormat;
    VkFormat                         depthFormat;

    bool operator==(const PipelineKey& other) const;
};

struct PipelineKeyHash
{
    size_t operator()(const PipelineKey& key) const;
};

// ============================================================================
// VkRenderState — Vulkan 渲染状态（延迟创建 Pipeline）
// ============================================================================
class VkRenderState : public RHIRenderState
{
public:
    VkRenderState(Context* context, RenderStateDesc const& desc);
    ~VkRenderState() override;

    // 创建 Graphics Pipeline（不缓存，调用者负责管理生命周期）
    VkPipeline CreateGraphicsPipeline(
        VkDevice device,
        VkPipelineCache pipelineCache,
        VkRenderPass renderPass,
        uint32_t subpass,
        VkPipelineLayout pipelineLayout,
        const VkPipelineVertexInputStateCreateInfo* vertexInput,
        VkSampleCountFlagBits samples,
        VkFormat colorFormat,
        VkFormat depthFormat,
        bool bUseDynamicRendering);
};

using VkRenderStatePtr = std::shared_ptr<VkRenderState>;

// ============================================================================
// VkRHISampler — Vulkan 采样器
// ============================================================================
class VkRHISampler : public RHISampler
{
public:
    VkRHISampler(Context* context, SamplerDesc const& desc);
    ~VkRHISampler() override;

    VkSampler GetVkSampler() const { return m_vkSamplerState; }
    bool      CreateSampler(VkDevice device);

private:
    VkSampler m_vkSamplerState = VK_NULL_HANDLE;
};

using VkRHISamplerPtr = std::shared_ptr<VkRHISampler>;

SEEK_NAMESPACE_END


