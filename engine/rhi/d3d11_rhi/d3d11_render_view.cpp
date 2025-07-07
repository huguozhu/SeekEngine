#include "rhi/d3d11_rhi/d3d11_render_view.h"
#include "rhi/d3d11_rhi/d3d11_texture.h"
#include "rhi/d3d11_rhi/d3d11_rhi_context.h"

#include "rhi/base/rhi_texture.h"

#include "kernel/context.h"

#define SEEK_MACRO_FILE_UID 4     // this code is auto generated, don't touch it!!!

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D11 Rtv
*******************************************************************************/
D3D11Rtv::D3D11Rtv(Context* context, void* src, uint32_t first_subres, uint32_t num_subres)
    : RHIRenderTargetView(context), m_pSrc(src), m_iFirstSubres(first_subres), m_iNumSubres(num_subres)
{

}
void D3D11Rtv::ClearColor(float4 const& color)
{
    if (m_pD3DRtv)
    {
        D3D11RHIContext& rc = static_cast<D3D11RHIContext&>(m_pContext->RHIContextInstance());
        rc.GetD3D11DeviceContext()->ClearRenderTargetView(m_pD3DRtv.Get(), &color[0]);
    }
    else
        LOG_ERROR("render target view is null");
}

D3D11Texture2DCubeRtv::D3D11Texture2DCubeRtv(Context* context, RHITexturePtr const& texture_2d_cube, int first_array_index, int array_size, int mip_level)
    : D3D11Rtv(context, texture_2d_cube.get(), D3D11CalcSubresource(mip_level, first_array_index, texture_2d_cube->NumMips()), 1)
{
    // for Texture
    m_iWidth = texture_2d_cube->Width();
    m_iHeight = texture_2d_cube->Height();
    m_ePixelFormat = texture_2d_cube->Format();
    m_iNumSamples = texture_2d_cube->NumSamples();

    m_Param.texture = texture_2d_cube;
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

D3D11Texture3DRtv::D3D11Texture3DRtv(Context* context, RHITexturePtr const& texture_3d, int array_index, uint32_t first_slice, uint32_t num_slices, int mip_level)
    :D3D11Rtv(context, texture_3d.get(), D3D11CalcSubresource(mip_level, array_index * texture_3d->Depth(mip_level) + first_slice, texture_3d->NumMips()), num_slices* texture_3d->NumMips() + mip_level)
{
    m_iWidth = texture_3d->Width(mip_level);
    m_iHeight = texture_3d->Height(mip_level);
    m_iNumSamples = texture_3d->NumSamples();

    m_Param.texture = texture_3d;
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
D3D11TextureCubeFaceRtv::D3D11TextureCubeFaceRtv(Context* context, RHITexturePtr const& texture_cube, int array_index, CubeFaceType face, int mip_level)
    :D3D11Rtv(context, texture_cube.get(), D3D11CalcSubresource(mip_level, array_index * 6 + (uint32_t)face, texture_cube->NumMips()), 1)
{
    m_iWidth = texture_cube->Width(mip_level);
    m_iHeight = texture_cube->Height(mip_level);
    m_iNumSamples = texture_cube->NumSamples();

    m_Param.texture = texture_cube;
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
