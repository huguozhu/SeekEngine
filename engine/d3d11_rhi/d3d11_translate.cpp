#include "rendering_d3d11/d3d11_translate.h"

#include "util/log.h"

DVF_NAMESPACE_BEGIN

const char* D3D11Translate::TranslateVertexElementUsageSemantic(VertexElementUsage usage)
{
    static const char* POSITION     = "POSITION";
    static const char* TEXCOORD     = "TEXCOORD";
    static const char* NORMAL       = "NORMAL";
    static const char* COLOR        = "COLOR";
    static const char* BLENDWEIGHT  = "BLENDWEIGHT";
    static const char* BLENDINDEX   = "BLENDINDEX";
    static const char* TANGENT      = "TANGENT";
    static const char* BINORMAL     = "BINORMAL";
    static const char* INSTANCE     = "INSTANCE";
    static const char* UNKNOWN      = "UNKNOWN";
    const char* res = POSITION;
    switch (usage)
    {
    case VertexElementUsage::Position:      res = POSITION;     break;
    case VertexElementUsage::TexCoord:      res = TEXCOORD;     break;
    case VertexElementUsage::Normal:        res = NORMAL;       break;
    case VertexElementUsage::Color:         res = COLOR;        break;
    case VertexElementUsage::BlendWeight:   res = BLENDWEIGHT;  break;
    case VertexElementUsage::BlendIndex:    res = BLENDINDEX;   break;
    case VertexElementUsage::Tangent:       res = TANGENT;      break;
    case VertexElementUsage::Binormal:      res = BINORMAL;     break;
    case VertexElementUsage::Instance:      res = INSTANCE;     break;

    default:
        res = UNKNOWN;
    }
    return res;
}

D3D11_PRIMITIVE_TOPOLOGY D3D11Translate::TranslatePrimitiveTopology(MeshTopologyType type)
{
    D3D11_PRIMITIVE_TOPOLOGY res = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    switch (type)
    {
    case MeshTopologyType::Points:        res = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;       break;
    case MeshTopologyType::Lines:         res = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;        break;
    case MeshTopologyType::Line_Strip:    res = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;       break;
    case MeshTopologyType::Triangles:     res = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;    break;
    case MeshTopologyType::Triangle_Strip:res = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;   break;
    }
    return res;
}
D3D11_CULL_MODE D3D11Translate::TranslateCullMode(CullMode CullMode)
{
    D3D11_CULL_MODE res = D3D11_CULL_NONE;
    switch (CullMode)
    {
    case CullMode::Back:   res = D3D11_CULL_BACK;  break;
    case CullMode::Front:  res = D3D11_CULL_FRONT; break;
    default:        res = D3D11_CULL_NONE;  break;
    };
    return res;
}
D3D11_FILL_MODE D3D11Translate::TranslateFillMode(FillMode FillMode)
{
    D3D11_FILL_MODE res = D3D11_FILL_SOLID;
    switch (FillMode)
    {
    case FillMode::Wireframe:   res = D3D11_FILL_WIREFRAME; break;
    case FillMode::Solid:       res = D3D11_FILL_SOLID;     break;
    };
    return res;
}

