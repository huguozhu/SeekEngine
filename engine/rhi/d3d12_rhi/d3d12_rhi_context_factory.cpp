#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_fence.h"

SEEK_NAMESPACE_BEGIN



RHIFencePtr D3D12RHIContext::CreateFence()
{
    return MakeSharedPtr<D3D12Fence>(m_pContext);
}

SEEK_NAMESPACE_END