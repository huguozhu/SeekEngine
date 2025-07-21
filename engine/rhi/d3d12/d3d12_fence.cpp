#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_fence.h"
#include "rhi/d3d12/d3d12_rhi_context.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN


D3D12Fence::D3D12Fence(Context* context)
    :RHIFence(context)
{
    ID3D12Device* pDevice = static_cast<D3D12RHIContext&>(m_pContext->RHIContextInstance()).GetD3D12Device();
    pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.ReleaseAndGetAddressOf()));

    m_hFenceEvent = ::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
}
uint64_t D3D12Fence::Signal()
{
    ID3D12CommandQueue* pCmdQueue = static_cast<D3D12RHIContext&>(m_pContext->RHIContextInstance()).GetD3D12CommandQueue();
    return this->Signal(pCmdQueue);
}
void D3D12Fence::Wait(uint64_t value)
{
    if (!this->IsCompleted(value))
    {
        m_pFence->SetEventOnCompletion(value, m_hFenceEvent);
        ::WaitForSingleObjectEx(m_hFenceEvent, INFINITE, false);
    }
}
bool D3D12Fence::IsCompleted(uint64_t value)
{
    if (value > m_iLastCompletedValue)
        m_iLastCompletedValue = std::max(m_iLastCompletedValue, m_pFence->GetCompletedValue());
    return value <= m_iLastCompletedValue;
}
uint64_t D3D12Fence::Signal(ID3D12CommandQueue* cmd_queue)
{
    uint64_t val = m_iFenceValue;
    cmd_queue->Signal(m_pFence.Get(), val);
    m_iFenceValue++;
    return val;
}


SEEK_NAMESPACE_END
