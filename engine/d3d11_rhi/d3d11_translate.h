/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_translate.h
**
**      Brief                  : translate DVF enum to D3D11 enum
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "rendering/render_state.h"
#include "rendering/render_definition.h"
#include "rendering_d3d11/d3d11_predeclare.h"

DVF_NAMESPACE_BEGIN

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

    static PixelFormat TranslateFromPlatformFormat(DXGI_FORMAT format);
    static DXGI_FORMAT TranslateToPlatformFormat(PixelFormat format);
    static DXGI_FORMAT TranslateToPlatformFormat(VertexFormat format);
    static void TranslateResourceFlagsToD3D11Foramt(ResourceFlags flags, D3D11_USAGE& usage, UINT& cpu_access_flags);
};

DVF_NAMESPACE_END
