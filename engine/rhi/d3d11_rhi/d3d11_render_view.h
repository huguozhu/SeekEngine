#pragma once
#include "kernel/kernel.h"
#include "math/vector.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/d3d11_rhi/d3d11_predeclare.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11 Rtv
*******************************************************************************/
class D3D11Rtv : public RHIRenderTargetView
{
public:
    D3D11Rtv(Context* context, void* src, uint32_t first_subres, uint32_t num_subres);
    void OnAttached(RHIFrameBuffer& fb, RHIFrameBuffer::Attachment attach) {}
    void OnDetached(RHIFrameBuffer& fb, RHIFrameBuffer::Attachment attach) {}
    void ClearColor(float4 const& color);

    virtual ID3D11RenderTargetView* GetD3DRtv() = 0;

protected:
    ID3D11RenderTargetViewPtr m_pD3DRtv = nullptr;
    void* m_pSrc = nullptr;
    uint32_t m_iFirstSubres;
    uint32_t m_iNumSubres;
};

class D3D11Texture2DCubeRtv final : public D3D11Rtv
{
public:
    D3D11Texture2DCubeRtv(Context* context, RHITexturePtr const& texture_2d_cube, int first_array_index, int array_size, int mip_level);

    ID3D11RenderTargetView* GetD3DRtv();
};
class D3D11Texture3DRtv final : public D3D11Rtv
{
public:
    D3D11Texture3DRtv(Context* context, RHITexturePtr const& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int mip_level);

    ID3D11RenderTargetView* GetD3DRtv();
};
class D3D11TextureCubeFaceRtv final : public D3D11Rtv
{
public:
    D3D11TextureCubeFaceRtv(Context* context, RHITexturePtr const& texture_cube, int array_index, CubeFaceType face, int mip_level);

    ID3D11RenderTargetView* GetD3DRtv();
};


/******************************************************************************
* D3D11DepthStencilView
*******************************************************************************/
class D3D11DepthStencilView : public RHIRenderView
{
public:
    D3D11DepthStencilView(Context* context, RHITexturePtr const& tex);
    virtual ~D3D11DepthStencilView();

    void OnAttached(RHIFrameBuffer& fb);
    void OnDetached(RHIFrameBuffer& fb);
    void ClearDepth(float depth = 1.0);
    void ClearStencil(uint32_t stencil = 0);
    void ClearDepthStencil(float depth, uint32_t stencil);

    ID3D11DepthStencilView* GetD3DDsv() { return m_pD3D11DepthStencilView.Get(); }

protected:
    ID3D11DepthStencilViewPtr m_pD3D11DepthStencilView = nullptr;

};

class D3D11CubeDepthStencilView : public D3D11DepthStencilView
{
public:
    D3D11CubeDepthStencilView(Context* context, RHITexturePtr const& tex, CubeFaceType face);
};

SEEK_NAMESPACE_END
