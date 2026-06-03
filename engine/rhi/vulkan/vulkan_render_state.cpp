#include "kernel/context.h"
#include "kernel/context.h"
#include "rhi/vulkan/vulkan_render_state.h"
#include "rhi/vulkan/vulkan_context.h"
#include <vulkan/vulkan.h>
#include "rhi/vulkan/vulkan_shader.h"
#include "rhi/vulkan/vulkan_program.h"
#include "utils/log.h"

#include <cstring>

SEEK_NAMESPACE_BEGIN

// ============================================================================
// PipelineKey
// ============================================================================
bool PipelineKey::operator==(const PipelineKey& other) const
{
    return renderPass == other.renderPass
        && subpass == other.subpass
        && pipelineLayout == other.pipelineLayout
        && samples == other.samples
        && desc == other.desc
        && colorFormat == other.colorFormat
        && depthFormat == other.depthFormat
        && vertexInput.vertexBindingDescriptionCount == other.vertexInput.vertexBindingDescriptionCount
        && vertexInput.vertexAttributeDescriptionCount == other.vertexInput.vertexAttributeDescriptionCount;
}

size_t PipelineKeyHash::operator()(const PipelineKey& key) const
{
    size_t h = 0;
    h ^= std::hash<uint64_t>()((uint64_t)key.renderPass);
    h ^= std::hash<uint64_t>()((uint64_t)key.pipelineLayout);
    h ^= std::hash<uint64_t>()((uint64_t)key.desc);
    h ^= std::hash<uint32_t>()(key.subpass);
    h ^= std::hash<uint32_t>()((uint32_t)key.samples);
    h ^= std::hash<uint32_t>()((uint32_t)key.colorFormat);
    h ^= std::hash<uint32_t>()((uint32_t)key.depthFormat);
    return h;
}

// ============================================================================
// VkRenderState
// ============================================================================
VkRenderState::VkRenderState(Context* context, RenderStateDesc const& desc)
    : RHIRenderState(context, desc)
{
}

VkRenderState::~VkRenderState()
{
}

VkPipeline VkRenderState::CreateGraphicsPipeline(
    VkDevice device,
    VkPipelineCache pipelineCache,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkPipelineLayout pipelineLayout,
    const VkPipelineVertexInputStateCreateInfo* vertexInput,
    VkSampleCountFlagBits samples,
    VkFormat colorFormat,
    VkFormat depthFormat,
    bool bUseDynamicRendering)
{
    // Rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = !m_stRenderStateDesc.rasterizer.bDepthClip;
    rasterizer.polygonMode = VkTranslate::FillModeToVkPolygonMode(m_stRenderStateDesc.rasterizer.eFillMode);
    rasterizer.cullMode = VkTranslate::CullModeToVkCullMode(m_stRenderStateDesc.rasterizer.eCullMode);
    rasterizer.frontFace = m_stRenderStateDesc.rasterizer.bFrontFaceCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = m_stRenderStateDesc.rasterizer.fLineWidth;

// (comment stripped)
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = m_stRenderStateDesc.depthStencil.bDepthEnable;
    depthStencil.depthWriteEnable = m_stRenderStateDesc.depthStencil.bDepthWriteMask;
    depthStencil.depthCompareOp = VkTranslate::CompareFuncToVkCompareOp(m_stRenderStateDesc.depthStencil.eDepthFunc);
    depthStencil.stencilTestEnable = m_stRenderStateDesc.depthStencil.bFrontStencilEnable;
    depthStencil.front.failOp = VkTranslate::StencilOpToVkStencilOp(m_stRenderStateDesc.depthStencil.eFrontStencilFail);
    depthStencil.front.passOp = VkTranslate::StencilOpToVkStencilOp(m_stRenderStateDesc.depthStencil.eFrontStencilPass);
    depthStencil.front.depthFailOp = VkTranslate::StencilOpToVkStencilOp(m_stRenderStateDesc.depthStencil.eFrontStencilDepthFail);
    depthStencil.front.compareOp = VkTranslate::CompareFuncToVkCompareOp(m_stRenderStateDesc.depthStencil.eFrontStencilFunction);
    depthStencil.front.compareMask = m_stRenderStateDesc.depthStencil.iFrontStencilReadMask;
    depthStencil.front.writeMask = m_stRenderStateDesc.depthStencil.iFrontStencilWriteMask;
    depthStencil.front.reference = m_stRenderStateDesc.depthStencil.iFrontStencilRef;
    depthStencil.back = depthStencil.front;

// (comment stripped)
    VkPipelineColorBlendAttachmentState blendAttachment = {};
    const auto& target = m_stRenderStateDesc.blend.stTargetBlend[0];
    blendAttachment.blendEnable = target.bBlendEnable;
    blendAttachment.srcColorBlendFactor = VkTranslate::BlendFactorToVkBlendFactor(target.eSrcBlendColor);
    blendAttachment.dstColorBlendFactor = VkTranslate::BlendFactorToVkBlendFactor(target.eDstBlendColor);
    blendAttachment.colorBlendOp = VkTranslate::BlendOpToVkBlendOp(target.eBlendOpColor);
    blendAttachment.srcAlphaBlendFactor = VkTranslate::BlendFactorToVkBlendFactor(target.eSrcBlendAlpha);
    blendAttachment.dstAlphaBlendFactor = VkTranslate::BlendFactorToVkBlendFactor(target.eDstBlendAlpha);
    blendAttachment.alphaBlendOp = VkTranslate::BlendOpToVkBlendOp(target.eBlendOpAlpha);
    blendAttachment.colorWriteMask = target.bColorWriteMask;

    VkPipelineColorBlendStateCreateInfo colorBlend = {};
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &blendAttachment;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(std::size(dynamicStates));
    dynamicState.pDynamicStates = dynamicStates;

    // Default vertex input
    VkPipelineVertexInputStateCreateInfo defaultVertexInput = {};
    defaultVertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (!vertexInput) vertexInput = &defaultVertexInput;

    // 输入装配
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Viewport（动态）
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // 多重采样
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = samples;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pVertexInputState = vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlend;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;

    if (bUseDynamicRendering)
    {
        // Vulkan 1.3 dynamic rendering — pipeline 不绑定 render pass
        VkPipelineRenderingCreateInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachmentFormats = &colorFormat;
        renderingInfo.depthAttachmentFormat = depthFormat;
        pipelineInfo.pNext = &renderingInfo;
    }

    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
    if (result != VK_SUCCESS)
    {
        LOG_ERROR("Failed to create graphics pipeline: %d", result);
        return VK_NULL_HANDLE;
    }

    return pipeline;
}

