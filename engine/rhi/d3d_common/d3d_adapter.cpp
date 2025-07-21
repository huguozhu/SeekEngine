#include "rhi/d3d11/d3d11_predeclare.h"
#include "rhi/d3d_common/d3d_adapter.h"


SEEK_NAMESPACE_BEGIN

D3DAdapter::D3DAdapter(uint32_t adapter_no, IDXGIAdapter1* adapter)
    : m_iAdapterNo(adapter_no)
    , m_pAdapter(nullptr)
{
    this->ResetAdapter(adapter);
}

D3DAdapter::~D3DAdapter()
{
    m_pAdapter.Reset();
}

void D3DAdapter::ResetAdapter(IDXGIAdapter1* adapter)
{
    m_pAdapter = adapter;
    if (m_pAdapter)
        m_pAdapter->GetDesc1(&m_stAdapterDesc);
    else
        ZeroMemory(&m_stAdapterDesc, sizeof(m_stAdapterDesc));
}

SEEK_NAMESPACE_END
