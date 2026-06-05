#include "rhi/vulkan/vulkan_translate.h"

SEEK_NAMESPACE_BEGIN

VkFormat VkTranslate::PixelFormatToVkFormat(PixelFormat fmt)
{
    switch (fmt)
    {
    case PixelFormat::R8_UNORM:           return VK_FORMAT_R8_UNORM;
    case PixelFormat::R8_UINT:            return VK_FORMAT_R8_UINT;
    case PixelFormat::R16_UINT:           return VK_FORMAT_R16_UINT;
    case PixelFormat::R16_SINT:           return VK_FORMAT_R16_SINT;
    case PixelFormat::R8G8_UINT:          return VK_FORMAT_R8G8_UINT;
    case PixelFormat::R8G8_SINT:          return VK_FORMAT_R8G8_SINT;
    case PixelFormat::R8G8_UNORM:         return VK_FORMAT_R8G8_UNORM;
    case PixelFormat::R32F:               return VK_FORMAT_R32_SFLOAT;
    case PixelFormat::R32_UINT:           return VK_FORMAT_R32_UINT;
    case PixelFormat::R32_SINT:           return VK_FORMAT_R32_SINT;
    case PixelFormat::R8G8B8A8_UNORM:     return VK_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::R8G8B8A8_UNORM_SRGB:return VK_FORMAT_R8G8B8A8_SRGB;
    case PixelFormat::B8G8R8A8_UNORM:     return VK_FORMAT_B8G8R8A8_UNORM;
    case PixelFormat::B8G8R8A8_UNORM_SRGB:return VK_FORMAT_B8G8R8A8_SRGB;
    case PixelFormat::R8G8B8A8_UINT:      return VK_FORMAT_R8G8B8A8_UINT;
    case PixelFormat::R16G16_SNORM:       return VK_FORMAT_R16G16_SNORM;
    case PixelFormat::R32G32F:            return VK_FORMAT_R32G32_SFLOAT;
    case PixelFormat::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
    case PixelFormat::R16G16B16A16_UINT:  return VK_FORMAT_R16G16B16A16_UINT;
    case PixelFormat::R16G16B16A16_SINT:  return VK_FORMAT_R16G16B16A16_SINT;
    case PixelFormat::R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case PixelFormat::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case PixelFormat::D16:                return VK_FORMAT_D16_UNORM;
    case PixelFormat::D24S8:              return VK_FORMAT_D24_UNORM_S8_UINT;
    case PixelFormat::D32F:               return VK_FORMAT_D32_SFLOAT;
    default:                              return VK_FORMAT_UNDEFINED;
    }
}

VkFormat VkTranslate::VertexFormatToVkFormat(VertexFormat fmt)
{
    switch (fmt)
    {
    case VertexFormat::Float:     return VK_FORMAT_R32_SFLOAT;
    case VertexFormat::Float2:    return VK_FORMAT_R32G32_SFLOAT;
    case VertexFormat::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
    case VertexFormat::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
    case VertexFormat::Half2:     return VK_FORMAT_R16G16_SFLOAT;
    case VertexFormat::Half4:     return VK_FORMAT_R16G16B16A16_SFLOAT;
    case VertexFormat::Short2:    return VK_FORMAT_R16G16_SINT;
    case VertexFormat::Short4:    return VK_FORMAT_R16G16B16A16_SINT;
    case VertexFormat::Char4:     return VK_FORMAT_R8G8B8A8_SINT;
    case VertexFormat::Char4Normalized:    return VK_FORMAT_R8G8B8A8_UNORM;
    case VertexFormat::UChar4:    return VK_FORMAT_R8G8B8A8_UINT;
    case VertexFormat::UChar4Normalized:   return VK_FORMAT_R8G8B8A8_UNORM;
    case VertexFormat::Unknown:     return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    default:                      return VK_FORMAT_UNDEFINED;
    }
}

uint32_t VkTranslate::VertexFormatSize(VertexFormat fmt)
{
    switch (fmt)
    {
    case VertexFormat::Float:   return 4;
    case VertexFormat::Float2:  return 8;
    case VertexFormat::Float3:  return 12;
    case VertexFormat::Float4:  return 16;
    case VertexFormat::Half2:   return 4;
    case VertexFormat::Half4:   return 8;
    case VertexFormat::Short2:  return 4;
    case VertexFormat::Short4:  return 8;
    case VertexFormat::Char4:   return 4;
    case VertexFormat::Char4Normalized:  return 4;
    case VertexFormat::UChar4:  return 4;
    case VertexFormat::UChar4Normalized: return 4;
    case VertexFormat::Unknown:   return 4;
    default:                    return 0;
    }
}

VkPrimitiveTopology VkTranslate::MeshTopologyToVkTopology(MeshTopologyType type)
{
    switch (type)
    {
    case MeshTopologyType::Points:         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case MeshTopologyType::Lines:          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case MeshTopologyType::Line_Strip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case MeshTopologyType::Triangles:      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case MeshTopologyType::Triangle_Strip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    default:                               return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

VkCullModeFlags VkTranslate::CullModeToVkCullMode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None:  return VK_CULL_MODE_NONE;
    case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
    case CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
    default:              return VK_CULL_MODE_NONE;
    }
}

VkPolygonMode VkTranslate::FillModeToVkPolygonMode(FillMode mode)
{
    switch (mode)
    {
    case FillMode::Point:     return VK_POLYGON_MODE_POINT;
    case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
    case FillMode::Solid:     return VK_POLYGON_MODE_FILL;
    default:                  return VK_POLYGON_MODE_FILL;
    }
}

