#pragma once

#include "rhi/base/rhi_render_state.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d11/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

class D3D11Translate
{
public:
    static const char* TranslateVertexElementUsageSemantic(VertexElementUsage usage);
    static D3D11_PRIMITIVE_TOPOLOGY TranslatePrimitiveTopology(MeshTopologyType type);
    static D3D11_CULL_MODE TranslateCullMode(CullMode CullMode);
    static D3D11_FILL_MODE TranslateFillMode(FillMode FillMode);
    static D3D11_COMPARISON_FUNC TranslateCompareFunction(CompareFunction CompareFunction);
    static D3D11_STENCIL_OP TranslateStencilOp(StencilOperation StencilOp);
    static D3D11_BLEND_OP TranslateBlendOp(BlendOperation BlendOp);
    static D3D11_BLEND TranslateBlendFactor(BlendFactor BlendFactor);
    static D3D11_FILTER TranslateTexFilterOp(TexFilterOp filter);
    static D3D11_TEXTURE_ADDRESS_MODE TranslateAddressMode(TexAddressMode AddressMode);

    static void TranslateResourceFlagsToD3D11Foramt(ResourceFlags flags, D3D11_USAGE& usage, UINT& cpu_access_flags);
};

SEEK_NAMESPACE_END
