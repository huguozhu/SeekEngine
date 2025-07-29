#pragma once
#include "kernel/kernel.h"
#include "math/vector.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/base/rhi_texture.h"
#include "rhi/d3d11/d3d11_predeclare.h"


SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11 Srv
*******************************************************************************/
class D3D11ShaderResourceView : public RHIShaderResourceView
{
public:
    D3D11ShaderResourceView(Context* context) 
        : RHIShaderResourceView(context) {}
    virtual ID3D11ShaderResourceView* GetD3DSrv() = 0;

protected:    
    ID3D11ShaderResourceViewPtr m_pD3DSrv = nullptr;
};

class D3D11TextureShaderResourceView final : public D3D11ShaderResourceView
{
public:    
    D3D11TextureShaderResourceView(Context* context, RHITexturePtr const& texture, PixelFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels);
    ID3D11ShaderResourceView* GetD3DSrv() override;
};

class D3D11CubeTextureFaceShaderResourceView final : public D3D11ShaderResourceView
{
public:
    D3D11CubeTextureFaceShaderResourceView(Context* context, RHITexturePtr const& texture_cube, PixelFormat pf, int array_index, CubeFaceType face,
        uint32_t first_level, uint32_t num_levels);
    ID3D11ShaderResourceView* GetD3DSrv() override;
};

class D3D11BufferShaderResourceView final : public D3D11ShaderResourceView
{
public:
    D3D11BufferShaderResourceView(Context* context, RHIGpuBufferPtr const& buffer, PixelFormat pf, uint32_t first_elem, uint32_t num_elems);
    ID3D11ShaderResourceView* GetD3DSrv() override;
};


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

    ID3D11RenderTargetView* GetD3DRtv() override;
};
class D3D11Texture3DRtv final : public D3D11RenderTargetView
{
public:
    D3D11Texture3DRtv(Context* context, RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level);

    ID3D11RenderTargetView* GetD3DRtv() override;
};
class D3D11TextureCubeFaceRtv final : public D3D11RenderTargetView
{
public:
    D3D11TextureCubeFaceRtv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level);

    ID3D11RenderTargetView* GetD3DRtv() override;
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
    ID3D11DepthStencilView* GetD3DDsv() override;
};
class D3D11TextureCubeFaceDsv : public D3D11DepthStencilView
{
public:
    D3D11TextureCubeFaceDsv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level);
    ID3D11DepthStencilView* GetD3DDsv() override;
};


/******************************************************************************
* D3D11 Uav
*******************************************************************************/
class D3D11UnorderedAccessView : public RHIUnorderedAccessView
{
public:
    D3D11UnorderedAccessView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres);
    virtual ID3D11UnorderedAccessView* GetD3DUav() = 0;

protected:
    ID3D11UnorderedAccessViewPtr m_pD3DUav = nullptr;
    void* m_pSrc;
    uint32_t m_iFirstSubres;
    uint32_t m_iNumSubres;
};

class D3D11Texture2DCubeUnorderedAccessView final : public D3D11UnorderedAccessView
{
public:
    D3D11Texture2DCubeUnorderedAccessView(Context* context, RHITexturePtr const& tex_2d_cube, PixelFormat pf,
        int first_array_index, int array_size, int mip_level);
    ID3D11UnorderedAccessView* GetD3DUav() override;
};

class D3D11Texture3DUnorderedAccessView final : public D3D11UnorderedAccessView
{
public:
    D3D11Texture3DUnorderedAccessView(Context* context, RHITexturePtr const& tex_3d, PixelFormat pf, int array_index,
        uint32_t first_slice, uint32_t num_slices, int mip_level);
    ID3D11UnorderedAccessView* GetD3DUav() override;
};

class D3D11TextureCubeFaceUnorderedAccessView final : public D3D11UnorderedAccessView
{
public:
    D3D11TextureCubeFaceUnorderedAccessView(Context* context, RHITexturePtr const& tex_cube, PixelFormat pf, int array_index, CubeFaceType face, int mip_level);
    ID3D11UnorderedAccessView* GetD3DUav() override;
};

class D3D11BufferUnorderedAccessView final : public D3D11UnorderedAccessView
{
public:
    D3D11BufferUnorderedAccessView(Context* context, RHIGpuBufferPtr const& gb, PixelFormat pf, uint32_t first_elem, uint32_t num_elems);
    ID3D11UnorderedAccessView* GetD3DUav() override;
};
SEEK_NAMESPACE_END
