/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_adapter.h
**
**      Brief                  : implement d3d11 adapter
**
**      Additional             : None
**
**------------------------------------------------------------------------------------------------
**
**      History                : 2021-05-28  Created by Ted Hu
**
**************************************************************************************************/
#pragma once

#include "kernel/kernel.h"

DVF_NAMESPACE_BEGIN

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


DVF_NAMESPACE_END
