#include "rhi/d3d11/d3d11_render_view.h"
#include "rhi/d3d11/d3d11_texture.h"
#include "rhi/d3d11/d3d11_context.h"
#include "rhi/d3d11/d3d11_gpu_buffer.h"

#include "rhi/base/rhi_texture.h"

#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 4     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11 Srv
*******************************************************************************/
D3D11TextureShaderResourceView::D3D11TextureShaderResourceView(Context* context, RHITexturePtr const& texture, PixelFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
    :D3D11ShaderResourceView(context)
{
    SEEK_ASSERT(texture->Flags() & RESOURCE_FLAG_GPU_READ);

    m_Param.pixel_format = pf == PixelFormat::Unknown ? texture->Format() : pf;

    m_Param.texture = texture;
    m_Param.first_array_index = first_array_index;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = first_level;
    this->GetD3DSrv();
}
ID3D11ShaderResourceView* D3D11TextureShaderResourceView::GetD3DSrv()
{
    if (!m_pD3DSrv && m_Param.texture)
    {
        m_pD3DSrv = ((D3D11Texture&)*m_Param.texture).GetD3DSrv(m_Param.pixel_format, m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level, m_iNumMipLevels);
    }
    return m_pD3DSrv.Get();
}


D3D11CubeTextureFaceShaderResourceView::D3D11CubeTextureFaceShaderResourceView(Context* context, RHITexturePtr const& texture_cube, PixelFormat pf, 
    int array_index, CubeFaceType face, uint32_t first_level, uint32_t num_levels)
    :D3D11ShaderResourceView(context)
{
    m_Param.pixel_format = pf == PixelFormat::Unknown ? texture_cube->Format() : pf;
    
    m_Param.texture = texture_cube;
    m_Param.first_array_index = array_index * 6 + (uint32_t)face;
    m_Param.num_arrays = 1;
    m_Param.mip_level = first_level;
    m_iNumMipLevels = num_levels;
    this->GetD3DSrv();
}
ID3D11ShaderResourceView* D3D11CubeTextureFaceShaderResourceView::GetD3DSrv()
{
    if (!m_pD3DSrv && m_Param.texture)
    {
        uint32_t array_index = m_Param.first_array_index / 6;
        CubeFaceType face = (CubeFaceType)(m_Param.first_array_index - array_index * 6);

        m_pD3DSrv = ((D3D11Texture&)*m_Param.texture).GetD3DSrv(m_Param.pixel_format, array_index, face, m_Param.mip_level, m_iNumMipLevels);
    }
    return m_pD3DSrv.Get();
}


D3D11BufferShaderResourceView::D3D11BufferShaderResourceView(Context* context, RHIGpuBufferPtr const& buffer, PixelFormat pf, uint32_t first_elem, uint32_t num_elems)
    :D3D11ShaderResourceView(context)
{
    m_Param.pixel_format = pf;

    m_Param.buffer = buffer;
    m_Param.first_elem = first_elem;
    m_Param.num_elem = num_elems;
    this->GetD3DSrv();
}
ID3D11ShaderResourceView* D3D11BufferShaderResourceView::GetD3DSrv()
{
    if (!m_pD3DSrv && m_Param.buffer)
    {
        m_pD3DSrv = ((D3D11GpuBuffer&)*m_Param.buffer).GetD3DSrv(m_Param.pixel_format, m_Param.first_elem, m_Param.num_elem);
    }
    return m_pD3DSrv.Get();
}

/******************************************************************************
* D3D11 Rtv
*******************************************************************************/
D3D11RenderTargetView::D3D11RenderTargetView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres)
    : RHIRenderTargetView(context), m_pSrc(src), m_iFirstSubres(first_subres), m_iNumSubres(num_subres)
{

}
void D3D11RenderTargetView::ClearColor(float4 const& color)
{
    if (m_pD3DRtv)
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearRenderTargetView(m_pD3DRtv.Get(), &color[0]);
    }
    else
        LOG_ERROR("render target view is null");
}