D3D11_COMPARISON_FUNC D3D11Translate::TranslateCompareFunction(CompareFunction CompareFunction)
{
    D3D11_COMPARISON_FUNC res = D3D11_COMPARISON_ALWAYS;
    switch (CompareFunction)
    {
    case CompareFunction::Less:           res = D3D11_COMPARISON_LESS;            break;
    case CompareFunction::LessEqual:      res = D3D11_COMPARISON_LESS_EQUAL;      break;
    case CompareFunction::Greater:        res = D3D11_COMPARISON_GREATER;         break;
    case CompareFunction::GreaterEqual:   res = D3D11_COMPARISON_GREATER_EQUAL;   break;
    case CompareFunction::Equal:          res = D3D11_COMPARISON_EQUAL;           break;
    case CompareFunction::NotEqual:       res = D3D11_COMPARISON_NOT_EQUAL;       break;
    case CompareFunction::Never:          res = D3D11_COMPARISON_NEVER;           break;
    };
    return res;
}
D3D11_STENCIL_OP D3D11Translate::TranslateStencilOp(StencilOperation StencilOp)
{
    D3D11_STENCIL_OP res = D3D11_STENCIL_OP_KEEP;
    switch (StencilOp)
    {
    case StencilOperation::Zero:                res = D3D11_STENCIL_OP_ZERO;    break;
    case StencilOperation::Replace:             res = D3D11_STENCIL_OP_REPLACE; break;
    case StencilOperation::SaturatedIncrement:  res = D3D11_STENCIL_OP_INCR_SAT;break;
    case StencilOperation::SaturatedDecrement:  res = D3D11_STENCIL_OP_DECR_SAT;break;
    case StencilOperation::Invert:              res = D3D11_STENCIL_OP_INVERT;  break;
    case StencilOperation::Increment:           res = D3D11_STENCIL_OP_INCR;    break;
    case StencilOperation::Decrement:           res = D3D11_STENCIL_OP_DECR;    break;
    default:                                    res = D3D11_STENCIL_OP_KEEP;    break;
    };
    return res;
}
D3D11_BLEND_OP D3D11Translate::TranslateBlendOp(BlendOperation BlendOp)
{
    D3D11_BLEND_OP res = D3D11_BLEND_OP_ADD;
    switch (BlendOp)
    {
    case BlendOperation::Subtract:          res = D3D11_BLEND_OP_SUBTRACT;      break;
    case BlendOperation::Min:               res = D3D11_BLEND_OP_MIN;           break;
    case BlendOperation::Max:               res = D3D11_BLEND_OP_MAX;           break;
    case BlendOperation::ReverseSubtract:   res = D3D11_BLEND_OP_REV_SUBTRACT;  break;
    default:                                res = D3D11_BLEND_OP_ADD;           break;
    };
    return res;
}

D3D11_BLEND D3D11Translate::TranslateBlendFactor(BlendFactor BlendFactor)
{
    D3D11_BLEND res = D3D11_BLEND_ZERO;
    switch (BlendFactor)
    {
    case BlendFactor::Zero:           res = D3D11_BLEND_ZERO;             break;
    case BlendFactor::One:            res = D3D11_BLEND_ONE;              break;
    case BlendFactor::SrcColor:       res = D3D11_BLEND_SRC_COLOR;        break;
    case BlendFactor::InvSrcColor:    res = D3D11_BLEND_INV_SRC_COLOR;    break;
    case BlendFactor::SrcAlpha:       res = D3D11_BLEND_SRC_ALPHA;        break;
    case BlendFactor::InvSrcAlpha:    res = D3D11_BLEND_INV_SRC_ALPHA;    break;
    case BlendFactor::DstAlpha:       res = D3D11_BLEND_DEST_ALPHA;       break;
    case BlendFactor::InvDstAlpha:    res = D3D11_BLEND_INV_DEST_ALPHA;   break;
    case BlendFactor::DstColor:       res = D3D11_BLEND_DEST_COLOR;       break;
    case BlendFactor::InvDstColor:    res = D3D11_BLEND_INV_DEST_COLOR;   break;

    case BlendFactor::SrcAlphaSat:    res = D3D11_BLEND_SRC_ALPHA_SAT;    break;
    case BlendFactor::BlendFactor:    res = D3D11_BLEND_BLEND_FACTOR;     break;
    case BlendFactor::InvBlendFactor: res = D3D11_BLEND_INV_BLEND_FACTOR; break;

    case BlendFactor::Src1Color:      res = D3D11_BLEND_SRC1_COLOR;       break;
    case BlendFactor::InvSrc1Color:   res = D3D11_BLEND_INV_SRC1_COLOR;   break;
    case BlendFactor::Src1Alpha:      res = D3D11_BLEND_SRC1_ALPHA;       break;
    case BlendFactor::InvSrc1Alpha:   res = D3D11_BLEND_INV_SRC1_ALPHA;   break;

    default:                res = D3D11_BLEND_ZERO;             break;
    };
    return res;
}