// ============================================================================
// VkRHISampler
// ============================================================================
VkRHISampler::VkRHISampler(Context* context, SamplerDesc const& desc)
    : RHISampler(context, desc)
{
}

VkRHISampler::~VkRHISampler()
{
    if (m_vkSamplerState != VK_NULL_HANDLE)
    {
        VkContext* vkCtx = static_cast<VkContext*>(&m_pContext->RHIContextInstance());
        vkDestroySampler(vkCtx->GetVkDevice(), m_vkSamplerState, nullptr);
        m_vkSamplerState = VK_NULL_HANDLE;
    }
}

bool VkRHISampler::CreateSampler(VkDevice device)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VkTranslate::TexFilterToVkFilter(m_stSamplerDesc.eFilterOp);
    samplerInfo.minFilter = VkTranslate::TexFilterToVkFilter(m_stSamplerDesc.eFilterOp);
    samplerInfo.addressModeU = VkTranslate::TexAddressToVkAddress(m_stSamplerDesc.eAddrModeU);
    samplerInfo.addressModeV = VkTranslate::TexAddressToVkAddress(m_stSamplerDesc.eAddrModeV);
    samplerInfo.addressModeW = VkTranslate::TexAddressToVkAddress(m_stSamplerDesc.eAddrModeW);
    samplerInfo.anisotropyEnable = (m_stSamplerDesc.eFilterOp == TexFilterOp::Anisotropic);
    samplerInfo.maxAnisotropy = (float)m_stSamplerDesc.iMaxAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerInfo.compareEnable = VkTranslate::IsSamplerCompareEnabled(m_stSamplerDesc.eCompareFun);
    samplerInfo.compareOp = VkTranslate::CompareFuncToVkCompareOp(m_stSamplerDesc.eCompareFun);
    samplerInfo.minLod = m_stSamplerDesc.iMinLod;
    samplerInfo.maxLod = m_stSamplerDesc.iMaxLod;
    samplerInfo.mipLodBias = m_stSamplerDesc.iMipMapLodBias;

    return vkCreateSampler(device, &samplerInfo, nullptr, &m_vkSamplerState) == VK_SUCCESS;
}

SEEK_NAMESPACE_END