D3D11Texture2DCubeRtv::D3D11Texture2DCubeRtv(Context* context, RHITexturePtr const& tex_2d_cube, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
    : D3D11RenderTargetView(context, tex_2d_cube.get(), D3D11CalcSubresource(mip_level, first_array_index, tex_2d_cube->NumMips()), array_size)
{
    // for Texture
    m_iWidth = tex_2d_cube->Width();
    m_iHeight = tex_2d_cube->Height();
    m_ePixelFormat = tex_2d_cube->Format();
    m_iNumSamples = tex_2d_cube->NumSamples();

    m_Param.texture = tex_2d_cube;
    m_Param.first_array_index = first_array_index;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = mip_level;
    this->GetD3DRtv();
}
ID3D11RenderTargetView* D3D11Texture2DCubeRtv::GetD3DRtv()
{
    if (!m_pD3DRtv)
    {
        m_pD3DRtv = ((D3D11Texture*)m_Param.texture.get())->GetD3DRtv(m_ePixelFormat, m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
    }
    return m_pD3DRtv.Get();
}

D3D11Texture3DRtv::D3D11Texture3DRtv(Context* context, RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
    :D3D11RenderTargetView(context, tex_3d.get(), D3D11CalcSubresource(mip_level, array_index * tex_3d->Depth(mip_level) + first_slice, tex_3d->NumMips()), num_slices* tex_3d->NumMips() + mip_level)
{
    m_iWidth = tex_3d->Width(mip_level);
    m_iHeight = tex_3d->Height(mip_level);
    m_iNumSamples = tex_3d->NumSamples();

    m_Param.texture = tex_3d;
    m_Param.first_array_index = array_index;
    m_Param.num_arrays = 1;
    m_Param.mip_level = mip_level;

    m_Param.first_slice = first_slice;
    m_Param.num_slices = num_slices;
    this->GetD3DRtv();
}
ID3D11RenderTargetView* D3D11Texture3DRtv::GetD3DRtv()
{
    if (!m_pD3DRtv)
    {
        m_pD3DRtv = ((D3D11Texture*)m_Param.texture.get())->GetD3DRtv(m_ePixelFormat, m_Param.first_array_index, m_Param.first_slice, m_Param.num_slices, m_Param.mip_level);
    }
    return m_pD3DRtv.Get();
}
D3D11TextureCubeFaceRtv::D3D11TextureCubeFaceRtv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
    :D3D11RenderTargetView(context, tex_cube.get(), D3D11CalcSubresource(mip_level, array_index * 6 + (uint32_t)face, tex_cube->NumMips()), 1)
{
    m_iWidth = tex_cube->Width(mip_level);
    m_iHeight = tex_cube->Height(mip_level);
    m_iNumSamples = tex_cube->NumSamples();

    m_Param.texture = tex_cube;
    m_Param.first_array_index = array_index;
    m_Param.num_arrays = 1;
    m_Param.mip_level = mip_level;

    m_Param.first_face = face;
    m_Param.num_faces = 1;

    this->GetD3DRtv();
}
ID3D11RenderTargetView* D3D11TextureCubeFaceRtv::GetD3DRtv()
{
    if (!m_pD3DRtv)
    {
        m_pD3DRtv = ((D3D11Texture*)m_Param.texture.get())->GetD3DRtv(m_ePixelFormat, m_Param.first_array_index, m_Param.first_face, m_Param.mip_level);
    }
    return m_pD3DRtv.Get();
}

/******************************************************************************
* D3D11 Dsv
*******************************************************************************/
D3D11DepthStencilView::D3D11DepthStencilView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres)
    :RHIDepthStencilView(context)
{
}
void D3D11DepthStencilView::ClearDepth(float depth)
{
    if (m_pD3D11Dsv)
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11Dsv.Get(), D3D11_CLEAR_DEPTH, depth, 0);
    }
    else
        LOG_ERROR("depth stencil view is null");
}
void D3D11DepthStencilView::ClearStencil(uint32_t stencil)
{
    if (m_pD3D11Dsv)
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11Dsv.Get(), D3D11_CLEAR_STENCIL, 0, (UINT8)stencil);
    }
    else
        LOG_ERROR("depth stencil view is null");
}
void D3D11DepthStencilView::ClearDepthStencil(float depth, uint32_t stencil)
{
    if (m_pD3D11Dsv)
    {
        D3D11Context& rc = static_cast<D3D11Context&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11Dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, (UINT8)stencil);
    }
    else
        LOG_ERROR("depth stencil view is null");
}

