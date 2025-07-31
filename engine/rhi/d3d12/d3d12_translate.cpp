#include "rhi/d3d12/d3d12_translate.h"

#include "utils/log.h"

SEEK_NAMESPACE_BEGIN

const char* D3D12Translate::TranslateVertexElementUsageSemantic(VertexElementUsage usage)
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

D3D12_PRIMITIVE_TOPOLOGY D3D12Translate::TranslatePrimitiveTopology(MeshTopologyType type)
{
    D3D12_PRIMITIVE_TOPOLOGY res = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    switch (type)
    {
    case MeshTopologyType::Points:        res = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;       break;
    case MeshTopologyType::Lines:         res = D3D_PRIMITIVE_TOPOLOGY_LINELIST;        break;
    case MeshTopologyType::Line_Strip:    res = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;       break;
    case MeshTopologyType::Triangles:     res = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;    break;
    case MeshTopologyType::Triangle_Strip:res = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;   break;
    }
    return res;
}
D3D12_CULL_MODE D3D12Translate::TranslateCullMode(CullMode CullMode)
{
    D3D12_CULL_MODE res = D3D12_CULL_MODE_NONE;
    switch (CullMode)
    {
    case CullMode::Back:   res = D3D12_CULL_MODE_BACK;  break;
    case CullMode::Front:  res = D3D12_CULL_MODE_FRONT; break;
    default:        res = D3D12_CULL_MODE_NONE;  break;
    };
    return res;
}
D3D12_FILL_MODE D3D12Translate::TranslateFillMode(FillMode FillMode)
{
    D3D12_FILL_MODE res = D3D12_FILL_MODE_SOLID;
    switch (FillMode)
    {
    case FillMode::Wireframe:   res = D3D12_FILL_MODE_WIREFRAME; break;
    case FillMode::Solid:       res = D3D12_FILL_MODE_SOLID;     break;
    };
    return res;
}

D3D12_COMPARISON_FUNC D3D12Translate::TranslateCompareFunction(CompareFunction CompareFunction)
{
    D3D12_COMPARISON_FUNC res = D3D12_COMPARISON_FUNC_ALWAYS;
    switch (CompareFunction)
    {
    case CompareFunction::Less:           res = D3D12_COMPARISON_FUNC_LESS;            break;
    case CompareFunction::LessEqual:      res = D3D12_COMPARISON_FUNC_LESS_EQUAL;      break;
    case CompareFunction::Greater:        res = D3D12_COMPARISON_FUNC_GREATER;         break;
    case CompareFunction::GreaterEqual:   res = D3D12_COMPARISON_FUNC_GREATER_EQUAL;   break;
    case CompareFunction::Equal:          res = D3D12_COMPARISON_FUNC_EQUAL;           break;
    case CompareFunction::NotEqual:       res = D3D12_COMPARISON_FUNC_NOT_EQUAL;       break;
    case CompareFunction::Never:          res = D3D12_COMPARISON_FUNC_NEVER;           break;
    };
    return res;
}
D3D12_STENCIL_OP D3D12Translate::TranslateStencilOp(StencilOperation StencilOp)
{
    D3D12_STENCIL_OP res = D3D12_STENCIL_OP_KEEP;
    switch (StencilOp)
    {
    case StencilOperation::Zero:                res = D3D12_STENCIL_OP_ZERO;    break;
    case StencilOperation::Replace:             res = D3D12_STENCIL_OP_REPLACE; break;
    case StencilOperation::SaturatedIncrement:  res = D3D12_STENCIL_OP_INCR_SAT;break;
    case StencilOperation::SaturatedDecrement:  res = D3D12_STENCIL_OP_DECR_SAT;break;
    case StencilOperation::Invert:              res = D3D12_STENCIL_OP_INVERT;  break;
    case StencilOperation::Increment:           res = D3D12_STENCIL_OP_INCR;    break;
    case StencilOperation::Decrement:           res = D3D12_STENCIL_OP_DECR;    break;
    default:                                    res = D3D12_STENCIL_OP_KEEP;    break;
    };
    return res;
}
D3D12_BLEND_OP D3D12Translate::TranslateBlendOp(BlendOperation BlendOp)
{
    D3D12_BLEND_OP res = D3D12_BLEND_OP_ADD;
    switch (BlendOp)
    {
    case BlendOperation::Subtract:          res = D3D12_BLEND_OP_SUBTRACT;      break;
    case BlendOperation::Min:               res = D3D12_BLEND_OP_MIN;           break;
    case BlendOperation::Max:               res = D3D12_BLEND_OP_MAX;           break;
    case BlendOperation::ReverseSubtract:   res = D3D12_BLEND_OP_REV_SUBTRACT;  break;
    default:                                res = D3D12_BLEND_OP_ADD;           break;
    };
    return res;
}