VkCompareOp VkTranslate::CompareFuncToVkCompareOp(CompareFunction func)
{
    switch (func)
    {
    case CompareFunction::Less:          return VK_COMPARE_OP_LESS;
    case CompareFunction::LessEqual:     return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareFunction::Greater:       return VK_COMPARE_OP_GREATER;
    case CompareFunction::GreaterEqual:  return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareFunction::Equal:         return VK_COMPARE_OP_EQUAL;
    case CompareFunction::NotEqual:      return VK_COMPARE_OP_NOT_EQUAL;
    case CompareFunction::Never:         return VK_COMPARE_OP_NEVER;
    case CompareFunction::Always:        return VK_COMPARE_OP_ALWAYS;
    default:                             return VK_COMPARE_OP_ALWAYS;
    }
}

VkStencilOp VkTranslate::StencilOpToVkStencilOp(StencilOperation op)
{
    switch (op)
    {
    case StencilOperation::Keep:                return VK_STENCIL_OP_KEEP;
    case StencilOperation::Zero:                return VK_STENCIL_OP_ZERO;
    case StencilOperation::Replace:             return VK_STENCIL_OP_REPLACE;
    case StencilOperation::Increment:           return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOperation::Decrement:           return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOperation::Invert:              return VK_STENCIL_OP_INVERT;
    case StencilOperation::SaturatedIncrement:  return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOperation::SaturatedDecrement:  return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    default:                                    return VK_STENCIL_OP_KEEP;
    }
}

VkBlendFactor VkTranslate::BlendFactorToVkBlendFactor(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:             return VK_BLEND_FACTOR_ZERO;
    case BlendFactor::One:              return VK_BLEND_FACTOR_ONE;
    case BlendFactor::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFactor::InvSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFactor::InvSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFactor::InvDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFactor::InvDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case BlendFactor::SrcAlphaSat:      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case BlendFactor::BlendFactor:      return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case BlendFactor::InvBlendFactor:   return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case BlendFactor::Src1Color:        return VK_BLEND_FACTOR_SRC1_COLOR;
    case BlendFactor::InvSrc1Color:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case BlendFactor::Src1Alpha:        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case BlendFactor::InvSrc1Alpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:                            return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp VkTranslate::BlendOpToVkBlendOp(BlendOperation op)
{
    switch (op)
    {
    case BlendOperation::Add:             return VK_BLEND_OP_ADD;
    case BlendOperation::Subtract:        return VK_BLEND_OP_SUBTRACT;
    case BlendOperation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOperation::Min:             return VK_BLEND_OP_MIN;
    case BlendOperation::Max:             return VK_BLEND_OP_MAX;
    default:                              return VK_BLEND_OP_ADD;
    }
}

VkFilter VkTranslate::TexFilterToVkFilter(TexFilterOp filterOp)
{
    switch (filterOp)
    {
    case TexFilterOp::Min_Mag_Mip_Point:
    case TexFilterOp::Min_Mag_Point_Mip_Linear:
    case TexFilterOp::Min_Point_Mag_Linear_Mip_Point:
    case TexFilterOp::Min_Point_Mag_Mip_Linear:
        return VK_FILTER_NEAREST;
    default:
        return VK_FILTER_LINEAR;
    }
}

VkSamplerAddressMode VkTranslate::TexAddressToVkAddress(TexAddressMode mode)
{
    switch (mode)
    {
    case TexAddressMode::Wrap:   return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TexAddressMode::Clamp:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TexAddressMode::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TexAddressMode::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:                     return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkSampleCountFlagBits VkTranslate::SampleCountToVkFlags(uint32_t sampleCount)
{
    switch (sampleCount)
    {
    case 1:  return VK_SAMPLE_COUNT_1_BIT;
    case 2:  return VK_SAMPLE_COUNT_2_BIT;
    case 4:  return VK_SAMPLE_COUNT_4_BIT;
    case 8:  return VK_SAMPLE_COUNT_8_BIT;
    case 16: return VK_SAMPLE_COUNT_16_BIT;
    default: return VK_SAMPLE_COUNT_1_BIT;
    }
}

VkImageUsageFlags VkTranslate::GetVkImageUsageFlags(ResourceFlags flags, PixelFormat format)
{
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (flags & RESOURCE_FLAG_GPU_READ)
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (flags & RESOURCE_FLAG_GPU_WRITE)
    {
        if (Formatutil::IsDepthFormat(format))
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (flags & RESOURCE_FLAG_UAV)
        usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    return usage;
}

VkBufferUsageFlags VkTranslate::GetVkBufferUsageFlags(ResourceFlags flags)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (flags & RESOURCE_FLAG_GPU_READ)
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (flags & RESOURCE_FLAG_UAV)
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (flags & RESOURCE_FLAG_GPU_STRUCTURED)
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (flags & RESOURCE_FLAG_DRAW_INDIRECT_ARGS)
        usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    return usage;
}

VkShaderStageFlagBits VkTranslate::ShaderTypeToVkStage(ShaderType type)
{
    switch (type)
    {
    case ShaderType::Vertex:   return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderType::Pixel:    return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderType::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderType::Hull:     return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderType::Domain:   return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderType::Compute:  return VK_SHADER_STAGE_COMPUTE_BIT;
    default:                   return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

VkImageAspectFlags VkTranslate::GetImageAspectFromVkFormat(VkFormat fmt)
{
    switch (fmt)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

SEEK_NAMESPACE_END





