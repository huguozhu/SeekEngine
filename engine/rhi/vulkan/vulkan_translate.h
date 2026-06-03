#pragma once

#include "kernel/kernel.h"
#include "rhi/base/format.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/base/rhi_render_state.h"
#include "rhi/base/rhi_shader.h"
#include <vulkan/vulkan.h>

SEEK_NAMESPACE_BEGIN

class VkTranslate
{
public:
    static VkFormat PixelFormatToVkFormat(PixelFormat fmt);
    static VkFormat VertexFormatToVkFormat(VertexFormat fmt);
    static uint32_t VertexFormatSize(VertexFormat fmt);
    static VkPrimitiveTopology MeshTopologyToVkTopology(MeshTopologyType type);
    static VkCullModeFlags      CullModeToVkCullMode(CullMode mode);
    static VkPolygonMode        FillModeToVkPolygonMode(FillMode mode);
    static VkCompareOp          CompareFuncToVkCompareOp(CompareFunction func);
    static VkStencilOp          StencilOpToVkStencilOp(StencilOperation op);
    static VkBlendFactor        BlendFactorToVkBlendFactor(BlendFactor factor);
    static VkBlendOp            BlendOpToVkBlendOp(BlendOperation op);
    static VkFilter             TexFilterToVkFilter(TexFilterOp filterOp);
    static VkSamplerAddressMode  TexAddressToVkAddress(TexAddressMode mode);
    static VkSampleCountFlagBits SampleCountToVkFlags(uint32_t sampleCount);
    static VkImageUsageFlags GetVkImageUsageFlags(ResourceFlags flags, PixelFormat format);
    static VkBufferUsageFlags GetVkBufferUsageFlags(ResourceFlags flags);
    static VkShaderStageFlagBits ShaderTypeToVkStage(ShaderType type);
    static bool IsSamplerCompareEnabled(CompareFunction func) { return func != CompareFunction::Never; }
    // 根据 VkFormat 返回正确的图像 aspect 标记（深度格式用 DEPTH_BIT，颜色格式用 COLOR_BIT）
    static VkImageAspectFlags GetImageAspectFromVkFormat(VkFormat fmt);
};

SEEK_NAMESPACE_END
