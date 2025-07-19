#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_render_view.h"
#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

D3D12RenderTargetView::D3D12RenderTargetView(Context* context, ID3D12Resource* src, uint32_t first_subres, uint32_t num_subres)
    :RHIRenderTargetView(context), m_pResource(src)
{

}

D3D12UnorderedAccessView::D3D12UnorderedAccessView(Context* context, ID3D12Resource* res)
    :RHIUnorderedAccessView(context), m_pResource(res)
{

}

SEEK_NAMESPACE_END
