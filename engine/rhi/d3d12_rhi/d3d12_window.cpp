#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_window.h"

SEEK_NAMESPACE_BEGIN

D3D12Window::D3D12Window(Context* context)
    :D3D12RHIFrameBuffer(context)
{

}
D3D12Window::~D3D12Window()
{

}

SEEK_NAMESPACE_END