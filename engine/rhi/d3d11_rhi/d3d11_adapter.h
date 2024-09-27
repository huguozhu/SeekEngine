#pragma once

#include "kernel/kernel.h"

SEEK_NAMESPACE_BEGIN

static const uint32_t INVALID_ADAPTER_INDEX = (uint32_t)-1;

class D3D11Adapter final
{
public:
    D3D11Adapter(uint32_t adapter_no, IDXGIAdapter1* adapter);
    virtual ~D3D11Adapter();
    IDXGIAdapter1* DXGIAdapter() { return m_pAdapter.Get(); }
    void ResetAdapter(IDXGIAdapter1* adapter);
    const DXGI_ADAPTER_DESC1& DXGIAdapterDesc() { return m_stAdapterDesc; }
private:
    uint32_t m_iAdapterNo = INVALID_ADAPTER_INDEX;
    IDXGIAdapter1Ptr m_pAdapter = nullptr;
    DXGI_ADAPTER_DESC1 m_stAdapterDesc;
};

using D3D11AdapterPtr = std::shared_ptr<D3D11Adapter>;


SEEK_NAMESPACE_END