D3D11_FILTER D3D11Translate::TranslateTexFilterOp(TexFilterOp filter)
{
    D3D11_FILTER res = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    switch (filter)
    {
    case TexFilterOp::Min_Mag_Mip_Point:                res = D3D11_FILTER_MIN_MAG_MIP_POINT;               break;
    case TexFilterOp::Min_Mag_Point_Mip_Linear:         res = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;        break;
    case TexFilterOp::Min_Point_Mag_Linear_Mip_Point:   res = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;  break;
    case TexFilterOp::Min_Point_Mag_Mip_Linear:         res = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;        break;
    case TexFilterOp::Min_Linear_Mag_Mip_Point:         res = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;        break;
    case TexFilterOp::Min_Linear_Mag_Point_Mip_Linear:  res = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; break;
    case TexFilterOp::Min_Mag_Linear_Mip_Point:         res = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;        break;
    case TexFilterOp::Min_Mag_Mip_Linear:               res = D3D11_FILTER_MIN_MAG_MIP_LINEAR;              break;
    case TexFilterOp::Anisotropic:                      res = D3D11_FILTER_ANISOTROPIC;                     break;
    default:                                            LOG_ERROR("Invalid texture filter operation");
    }
    return res;
}

D3D11_TEXTURE_ADDRESS_MODE D3D11Translate::TranslateAddressMode(TexAddressMode AddressMode)
{
    D3D11_TEXTURE_ADDRESS_MODE res = D3D11_TEXTURE_ADDRESS_WRAP;
    switch (AddressMode)
    {
    case TexAddressMode::Wrap:      res = D3D11_TEXTURE_ADDRESS_WRAP;   break;
    case TexAddressMode::Clamp:     res = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
    case TexAddressMode::Mirror:    res = D3D11_TEXTURE_ADDRESS_MIRROR; break;
    case TexAddressMode::Border:    res = D3D11_TEXTURE_ADDRESS_BORDER; break;
    default:                        LOG_ERROR("Invalid texture address mode");
    };
    return res;
}

