#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/d3d12/d3d12_resource.h"
#include "rhi/d3d12/d3d12_gpu_memory_allocator.h"
#include "rhi/d3d12/d3d12_render_view.h"

SEEK_NAMESPACE_BEGIN

class D3D12Texture : public RHITexture, public D3D12Resource
{
public:
    D3D12Texture(Context* context, const RHITexture::Desc& tex_desc);
    virtual ~D3D12Texture() override;

    //virtual D3DRtv* GetD3DRtv() {}
    //virtual D3DDsv* GetD3DDsv() {}
    //virtual ID3D11ShaderResourceView* GetD3DSrv();
    //virtual ID3D11UnorderedAccessView* GetD3DUav();


protected:
    DXGI_FORMAT             m_eDxgiFormat = DXGI_FORMAT_UNKNOWN;
    D3D12GpuMemoryBlock     m_MappedMemoryBlock;

    uint32_t m_iMappedXOffset;
    uint32_t m_iMappedYOffset;
    uint32_t m_iMappedZOffset;
    uint32_t m_iMappedWidth;
    uint32_t m_iMappedHeight;
    uint32_t m_iMappedDepth;
};

SEEK_NAMESPACE_END
