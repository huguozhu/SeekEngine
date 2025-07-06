#include "rhi/d3d11_rhi/d3d11_render_view.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"

#include "rhi/base/rhi_texture.h"

#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 4     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN
/******************************************************************************
* D3D11RenderTarget
*******************************************************************************/
D3D11RenderTargetView::D3D11RenderTargetView(Context* context, RHITexturePtr const& tex, uint32_t mip_level)
    : RHIRenderView(context, tex, mip_level), m_pD3dRenderTargetView(nullptr)
{
    if (tex->Type() != TextureType::Cube)
    {
        D3D11Texture& d3d_tex = static_cast<D3D11Texture&>(*tex);
        m_pD3dRenderTargetView = d3d_tex.GetD3DRtv();
    }
}
D3D11RenderTargetView::~D3D11RenderTargetView()
{
    m_pD3dRenderTargetView.Reset();
}
void D3D11RenderTargetView::OnAttached(RHIFrameBuffer& fb, RHIFrameBuffer::Attachment attach)
{
    // empty
}
void D3D11RenderTargetView::OnDetached(RHIFrameBuffer& fb, RHIFrameBuffer::Attachment attach)
{
    // empty
}
void D3D11RenderTargetView::ClearColor(float4 const& color)
{
    if (m_pD3dRenderTargetView)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearRenderTargetView(m_pD3dRenderTargetView.Get(), &color[0]);
    }
    else
        LOG_ERROR("render target view is null");
}

/******************************************************************************
 * D3D11RenderTarget
 ******************************************************************************/
D3D11CubeFaceRenderTargetView::D3D11CubeFaceRenderTargetView(Context* context, RHITexturePtr const& tex, CubeFaceType face, uint32_t mip_level)
    :D3D11RenderTargetView(context, tex, mip_level)
{
    if (tex->Type() != TextureType::Cube)
        return;
    m_eCubeType = face;
    D3D11TextureCube& d3d_tex = static_cast<D3D11TextureCube&>(*tex);
    m_pD3dRenderTargetView = d3d_tex.GetD3DRtv(face, mip_level);
}

/******************************************************************************
* D3D11DepthStencilView
*******************************************************************************/
D3D11DepthStencilView::D3D11DepthStencilView(Context* context, RHITexturePtr const& tex)
    : RHIRenderView(context, tex), m_pD3D11DepthStencilView(nullptr)
{
    if (tex->Type() != TextureType::Cube)
    {
        D3D11Texture& d3d_tex = static_cast<D3D11Texture&>(*tex);
        m_pD3D11DepthStencilView = d3d_tex.GetD3DDsv();
    }
}
D3D11DepthStencilView::~D3D11DepthStencilView()
{
    m_pD3D11DepthStencilView.Reset();
}
void D3D11DepthStencilView::OnAttached(RHIFrameBuffer& fb)
{
    // empty
}
void D3D11DepthStencilView::OnDetached(RHIFrameBuffer& fb)
{
    // empty
}
void D3D11DepthStencilView::ClearDepth(float depth)
{
    if (m_pD3D11DepthStencilView)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11DepthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
    }
    else
        LOG_ERROR("depth stencil view is null");
}
void D3D11DepthStencilView::ClearStencil(uint32_t stencil)
{
    if (m_pD3D11DepthStencilView)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11DepthStencilView.Get(), D3D11_CLEAR_STENCIL, 0, (UINT8)stencil);
    }
    else
        LOG_ERROR("depth stencil view is null");
}
void D3D11DepthStencilView::ClearDepthStencil(float depth, uint32_t stencil)
{
    if (m_pD3D11DepthStencilView)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, (UINT8)stencil);
    }
    else
        LOG_ERROR("depth stencil view is null");
}


/******************************************************************************
* D3D11CubeDepthStencilView
*******************************************************************************/
D3D11CubeDepthStencilView::D3D11CubeDepthStencilView(Context* context, RHITexturePtr const& tex, CubeFaceType face)
    : D3D11DepthStencilView(context, tex)
{
    if (tex->Type() != TextureType::Cube)
        return;
    m_eCubeType = face;
    D3D11TextureCube& d3d_tex = static_cast<D3D11TextureCube&>(*tex);
    m_pD3D11DepthStencilView = d3d_tex.GetD3DDsv(face);
}


SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
