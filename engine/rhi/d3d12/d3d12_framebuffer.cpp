#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_framebuffer.h"

SEEK_NAMESPACE_BEGIN

D3D12FrameBuffer::D3D12FrameBuffer(Context* context)
    :RHIFrameBuffer(context)
{
    m_stD3dViewport.MinDepth = 0.0f;
    m_stD3dViewport.MaxDepth = 1.0;
}
D3D12FrameBuffer::~D3D12FrameBuffer()
{

}
SResult D3D12FrameBuffer::OnBind()
{
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
SEEK_NAMESPACE_END