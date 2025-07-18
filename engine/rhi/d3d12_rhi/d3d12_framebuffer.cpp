#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_framebuffer.h"

SEEK_NAMESPACE_BEGIN

D3D12RHIFrameBuffer::D3D12RHIFrameBuffer(Context* context)
    :RHIFrameBuffer(context)
{
    m_stD3dViewport.MinDepth = 0.0f;
    m_stD3dViewport.MaxDepth = 1.0;
}
D3D12RHIFrameBuffer::~D3D12RHIFrameBuffer()
{

}
SResult D3D12RHIFrameBuffer::OnBind()
{
    return S_Success;
}
SResult D3D12RHIFrameBuffer::OnUnbind()
{
    return S_Success;
}
SResult D3D12RHIFrameBuffer::Resolve()
{
    return S_Success;
}
void D3D12RHIFrameBuffer::Clear(uint32_t flags, float4 const& clr, float depth, int32_t stencil)
{

}
void D3D12RHIFrameBuffer::ClearRenderTarget(Attachment att, float4 const& clr)
{

}
SEEK_NAMESPACE_END