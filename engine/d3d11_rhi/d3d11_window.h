/*************************************************************************************************
**
**      Copyright (C) 2021. All rights reserved.
**
**      Name                   : d3d11_window.h
**
**      Brief                  : d3d11 native window
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
#include "rendering_d3d11/d3d11_framebuffer.h"


DVF_NAMESPACE_BEGIN
class D3D11Window : public D3D11FrameBuffer
{
public:
    D3D11Window(Context* context);
    virtual ~D3D11Window() override;

    DVFResult Create(D3D11Adapter* adapter, std::string const& name, void* native_wnd);
    void Destroy();

private:
    virtual DVFResult SwapBuffers() override;

private:
    std::string m_szName;
    HWND m_hWnd = nullptr;

    D3D11Adapter* m_pAdapter = nullptr;

    DXGI_SWAP_CHAIN_DESC m_stSwapChainDesc;
    IDXGISwapChainPtr m_pSwapChain = nullptr;
    int32_t m_iLeft = 0;
    int32_t m_iTop = 0;
    uint32_t m_iWidth = 0;
    uint32_t m_iHeight = 0;
};

using D3D11WindowPtr = std::shared_ptr<D3D11Window>;

DVF_NAMESPACE_END
