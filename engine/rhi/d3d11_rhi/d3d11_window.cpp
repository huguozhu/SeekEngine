
#include "rhi/d3d11_rhi/d3d11_predeclare.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"
#include "rhi/d3d11_rhi/d3d11_window.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d_rhi_common/d3d_adapter.h"

#include "kernel/context.h"

#include "utils/log.h"
#include "utils/error.h"
#include <dxgidebug.h>

#define SEEK_MACRO_FILE_UID 5     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN
D3D11Window::D3D11Window(Context* context)
    : D3D11RHIFrameBuffer(context)
{

}

D3D11Window::~D3D11Window()
{
    Destroy();
}

SResult D3D11Window::Create(D3DAdapter* adapter, std::string const& name, void* native_wnd)
{
    m_pAdapter = adapter;
    m_szName = name;
    m_hWnd = (HWND)native_wnd;
    do {
        D3D11RHIContext& d3d11_rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        RECT rc;
        GetClientRect(m_hWnd, &rc);
        m_iLeft = rc.left;
        m_iTop = rc.top;
        m_iWidth = rc.right - rc.left;
        m_iHeight = rc.bottom - rc.top;

        DXGI_SWAP_CHAIN_DESC desc;
        desc.BufferCount = 2;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferDesc.Width = m_iWidth;
        desc.BufferDesc.Height = m_iHeight;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.RefreshRate.Numerator = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        if (1/*d3d11_rc.GetDxgiSubVerion() < 4 || m_pContext->GetNumSamples() > 1*/)
        {
            desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        }
        else
        {
            desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        desc.Windowed = TRUE;
        desc.OutputWindow = m_hWnd;

        IDXGIFactory1* factory1 = d3d11_rc.GetDXGIFactory1();
        ID3D11Device* pD3DDevice = d3d11_rc.GetD3D11Device();
        HRESULT hr = factory1->CreateSwapChain(pD3DDevice, &desc, m_pSwapChain.GetAddressOf());
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

        RHITexturePtr pBackBufferTex = d3d11_rc.CreateTexture2D(back_buffer);
        RHIRenderTargetViewPtr drv = d3d11_rc.Create2DRenderTargetView(pBackBufferTex);
        this->AttachTargetView(Attachment::Color0, drv);

        RHITexture::Desc tex_desc;
        tex_desc.type = TextureType::Tex2D;
        tex_desc.width = m_iWidth;
        tex_desc.height = m_iHeight;
        tex_desc.format = PixelFormat::D24S8;
        tex_desc.flags = RESOURCE_FLAG_GPU_WRITE;
        RHITexturePtr pDepthStencilTex = d3d11_rc.CreateTexture2D(tex_desc);
        RHIDepthStencilViewPtr dsv = d3d11_rc.Create2DDepthStencilView(pDepthStencilTex);
        this->AttachDepthStencilView(dsv);

        return S_Success;
    } while (0);

    Destroy();
    return ERR_NOT_SUPPORT;
}

void D3D11Window::Destroy()
{
    Reset();
    m_pSwapChain.Reset();
    m_pAdapter = nullptr;
    m_szName.clear();
    m_hWnd = nullptr;
}

SResult D3D11Window::SwapBuffers()
{
    if (m_pSwapChain)
    {
        if (FAILED(m_pSwapChain->Present(0, 0)))
            return ERR_NOT_SUPPORT;
    }
    return S_Success;
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
