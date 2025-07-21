#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_fence.h"



SEEK_NAMESPACE_BEGIN

class D3D12Fence : public RHIFence
{
public:
    D3D12Fence(Context* context);

    uint64_t Signal() override;
    void Wait(uint64_t value)override;
    bool IsCompleted(uint64_t value)override;

    uint64_t Signal(ID3D12CommandQueue* cmd_queue);

protected:

    ID3D12FencePtr          m_pFence = nullptr;
    std::atomic<uint64_t>   m_iFenceValue;
    uint64_t                m_iLastCompletedValue = 0;
    HANDLE                  m_hFenceEvent = nullptr;

};

SEEK_NAMESPACE_END
