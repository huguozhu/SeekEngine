#pragma once

#include "kernel/kernel.h"
#include "rhi/d3d_rhi_common/d3d_common_predeclare.h"

SEEK_NAMESPACE_BEGIN

static const uint32_t INVALID_ADAPTER_INDEX = (uint32_t)-1;

class D3DAdapter final
{
public:
    D3DAdapter(uint32_t adapter_no, IDXGIAdapter1* adapter);
    virtual ~D3DAdapter();
    IDXGIAdapter1* DXGIAdapter() { return m_pAdapter.Get(); }
    void ResetAdapter(IDXGIAdapter1* adapter);
    const DXGI_ADAPTER_DESC1& DXGIAdapterDesc() { return m_stAdapterDesc; }
private:
    uint32_t m_iAdapterNo = INVALID_ADAPTER_INDEX;
    IDXGIAdapter1Ptr m_pAdapter = nullptr;
    DXGI_ADAPTER_DESC1 m_stAdapterDesc;
};

using D3DAdapterPtr = std::shared_ptr<D3DAdapter>;


SEEK_NAMESPACE_END
