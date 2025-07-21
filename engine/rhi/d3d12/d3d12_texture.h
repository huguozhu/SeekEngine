#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/d3d12/d3d12_resource.h"

SEEK_NAMESPACE_BEGIN

class D3D12Texture : public RHITexture, public D3D12Resource
{
public:
    D3D12Texture(Context* context, const RHITexture::Desc& tex_desc);

protected:

protected:
    DXGI_FORMAT                     m_eDxgiFormat = DXGI_FORMAT_UNKNOWN;

};

SEEK_NAMESPACE_END