D3D11Texture2DDsv::D3D11Texture2DDsv(Context* context, RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
    :D3D11DepthStencilView(context, tex_2d.get(), D3D11CalcSubresource(mip_level, first_array_index, tex_2d->NumMips()), array_size)
{
    // for Texture
    m_iWidth = tex_2d->Width();
    m_iHeight = tex_2d->Height();
    m_ePixelFormat = tex_2d->Format();
    m_iNumSamples = tex_2d->NumSamples();

    m_Param.texture = tex_2d;
    m_Param.first_array_index = first_array_index;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = mip_level;

    this->GetD3DDsv();
}
ID3D11DepthStencilView* D3D11Texture2DDsv::GetD3DDsv()
{
    if (!m_pD3D11Dsv)
    {
        m_pD3D11Dsv = ((D3D11Texture*)m_Param.texture.get())->GetD3DDsv(m_ePixelFormat, m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
    }
    return m_pD3D11Dsv.Get();
}
D3D11TextureCubeFaceDsv::D3D11TextureCubeFaceDsv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
    : D3D11DepthStencilView(context, tex_cube.get(), D3D11CalcSubresource(mip_level, array_index, tex_cube->NumMips()), 1)
{
    // for Texture
    m_iWidth = tex_cube->Width();
    m_iHeight = tex_cube->Height();
    m_ePixelFormat = tex_cube->Format();
    m_iNumSamples = tex_cube->NumSamples();

    m_Param.texture = tex_cube;
    m_Param.first_array_index = array_index;
    m_Param.num_arrays = 1;
    m_Param.mip_level = mip_level;

    m_Param.first_face = face;
    m_Param.num_faces = 1;

    this->GetD3DDsv();
}
ID3D11DepthStencilView* D3D11TextureCubeFaceDsv::GetD3DDsv()
{
    if (!m_pD3D11Dsv)
    {
        m_pD3D11Dsv = ((D3D11Texture*)m_Param.texture.get())->GetD3DDsv(m_ePixelFormat, m_Param.first_array_index, m_Param.first_face, m_Param.mip_level);
    }
    return m_pD3D11Dsv.Get();
}

/******************************************************************************
* D3D11 Uav
*******************************************************************************/
D3D11UnorderedAccessView::D3D11UnorderedAccessView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres)
    :RHIUnorderedAccessView(context)
{
}

D3D11Texture2DCubeUnorderedAccessView::D3D11Texture2DCubeUnorderedAccessView(Context* context, RHITexturePtr const& tex_2d_cube,
    PixelFormat pf, int first_array_index, int array_size, int mip_level)
    :D3D11UnorderedAccessView(context, tex_2d_cube.get(), first_array_index* tex_2d_cube->NumMips() + mip_level, 1)
{
    // for Texture
    m_Param.texture = tex_2d_cube;
    m_Param.pixel_format = pf == PixelFormat::Unknown ? tex_2d_cube->Format() : pf;
    m_Param.first_array_index = first_array_index;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = mip_level;
    
    this->GetD3DUav();
}
ID3D11UnorderedAccessView* D3D11Texture2DCubeUnorderedAccessView::GetD3DUav()
{
    if (!m_pD3DUav && m_Param.texture)
    {
        m_pD3DUav = ((D3D11Texture&)*m_Param.texture).GetD3DUav(m_Param.pixel_format, m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
    }
    return m_pD3DUav.Get();
}


D3D11Texture3DUnorderedAccessView::D3D11Texture3DUnorderedAccessView(Context* context, RHITexturePtr const& tex_3d, PixelFormat pf, int array_index,
    uint32_t first_slice, uint32_t num_slices, int mip_level)
    :D3D11UnorderedAccessView(context, tex_3d.get(), array_index * tex_3d->Depth(mip_level) + mip_level, num_slices* tex_3d->NumMips() + mip_level)
{
    m_Param.texture = tex_3d;
    m_Param.pixel_format = pf == PixelFormat::Unknown ? tex_3d->Format() : pf;

    m_Param.first_array_index = array_index;
    m_Param.num_arrays = 1;
    m_Param.mip_level = mip_level;

    m_Param.first_slice = first_slice;
    m_Param.num_slices = num_slices;
    this->GetD3DUav();
}
ID3D11UnorderedAccessView* D3D11Texture3DUnorderedAccessView::GetD3DUav()
{
    if (!m_pD3DUav && m_Param.texture)
    {
        m_pD3DUav = ((D3D11Texture&)*m_Param.texture).GetD3DUav(m_Param.pixel_format, m_Param.first_array_index, m_Param.first_slice, m_Param.num_slices, m_Param.mip_level);
    }
    return m_pD3DUav.Get();
}


D3D11TextureCubeFaceUnorderedAccessView::D3D11TextureCubeFaceUnorderedAccessView(Context* context, RHITexturePtr const& tex_cube, PixelFormat pf, int array_index, CubeFaceType face, int mip_level)
    : D3D11UnorderedAccessView(context, tex_cube.get(), (array_index * 6 + (uint32_t)face)* tex_cube->NumMips() + mip_level, 1)
{
    m_Param.texture = tex_cube;
    m_Param.pixel_format = pf == PixelFormat::Unknown ? tex_cube->Format() : pf;

    m_Param.first_array_index = array_index;
    m_Param.num_arrays = 1;
    m_Param.mip_level = mip_level;

    m_Param.first_face = face;
    m_Param.num_faces = 1;
    m_Param.mip_level = mip_level;
    this->GetD3DUav();
}
ID3D11UnorderedAccessView* D3D11TextureCubeFaceUnorderedAccessView::GetD3DUav()
{
    if (!m_pD3DUav && m_Param.texture)
    {
        m_pD3DUav = ((D3D11Texture&)*m_Param.texture).GetD3DUav(m_Param.pixel_format, m_Param.first_array_index, 
            m_Param.num_arrays, m_Param.first_face, m_Param.num_faces, m_Param.mip_level);
    }
    return m_pD3DUav.Get();
}

D3D11BufferUnorderedAccessView::D3D11BufferUnorderedAccessView(Context* context, RHIGpuBufferPtr const& gb, PixelFormat pf, uint32_t first_elem, uint32_t num_elems)
    : D3D11UnorderedAccessView(context, gb.get(), 0, 1)
{    
    m_Param.pixel_format = pf;

    m_Param.buffer = gb;
    m_Param.first_elem = first_elem;
    m_Param.num_elem = num_elems;
    this->GetD3DUav();
}
ID3D11UnorderedAccessView* D3D11BufferUnorderedAccessView::GetD3DUav()
{
    if (!m_pD3DUav && m_Param.buffer)
    {
        m_pD3DUav = ((D3D11GpuBuffer&)*m_Param.buffer).GetD3DUav(m_Param.pixel_format, m_Param.first_elem, m_Param.num_elem);
    }
    return m_pD3DUav.Get();
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
