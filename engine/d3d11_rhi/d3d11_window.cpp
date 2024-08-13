
#include "rendering_d3d11/d3d11_predeclare.h"
#include "rendering_d3d11/d3d11_render_context.h"
#include "rendering_d3d11/d3d11_window.h"
#include "rendering_d3d11/d3d11_adapter.h"
#include "rendering_d3d11/d3d11_texture.h"

#include "kernel/context.h"

#include "util/log.h"
#include "util/error.h"
#include <dxgidebug.h>

#define DVF_MACRO_FILE_UID 5     // this code is auto generated, don't touch it!!!

DVF_NAMESPACE_BEGIN
D3D11Window::D3D11Window(Context* context)
    : D3D11FrameBuffer(context)
{

}

D3D11Window::~D3D11Window()
{
    Destroy();
}

DVFResult D3D11Window::Create(D3D11Adapter* adapter, std::string const& name, void* native_wnd)
{
    m_pAdapter = adapter;
    m_szName = name;
    m_hWnd = (HWND)native_wnd;
    do {
        D3D11RenderContext& d3d11_rc = static_cast<D3D11RenderContext&>(m_pContext->RenderContextInstance());
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        m_iLeft = rc.left;
        m_iTop = rc.top;
        m_iWidth = rc.right - rc.left;
        m_iHeight = rc.bottom - rc.top;

        m_stSwapChainDesc.BufferCount = 2;
        m_stSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_stSwapChainDesc.BufferDesc.Width = m_iWidth;
        m_stSwapChainDesc.BufferDesc.Height = m_iHeight;
        m_stSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        m_stSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        m_stSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        m_stSwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        m_stSwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        m_stSwapChainDesc.SampleDesc.Count = 1;
        m_stSwapChainDesc.SampleDesc.Quality = 0;
        if (1/*d3d11_rc.GetDxgiSubVerion() < 4 || m_pContext->GetNumSamples() > 1*/)
        {
            m_stSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            m_stSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        }
        else
        {
            m_stSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            m_stSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        m_stSwapChainDesc.Windowed = TRUE;
        m_stSwapChainDesc.OutputWindow = m_hWnd;

        IDXGIFactory1* factory1 = d3d11_rc.GetDXGIFactory1();
        ID3D11Device* pD3DDevice = d3d11_rc.GetD3D11Device();
        HRESULT hr = factory1->CreateSwapChain(pD3DDevice, &m_stSwapChainDesc, m_pSwapChain.GetAddressOf());
        if (FAILED(hr))
        {
            LOG_ERROR("CreateSwapChain Error: %x.", hr);
            break;
        }

        hr = factory1->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
        {
            LOG_ERROR("MakeWindowAssociation Error, hr=%x", hr);
            break;
        }

        hr = m_pSwapChain->SetFullscreenState(false, nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("SetFullscreenState Error, hr=%x", hr);
            break;
        }

        ID3D11Texture2DPtr back_buffer;
        hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back_buffer.GetAddressOf());
        if (FAILED(hr))
        {
            LOG_ERROR("GetBuffer Error, hr=%x.", hr);
            break;
        }

        TexturePtr pBackBufferTex = d3d11_rc.CreateTexture2D(back_buffer);
        RenderViewPtr drv = d3d11_rc.CreateRenderTargetView(pBackBufferTex);
        this->AttachTargetView(Attachment::Color0, drv);

        Texture::Desc tex_desc;
        tex_desc.type = TextureType::Tex2D;
        tex_desc.width = m_iWidth;
        tex_desc.height = m_iHeight;
        tex_desc.format = PixelFormat::D24S8;
        tex_desc.flags = RESOURCE_FLAG_RENDER_TARGET;
        TexturePtr pDepthStencilTex = d3d11_rc.CreateTexture2D(tex_desc);
        RenderViewPtr dsv = d3d11_rc.CreateDepthStencilView(pDepthStencilTex);
        this->AttachDepthStencilView(dsv);

        return DVF_Success;
    } while (0);

    Destroy();
    return ERR_NOT_SUPPORT;
}

void D3D11Window::Destroy()
{
    Reset();
    m_stSwapChainDesc = {};
    m_pSwapChain.Reset();
    m_pAdapter = nullptr;
    m_szName.clear();
    m_hWnd = nullptr;
}

DVFResult D3D11Window::SwapBuffers()
{
    if (m_pSwapChain)
    {
        if (FAILED(m_pSwapChain->Present(0, 0)))
            return ERR_NOT_SUPPORT;
    }
    return DVF_Success;
}


DVF_NAMESPACE_END

#undef DVF_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
