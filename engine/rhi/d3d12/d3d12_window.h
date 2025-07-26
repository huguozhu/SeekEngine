#pragma once

#include "kernel/kernel.h"
#include "rhi/d3d12/d3d12_framebuffer.h"


SEEK_NAMESPACE_BEGIN
class D3DAdapter;
class D3D12Window : public D3D12FrameBuffer
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
    
    IDXGISwapChain3Ptr m_pSwapChain = nullptr;
    uint32_t m_iCurBackBufferIndex = 0;
    int32_t m_iLeft = 0;
    int32_t m_iTop = 0;
    uint32_t m_iWidth = 0;
    uint32_t m_iHeight = 0;

    std::array<RHITexturePtr, NUM_BACK_BUFFERS> m_vBackBufferTexes;
    std::array<RHIRenderTargetViewPtr, NUM_BACK_BUFFERS> m_vBackBufferRtvs;

    DXGI_SWAP_CHAIN_DESC1 m_sScDesc;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC m_sSfcDesc;
};

using D3D12WindowPtr = std::shared_ptr<D3D12Window>;

SEEK_NAMESPACE_END