D3D12_BLEND D3D12Translate::TranslateBlendFactor(BlendFactor BlendFactor)
{
    D3D12_BLEND res = D3D12_BLEND_ZERO;
    switch (BlendFactor)
    {
    case BlendFactor::Zero:           res = D3D12_BLEND_ZERO;             break;
    case BlendFactor::One:            res = D3D12_BLEND_ONE;              break;
    case BlendFactor::SrcColor:       res = D3D12_BLEND_SRC_COLOR;        break;
    case BlendFactor::InvSrcColor:    res = D3D12_BLEND_INV_SRC_COLOR;    break;
    case BlendFactor::SrcAlpha:       res = D3D12_BLEND_SRC_ALPHA;        break;
    case BlendFactor::InvSrcAlpha:    res = D3D12_BLEND_INV_SRC_ALPHA;    break;
    case BlendFactor::DstAlpha:       res = D3D12_BLEND_DEST_ALPHA;       break;
    case BlendFactor::InvDstAlpha:    res = D3D12_BLEND_INV_DEST_ALPHA;   break;
    case BlendFactor::DstColor:       res = D3D12_BLEND_DEST_COLOR;       break;
    case BlendFactor::InvDstColor:    res = D3D12_BLEND_INV_DEST_COLOR;   break;

    case BlendFactor::SrcAlphaSat:    res = D3D12_BLEND_SRC_ALPHA_SAT;    break;
    case BlendFactor::BlendFactor:    res = D3D12_BLEND_BLEND_FACTOR;     break;
    case BlendFactor::InvBlendFactor: res = D3D12_BLEND_INV_BLEND_FACTOR; break;

    case BlendFactor::Src1Color:      res = D3D12_BLEND_SRC1_COLOR;       break;
    case BlendFactor::InvSrc1Color:   res = D3D12_BLEND_INV_SRC1_COLOR;   break;
    case BlendFactor::Src1Alpha:      res = D3D12_BLEND_SRC1_ALPHA;       break;
    case BlendFactor::InvSrc1Alpha:   res = D3D12_BLEND_INV_SRC1_ALPHA;   break;

    default:                res = D3D12_BLEND_ZERO;             break;
    };
    return res;
}
D3D12_FILTER D3D12Translate::TranslateTexFilterOp(TexFilterOp filter)
{
    D3D12_FILTER res = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    switch (filter)
    {
    case TexFilterOp::Min_Mag_Mip_Point:                res = D3D12_FILTER_MIN_MAG_MIP_POINT;               break;
    case TexFilterOp::Min_Mag_Point_Mip_Linear:         res = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;        break;
    case TexFilterOp::Min_Point_Mag_Linear_Mip_Point:   res = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;  break;
    case TexFilterOp::Min_Point_Mag_Mip_Linear:         res = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;        break;
    case TexFilterOp::Min_Linear_Mag_Mip_Point:         res = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;        break;
    case TexFilterOp::Min_Linear_Mag_Point_Mip_Linear:  res = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR; break;
    case TexFilterOp::Min_Mag_Linear_Mip_Point:         res = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;        break;
    case TexFilterOp::Min_Mag_Mip_Linear:               res = D3D12_FILTER_MIN_MAG_MIP_LINEAR;              break;
    case TexFilterOp::Anisotropic:                      res = D3D12_FILTER_ANISOTROPIC;                     break;
    default:                                            LOG_ERROR("Invalid texture filter operation");
    }
    return res;
}
D3D12_TEXTURE_ADDRESS_MODE D3D12Translate::TranslateAddressMode(TexAddressMode AddressMode)
{
    D3D12_TEXTURE_ADDRESS_MODE res = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    switch (AddressMode)
    {
    case TexAddressMode::Wrap:      res = D3D12_TEXTURE_ADDRESS_MODE_WRAP;   break;
    case TexAddressMode::Clamp:     res = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;  break;
    case TexAddressMode::Mirror:    res = D3D12_TEXTURE_ADDRESS_MODE_MIRROR; break;
    case TexAddressMode::Border:    res = D3D12_TEXTURE_ADDRESS_MODE_BORDER; break;
    default:                        LOG_ERROR("Invalid texture address mode");
    };
    return res;
}

SEEK_NAMESPACE_END
