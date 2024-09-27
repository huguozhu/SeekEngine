#pragma once
#include "kernel/kernel.h"
#include "math/vector.h"
#include "rhi/base/render_view.h"
#include "rhi/base/framebuffer.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11RenderTarget
*******************************************************************************/
class D3D11RenderTargetView : public RenderView
{
public:
    D3D11RenderTargetView(Context* context, TexturePtr const& tex, uint32_t lod = 0);
    virtual ~D3D11RenderTargetView() override;

    void OnAttached(RHIFrameBuffer& fb, RHIFrameBuffer::Attachment attach);
    void OnDetached(RHIFrameBuffer& fb, RHIFrameBuffer::Attachment attach);
    void ClearColor(float4 const& color);

    ID3D11RenderTargetView* GetD3DRenderTargetView() { return m_pD3dRenderTargetView.Get(); }

protected:
    ID3D11RenderTargetViewPtr m_pD3dRenderTargetView = nullptr;

};


/******************************************************************************
 * D3D11RenderTarget
 ******************************************************************************/
class D3D11CubeFaceRenderTargetView : public D3D11RenderTargetView
{
public:
    D3D11CubeFaceRenderTargetView(Context* context, TexturePtr const& tex, CubeFaceType face, uint32_t lod = 0);
};

/******************************************************************************
* D3D11DepthStencilView
*******************************************************************************/
class D3D11DepthStencilView : public RenderView
{
public:
    D3D11DepthStencilView(Context* context, TexturePtr const& tex);
    virtual ~D3D11DepthStencilView() override;

    void OnAttached(RHIFrameBuffer& fb);
    void OnDetached(RHIFrameBuffer& fb);
    void ClearDepth(float depth = 1.0);
    void ClearStencil(uint32_t stencil = 0);
    void ClearDepthStencil(float depth, uint32_t stencil);

    ID3D11DepthStencilView* GetD3DDepthStencilView() { return m_pD3D11DepthStencilView.Get(); }

protected:
    ID3D11DepthStencilViewPtr m_pD3D11DepthStencilView = nullptr;

};

class D3D11CubeDepthStencilView : public D3D11DepthStencilView
{
public:
    D3D11CubeDepthStencilView(Context* context, TexturePtr const& tex, CubeFaceType face);
};

SEEK_NAMESPACE_END
