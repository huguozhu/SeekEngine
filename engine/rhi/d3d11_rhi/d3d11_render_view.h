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
class D3D11RenderTargetView : public RHIRenderTargetView
{
public:
    D3D11RenderTargetView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres);
    void ClearColor(float4 const& color);

    virtual ID3D11RenderTargetView* GetD3DRtv() = 0;

protected:
    ID3D11RenderTargetViewPtr m_pD3DRtv = nullptr;
    void* m_pSrc = nullptr;
    uint32_t m_iFirstSubres;
    uint32_t m_iNumSubres;
};

class D3D11Texture2DCubeRtv final : public D3D11RenderTargetView
{
public:
    D3D11Texture2DCubeRtv(Context* context, RHITexturePtr const& tex_2d_cube, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level);

    ID3D11RenderTargetView* GetD3DRtv();
};
class D3D11Texture3DRtv final : public D3D11RenderTargetView
{
public:
    D3D11Texture3DRtv(Context* context, RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);

    ID3D11RenderTargetView* GetD3DRtv();
};
class D3D11TextureCubeFaceRtv final : public D3D11RenderTargetView
{
public:
    D3D11TextureCubeFaceRtv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level);

    ID3D11RenderTargetView* GetD3DRtv();
};

/******************************************************************************
* D3D11Dsv
*******************************************************************************/
class D3D11DepthStencilView : public RHIDepthStencilView
{
public:
    D3D11DepthStencilView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres);

    void ClearDepth(float depth = 1.0);
    void ClearStencil(uint32_t stencil = 0);
    void ClearDepthStencil(float depth, uint32_t stencil);

    virtual ID3D11DepthStencilView* GetD3DDsv() = 0;

protected:
    ID3D11DepthStencilViewPtr m_pD3D11Dsv = nullptr;
    void* m_pSrc = nullptr;
    uint32_t m_iFirstSubres;
    uint32_t m_iNumSubres;
};

class D3D11Texture2DDsv : public D3D11DepthStencilView
{
public:
    D3D11Texture2DDsv(Context* context, RHITexturePtr const& tex_2d, uint32_t first_array_index = 0, uint32_t array_size = 1, uint32_t mip_level = 0);
    ID3D11DepthStencilView* GetD3DDsv();
};
class D3D11TextureCubeFaceDsv : public D3D11DepthStencilView
{
public:
    D3D11TextureCubeFaceDsv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level);
    ID3D11DepthStencilView* GetD3DDsv();
};


SEEK_NAMESPACE_END
