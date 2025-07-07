#include "rhi/d3d11_rhi/d3d11_render_view.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"

#include "rhi/base/rhi_texture.h"

#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 4     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11RenderTargetView
*******************************************************************************/
D3D11RenderTargetView::D3D11RenderTargetView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres)
    : RHIRenderTargetView(context), m_pSrc(src), m_iFirstSubres(first_subres), m_iNumSubres(num_subres)
{

}
void D3D11RenderTargetView::ClearColor(float4 const& color)
{
    if (m_pD3DRtv)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearRenderTargetView(m_pD3DRtv.Get(), &color[0]);
    }
    else
        LOG_ERROR("render target view is null");
}

D3D11Texture2DCubeRtv::D3D11Texture2DCubeRtv(Context* context, RHITexturePtr const& tex_2d_cube, int first_array_index, int array_size, int mip_level)
    : D3D11RenderTargetView(context, tex_2d_cube.get(), D3D11CalcSubresource(mip_level, first_array_index, tex_2d_cube->NumMips()), 1)
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
        m_pD3DRtv = ((D3D11Texture*)m_Param.texture.get())->GetD3DRtv(m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
    }
    return m_pD3DRtv.Get();
}

D3D11Texture3DRtv::D3D11Texture3DRtv(Context* context, RHITexturePtr const& tex_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int mip_level)
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
        m_pD3DRtv = ((D3D11Texture*)m_Param.texture.get())->GetD3DRtv(m_Param.first_array_index, m_Param.first_slice, m_Param.num_slices, m_Param.mip_level);
    }
    return m_pD3DRtv.Get();
}
D3D11TextureCubeFaceRtv::D3D11TextureCubeFaceRtv(Context* context, RHITexturePtr const& tex_cube, int array_index, CubeFaceType face, int mip_level)
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
        m_pD3DRtv = ((D3D11Texture*)m_Param.texture.get())->GetD3DRtv(m_Param.first_array_index, m_Param.first_face, m_Param.mip_level);
    }
    return m_pD3DRtv.Get();
}

/******************************************************************************
* D3D11Dsv
*******************************************************************************/
D3D11DepthStencilView::D3D11DepthStencilView(Context* context, void* src, uint32_t first_subres, uint32_t num_subres)
    :RHIDepthStencilView(context)
{
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
    if (m_pD3D11Dsv)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11Dsv.Get(), D3D11_CLEAR_DEPTH, depth, 0);
    }
    else
        LOG_ERROR("depth stencil view is null");
}
void D3D11DepthStencilView::ClearStencil(uint32_t stencil)
{
    if (m_pD3D11Dsv)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11Dsv.Get(), D3D11_CLEAR_STENCIL, 0, (UINT8)stencil);
    }
    else
        LOG_ERROR("depth stencil view is null");
}
void D3D11DepthStencilView::ClearDepthStencil(float depth, uint32_t stencil)
{
    if (m_pD3D11Dsv)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearDepthStencilView(m_pD3D11Dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, (UINT8)stencil);
    }
    else
        LOG_ERROR("depth stencil view is null");
}

D3D11Texture2DDsv::D3D11Texture2DDsv(Context* context, RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
    :D3D11DepthStencilView(context, tex_2d.get(), D3D11CalcSubresource(mip_level, first_array_index, tex_2d->NumMips()), 1)
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
        m_pD3D11Dsv = ((D3D11Texture*)m_Param.texture.get())->GetD3DDsv(m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
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
        m_pD3D11Dsv = ((D3D11Texture*)m_Param.texture.get())->GetD3DDsv(m_Param.first_array_index, m_Param.first_face, m_Param.mip_level);
    }
    return m_pD3D11Dsv.Get();
}
SEEK_NAMESPACE_END

#undef SEEK_MACRO_FILE_UID     // this code is auto generated, don't touch it!!!
