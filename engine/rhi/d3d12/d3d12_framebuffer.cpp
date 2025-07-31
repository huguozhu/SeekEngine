#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_framebuffer.h"
#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d12/d3d12_resource.h"

SEEK_NAMESPACE_BEGIN

D3D12FrameBuffer::D3D12FrameBuffer(Context* context)
    :RHIFrameBuffer(context)
{
    m_stD3dViewport.MinDepth = 0.0f;
    m_stD3dViewport.MaxDepth = 1.0;
}
SResult D3D12FrameBuffer::OnBind()
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());

    for (uint32_t i = 0; i < m_vRenderTargets.size(); ++i)
    {

    }
    return S_Success;
}
SResult D3D12FrameBuffer::OnUnbind()
{
    return S_Success;
}
SResult D3D12FrameBuffer::Resolve()
{
    return S_Success;
}
void D3D12FrameBuffer::Clear(uint32_t flags, float4 const& clr, float depth, int32_t stencil)
{

}
void D3D12FrameBuffer::ClearRenderTarget(Attachment att, float4 const& clr)
{

}

void D3D12FrameBuffer::BindBarrier(ID3D12GraphicsCommandList* cmd_list)
{
    if (m_bViewDirty)
    {
        this->UpdateAllViews();
        m_bViewDirty = false;
    }

    for (uint32_t i = 0; i < m_vD3dRtvResources.size(); ++i)
    {
        D3D12Resource* pRes     = std::get<0>(m_vD3dRtvResources[i]);
        uint32_t first_subres   = std::get<1>(m_vD3dRtvResources[i]);
        uint32_t num_subres     = std::get<2>(m_vD3dRtvResources[i]);
        for (uint32_t j = 0; j < num_subres; ++j)
            pRes->UpdateResourceBarrier(cmd_list, first_subres + j, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    if (std::get<0>(m_DsvResource))
    {
        D3D12Resource* pRes     = std::get<0>(m_DsvResource);
        uint32_t first_subres   = std::get<1>(m_DsvResource);
        uint32_t num_subres     = std::get<2>(m_DsvResource);
        for (uint32_t j = 0; j < num_subres; ++j)
            pRes->UpdateResourceBarrier(cmd_list, first_subres + j, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }


}
void D3D12FrameBuffer::SetRenderTargets(ID3D12GraphicsCommandList* cmd_list)
{
    m_vD3dRtvResources.clear();
    m_vD3dRtvCpuHandles.resize(m_vRenderTargets.size());
}
void D3D12FrameBuffer::UpdateAllViews()
{

}
SEEK_NAMESPACE_END