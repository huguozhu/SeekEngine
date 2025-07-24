#pragma once

#include "rhi/base/rhi_render_state.h"
#include "rhi/base/rhi_definition.h"
#include "rhi/d3d11/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

class D3DCommonTranslate
{
public:
    static PixelFormat TranslateFromPlatformFormat(DXGI_FORMAT format);
    static DXGI_FORMAT TranslateToPlatformFormat(PixelFormat format);
    static DXGI_FORMAT TranslateToPlatformFormat(VertexFormat format);
};

SEEK_NAMESPACE_END
