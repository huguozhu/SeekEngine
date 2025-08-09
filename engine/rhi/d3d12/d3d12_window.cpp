#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_context.h"
#include "rhi/d3d12/d3d12_window.h"
#include "rhi/d3d12/d3d12_texture.h"

SEEK_NAMESPACE_BEGIN

D3D12Window::D3D12Window(Context* context)
    :D3D12FrameBuffer(context)
{

}
D3D12Window::~D3D12Window()
{

}
SResult D3D12Window::Create(D3DAdapter* adapter, std::string const& name, void* native_wnd)
{
    m_pAdapter = adapter;
    m_szName = name;
    m_hWnd = (HWND)native_wnd;
    do
    {
        D3D12Context& d3d12_rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        RECT rc; GetClientRect(m_hWnd, &rc);
        m_iLeft = rc.left;
        m_iTop = rc.top;
        m_iWidth = rc.right - rc.left;
        m_iHeight = rc.bottom - rc.top;
         
        m_sScDesc.BufferCount = NUM_BACK_BUFFERS;
        m_sScDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_sScDesc.Width = m_iWidth;
        m_sScDesc.Height = m_iHeight;
        m_sScDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_sScDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_sScDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        m_sScDesc.SampleDesc.Count = m_pContext->GetNumSamples();
        m_sScDesc.SampleDesc.Quality = 0;
        m_sScDesc.Scaling = DXGI_SCALING_STRETCH;
        m_sScDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        m_sScDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        m_sSfcDesc.RefreshRate.Numerator = (UINT)m_pContext->GetFpsLimitType();
        m_sSfcDesc.RefreshRate.Denominator = 1;
        m_sSfcDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        m_sSfcDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        m_sSfcDesc.Windowed = !m_pContext->IsFullScreen();

        IDXGIFactory4* factory = (IDXGIFactory4*)d3d12_rc.GetDXGIFactory1();
        ID3D12Device* pD3DDevice = d3d12_rc.GetD3D12Device();
        ID3D12CommandQueue* pD3DCommandQueue = d3d12_rc.GetD3D12CommandQueue();
        
        ComPtr<IDXGISwapChain1> sc;
        HRESULT hr = factory->CreateSwapChainForHwnd(pD3DCommandQueue, m_hWnd,  &m_sScDesc, &m_sSfcDesc, nullptr, sc.GetAddressOf());
        if (FAILED(hr))
        {
            LOG_ERROR("CreateSwapChain Error: %x.", hr);
            break;
        }
        sc.As(&m_pSwapChain);
        m_pSwapChain->SetFullscreenState(m_pContext->IsFullScreen(), nullptr);
        
        for (uint32_t i = 0; i < m_vBackBufferTexes.size(); i++)
        {
            ID3D12ResourcePtr v = nullptr;
            SEEK_THROW_IFFAIL(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(v.GetAddressOf())));
            m_vBackBufferTexes[i] = d3d12_rc.CreateTexture2D(v);
            m_vBackBufferRtvs[i] = d3d12_rc.Create2DRenderTargetView(m_vBackBufferTexes[i], 0, 1, 0);
        }

        m_iCurBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        this->AttachTargetView(Attachment::Color0, m_vBackBufferRtvs[m_iCurBackBufferIndex]);

        RHITexture::Desc desc = {};
        desc.width = m_iWidth;
        desc.height = m_iHeight;
        desc.type = TextureType::Tex2D; 
        desc.format = PixelFormat::D24S8;
        desc.flags = RESOURCE_FLAG_GPU_READ | RESOURCE_FLAG_GPU_WRITE;
        RHITexturePtr tex_ds = d3d12_rc.CreateTexture2D(desc);
        this->AttachDepthStencilView(d3d12_rc.Create2DDepthStencilView(tex_ds));

    } while (0);

    return S_Success;
}
void D3D12Window::Destroy()
{
    
}
SResult D3D12Window::SwapBuffers()
{
    if (m_pSwapChain)
    {
        D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
        ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

        D3D12Texture& rt_tex = static_cast<D3D12Texture&>(*m_vBackBufferTexes[m_iCurBackBufferIndex]);
        rt_tex.UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_PRESENT);
        rc.FlushResourceBarriers(cmd_list);
        
        m_pSwapChain->Present(0, 0);
        m_iCurBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        this->AttachTargetView(Attachment::Color0, m_vBackBufferRtvs[m_iCurBackBufferIndex]);
    }

    return S_Success;
}
SEEK_NAMESPACE_END