PixelFormat D3D11Translate::TranslateFromPlatformFormat(DXGI_FORMAT format)
{
    PixelFormat res = PixelFormat::Unknown;
    switch (format)
    {
        // 8-bit
    case DXGI_FORMAT_R8_UNORM:              res = PixelFormat::R8_UNORM;            break;
    case DXGI_FORMAT_R8_UINT:               res = PixelFormat::R8_UINT;             break;

        // 16-bit
    case DXGI_FORMAT_R16_UINT:              res = PixelFormat::R16_UINT;            break;
    case DXGI_FORMAT_R16_SINT:              res = PixelFormat::R16_SINT;            break;
    case DXGI_FORMAT_R8G8_UINT:             res = PixelFormat::R8G8_UINT;           break;
    case DXGI_FORMAT_R8G8_SINT:             res = PixelFormat::R8G8_SINT;           break;

        // 32-bit
    case DXGI_FORMAT_R32_FLOAT:             res = PixelFormat::R32F;                break;
    case DXGI_FORMAT_R32_UINT:              res = PixelFormat::R32_UINT;            break;
    case DXGI_FORMAT_R32_SINT:              res = PixelFormat::R32_SINT;            break;
    case DXGI_FORMAT_R8G8B8A8_UNORM:        res = PixelFormat::R8G8B8A8_UNORM;      break;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   res = PixelFormat::R8G8B8A8_UNORM_SRGB; break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:        res = PixelFormat::B8G8R8A8_UNORM;      break;
    case DXGI_FORMAT_R8G8B8A8_UINT:         res = PixelFormat::R8G8B8A8_UINT;       break;
    case DXGI_FORMAT_R16G16_SNORM:          res = PixelFormat::R16G16_SNORM;        break;
        // 64-bit
    case DXGI_FORMAT_R32G32_FLOAT:          res = PixelFormat::R32G32F;             break;
    case DXGI_FORMAT_R16G16B16A16_UNORM:    res = PixelFormat::R16G16B16A16_UNORM;  break;
    case DXGI_FORMAT_R16G16B16A16_UINT:     res = PixelFormat::R16G16B16A16_UINT;   break;
    case DXGI_FORMAT_R16G16B16A16_SINT:     res = PixelFormat::R16G16B16A16_SINT;   break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:    res = PixelFormat::R16G16B16A16_FLOAT;  break;

        // 128-bit
    case DXGI_FORMAT_R32G32B32A32_FLOAT:    res = PixelFormat::R32G32B32A32_FLOAT;  break;

        // depth
    case DXGI_FORMAT_R16_TYPELESS:          res = PixelFormat::D16;                 break;
    case DXGI_FORMAT_R24G8_TYPELESS:        res = PixelFormat::D24S8;               break;
    case DXGI_FORMAT_R32_TYPELESS:          res = PixelFormat::D32F;                break;
    }
    return res;
}
DXGI_FORMAT D3D11Translate::TranslateToPlatformFormat(PixelFormat format)
{
    DXGI_FORMAT res = DXGI_FORMAT_UNKNOWN;
    switch (format)
    {
        // 8-bit
    case PixelFormat::R8_UNORM:             res = DXGI_FORMAT_R8_UNORM;             break;
    case PixelFormat::R8_UINT:              res = DXGI_FORMAT_R8_UINT;              break;

        // 16-bit
    case PixelFormat::R16_UINT:             res = DXGI_FORMAT_R16_UINT;             break;
    case PixelFormat::R16_SINT:             res = DXGI_FORMAT_R16_SINT;             break;
    case PixelFormat::R8G8_UINT:            res = DXGI_FORMAT_R8G8_UINT;            break;
    case PixelFormat::R8G8_SINT:            res = DXGI_FORMAT_R8G8_SINT;            break;
    case PixelFormat::R8G8_UNORM:           res = DXGI_FORMAT_R8G8_UNORM;           break;

        // 32-bit
    case PixelFormat::R32F:                 res = DXGI_FORMAT_R32_FLOAT;            break;
    case PixelFormat::R32_UINT:             res = DXGI_FORMAT_R32_UINT;             break;
    case PixelFormat::R32_SINT:             res = DXGI_FORMAT_R32_SINT;             break;
    case PixelFormat::R8G8B8A8_UNORM:       res = DXGI_FORMAT_R8G8B8A8_UNORM;       break;
    case PixelFormat::R8G8B8A8_UNORM_SRGB:  res = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  break;
    case PixelFormat::B8G8R8A8_UNORM:       res = DXGI_FORMAT_B8G8R8A8_UNORM;       break;
    case PixelFormat::R8G8B8A8_UINT:        res = DXGI_FORMAT_R8G8B8A8_UINT;        break;
    case PixelFormat::R16G16_SNORM:         res = DXGI_FORMAT_R16G16_SNORM;         break;
    case PixelFormat::R8G8_B8G8_UNORM:      res = DXGI_FORMAT_R8G8_B8G8_UNORM;      break;
    case PixelFormat::G8R8_G8B8_UNORM:      res = DXGI_FORMAT_G8R8_G8B8_UNORM;      break;

        // 64-bit
    case PixelFormat::R32G32F:              res = DXGI_FORMAT_R32G32_FLOAT;         break;
    case PixelFormat::R16G16B16A16_UNORM:   res = DXGI_FORMAT_R16G16B16A16_UNORM;   break;
    case PixelFormat::R16G16B16A16_UINT:    res = DXGI_FORMAT_R16G16B16A16_UINT;    break;
    case PixelFormat::R16G16B16A16_SINT:    res = DXGI_FORMAT_R16G16B16A16_SINT;    break;
    case PixelFormat::R16G16B16A16_FLOAT:   res = DXGI_FORMAT_R16G16B16A16_FLOAT;   break;

        // 128-bit
    case PixelFormat::R32G32B32A32_FLOAT:   res = DXGI_FORMAT_R32G32B32A32_FLOAT;   break;

        // depth
    case PixelFormat::D16:                  res = DXGI_FORMAT_R16_TYPELESS;         break;
    case PixelFormat::D24S8:                res = DXGI_FORMAT_R24G8_TYPELESS;       break;
    case PixelFormat::D32F:                 res = DXGI_FORMAT_R32_TYPELESS;         break;
    }
    return res;
}

