#pragma once

#include "kernel/kernel.h"
#include "rhi/d3d12_rhi/d3d12_framebuffer.h"


SEEK_NAMESPACE_BEGIN
class D3DAdapter;
class D3D12Window : public D3D12RHIFrameBuffer
{
public:
    D3D12Window(Context* context);
    virtual ~D3D12Window() override;

    SResult Create(D3DAdapter* adapter, std::string const& name, void* native_wnd);
    void Destroy();

private:
    virtual SResult SwapBuffers() override;

private:
    std::string m_szName;
    HWND m_hWnd = nullptr;

    D3DAdapter* m_pAdapter = nullptr;

    //DXGI_SWAP_CHAIN_DESC m_stSwapChainDesc;
    //IDXGISwapChainPtr m_pSwapChain = nullptr;
    int32_t m_iLeft = 0;
    int32_t m_iTop = 0;
    uint32_t m_iWidth = 0;
    uint32_t m_iHeight = 0;
};

using D3D12WindowPtr = std::shared_ptr<D3D12Window>;

SEEK_NAMESPACE_END