DXGI_FORMAT D3D11Translate::TranslateToPlatformFormat(VertexFormat format)
{
    switch (format)
    {
    case VertexFormat::Unknown:            return DXGI_FORMAT_UNKNOWN;
    case VertexFormat::UChar2:             return DXGI_FORMAT_R8G8_UINT;
    //case VertexFormat::UChar3:           return DXGI_FORMAT_R8G8B8A8_UINT;
    case VertexFormat::UChar4:             return DXGI_FORMAT_R8G8B8A8_UINT;
    case VertexFormat::Char2:              return DXGI_FORMAT_R8G8_SINT;
    //case VertexFormat::Char3:            return DXGI_FORMAT_R8G8B8A8_SINT;
    case VertexFormat::Char4:              return DXGI_FORMAT_R8G8B8A8_SINT;
    case VertexFormat::UChar2Normalized:   return DXGI_FORMAT_R8G8_UNORM;
    //case VertexFormat::UChar3Normalized: return DXGI_FORMAT_R8G8B8A8_SNORM;
    case VertexFormat::UChar4Normalized:   return DXGI_FORMAT_R8G8B8A8_SNORM;
    case VertexFormat::Char2Normalized:    return DXGI_FORMAT_R8G8_SNORM;
    //case VertexFormat::Char3Normalized:  return DXGI_FORMAT_R8G8B8A8_SNORM;
    case VertexFormat::Char4Normalized:    return DXGI_FORMAT_R8G8B8A8_SNORM;

    case VertexFormat::UShort2:            return DXGI_FORMAT_R16G16_UINT;
    //case VertexFormat::UShort3:          return DXGI_FORMAT_R16G16B16A16_UINT;
    case VertexFormat::UShort4:            return DXGI_FORMAT_R16G16B16A16_UINT;
    case VertexFormat::Short2:             return DXGI_FORMAT_R16G16_SINT;
    //case VertexFormat::Short3:           return DXGI_FORMAT_R16G16B16A16_SINT;
    case VertexFormat::Short4:             return DXGI_FORMAT_R16G16B16A16_SINT;
    case VertexFormat::UShort2Normalized:  return DXGI_FORMAT_R16G16_UNORM;
    //case VertexFormat::UShort3Normalized:return DXGI_FORMAT_R16G16B16A16_UNORM;
    case VertexFormat::UShort4Normalized:  return DXGI_FORMAT_R16G16B16A16_UNORM;
    case VertexFormat::Short2Normalized:   return DXGI_FORMAT_R16G16_SNORM;
    //case VertexFormat::Short3Normalized: return DXGI_FORMAT_R16G16B16A16_SNORM;
    case VertexFormat::Short4Normalized:   return DXGI_FORMAT_R16G16B16A16_SNORM;

    case VertexFormat::Half2:              return DXGI_FORMAT_R16G16_FLOAT;
    //case VertexFormat::Half3:              return DXGI_FORMAT_R16G16B16_FLOAT;
    case VertexFormat::Half4:              return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case VertexFormat::Float:              return DXGI_FORMAT_R32_FLOAT;
    case VertexFormat::Float2:             return DXGI_FORMAT_R32G32_FLOAT;
    case VertexFormat::Float3:             return DXGI_FORMAT_R32G32B32_FLOAT;
    case VertexFormat::Float4:             return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case VertexFormat::UInt:               return DXGI_FORMAT_R32_UINT;
    case VertexFormat::UInt2:              return DXGI_FORMAT_R32G32_UINT;
    case VertexFormat::UInt3:              return DXGI_FORMAT_R32G32B32_UINT;
    case VertexFormat::UInt4:              return DXGI_FORMAT_R32G32B32A32_UINT;
    case VertexFormat::Int:                return DXGI_FORMAT_R32_SINT;
    case VertexFormat::Int2:               return DXGI_FORMAT_R32G32_SINT;
    case VertexFormat::Int3:               return DXGI_FORMAT_R32G32B32_SINT;
    case VertexFormat::Int4:               return DXGI_FORMAT_R32G32B32A32_SINT;
    default:  return DXGI_FORMAT_UNKNOWN;
    }
}

void D3D11Translate::TranslateResourceFlagsToD3D11Foramt(ResourceFlags flags, D3D11_USAGE& usage, UINT& cpu_access_flags)
{
    bool cpu_write = flags & RESOURCE_FLAG_CPU_WRITE;
    bool gpu_write = false;
    if (flags & RESOURCE_FLAG_RENDER_TARGET)
        gpu_write = true;
    else if (flags & RESOURCE_FLAG_SHADER_WRITE)
        gpu_write = true;
    else if (flags & RESOURCE_FLAG_GPU_WRITE)
        gpu_write = true;

    if (flags & RESOURCE_FLAG_COPY_BACK)
        cpu_access_flags |= D3D11_CPU_ACCESS_READ;

    if (cpu_write && gpu_write)
    {
        usage = D3D11_USAGE_STAGING;
        cpu_access_flags |= D3D11_CPU_ACCESS_READ;
    }
    else if (!cpu_write && !gpu_write)
    {
        usage = D3D11_USAGE_IMMUTABLE;
    }
    else if (cpu_write)
    {
        usage = D3D11_USAGE_DYNAMIC;
        cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
    }
    else
    {
        usage = D3D11_USAGE_DEFAULT;
    }
}


DVF_NAMESPACE_END
