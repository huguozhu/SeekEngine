#include "rhi/d3d12/d3d12_predeclare.h"
#include "rhi/d3d12/d3d12_render_view.h"
#include "rhi/d3d12/d3d12_rhi_context.h"
#include "rhi/d3d12/d3d12_resource.h"
#include "rhi/d3d12/d3d12_texture.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN

UINT D3D12CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT MipLevels)
{
    return MipSlice + ArraySlice * MipLevels;
}

/******************************************************************************
* Render View Handle
*******************************************************************************/
D3D12Rtv::D3D12Rtv(Context* context, D3D12Resource* res, D3D12_RENDER_TARGET_VIEW_DESC const& rtv_desc)
    :m_pContext(context), m_pResource(res)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();
    m_Desc = rc.AllocRtvDescBlock(1);
    pDevice->CreateRenderTargetView(res->GetD3DResource(), &rtv_desc, m_Desc.CpuHandle());
}
D3D12Rtv::~D3D12Rtv()
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    rc.DeallocRtvDescBlock(std::move(m_Desc));
}


D3D12Dsv::D3D12Dsv(Context* context, D3D12Resource* res, D3D12_DEPTH_STENCIL_VIEW_DESC const& dsv_desc)
    :m_pContext(context), m_pResource(res)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();
    m_Desc = rc.AllocDsvDescBlock(1);
    pDevice->CreateDepthStencilView(res->GetD3DResource(), &dsv_desc, m_Desc.CpuHandle());
}
D3D12Dsv::~D3D12Dsv()
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    rc.DeallocDsvDescBlock(std::move(m_Desc));
}

D3D12Srv::D3D12Srv(Context* context, D3D12Resource* res, D3D12_SHADER_RESOURCE_VIEW_DESC const& srv_desc)
    :m_pContext(context), m_pResource(res)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();
    m_Desc = rc.AllocCbvSrvUavDescBlock(1);
    pDevice->CreateShaderResourceView(res->GetD3DResource(), &srv_desc, m_Desc.CpuHandle());
}
D3D12Srv::~D3D12Srv()
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    rc.DeallocCbvSrvUavDescBlock(std::move(m_Desc));
}


D3D12Uav::D3D12Uav(Context* context, D3D12Resource* res, D3D12_UNORDERED_ACCESS_VIEW_DESC const& uav_desc)
    :m_pContext(context), m_pResource(res)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();
    m_Desc = rc.AllocCbvSrvUavDescBlock(1);

    if (D3D12_UAV_DIMENSION_BUFFER == uav_desc.ViewDimension)
    {
        m_iCounterOffset = static_cast<uint32_t>(uav_desc.Buffer.CounterOffsetInBytes);
        if (m_iCounterOffset != 0)
        {
            m_pCounter = m_pResource->GetD3DResource();
        }
    }
    ID3D12Resource* counter = nullptr;
    pDevice->CreateUnorderedAccessView(res->GetD3DResource(), counter, &uav_desc, m_Desc.CpuHandle());
}
D3D12Uav::~D3D12Uav()
{
}

/******************************************************************************
* D3D12 Rtv
*******************************************************************************/
D3D12RenderTargetView::D3D12RenderTargetView(Context* context, D3D12ResourcePtr const& src, uint32_t first_subres, uint32_t num_subres)
    :RHIRenderTargetView(context), m_pRtvResource(src)
{
}
void D3D12RenderTargetView::ClearColor(float4 const& color)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();
    for (uint32_t i = 0; i < m_iNumSubres; i++)
    {
        m_pRtvResource->UpdateResourceBarrier(cmd_list, m_iFirstSubres + i, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    rc.FlushResourceBarriers(cmd_list);
    cmd_list->ClearRenderTargetView(m_pRtvHandle->Handle(), &color.x(), 0, nullptr);
}
D3D12Texture2DCubeRtv::D3D12Texture2DCubeRtv(Context* context, RHITexturePtr const& tex_2d_cube, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
    :D3D12RenderTargetView(context, std::static_pointer_cast<D3D12Texture>(tex_2d_cube), D3D12CalcSubresource(mip_level, first_array_index, tex_2d_cube->NumMips()), array_size)
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
D3D12Rtv* D3D12Texture2DCubeRtv::GetD3DRtv()
{
    if (!m_pRtvHandle && m_pRtvResource.get())
    {
        m_pRtvHandle = ((D3D12Texture*)m_Param.texture.get())->GetD3DRtv(m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
    }
    return m_pRtvHandle.get();
}
D3D12Texture3DRtv::D3D12Texture3DRtv(Context* context, RHITexturePtr const& tex_3d, uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t mip_level)
    :D3D12RenderTargetView(context, std::static_pointer_cast<D3D12Texture>(tex_3d), D3D12CalcSubresource(mip_level, array_index* tex_3d->Depth(mip_level) + first_slice, tex_3d->NumMips()), num_slices* tex_3d->NumMips() + mip_level)

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
D3D12Rtv* D3D12Texture3DRtv::GetD3DRtv()
{
    if (!m_pRtvHandle && m_pRtvResource.get())
    {
        m_pRtvHandle = ((D3D12Texture*)m_Param.texture.get())->GetD3DRtv(m_Param.first_array_index, m_Param.first_slice, m_Param.num_slices, m_Param.mip_level);
    }
    return m_pRtvHandle.get();
}
D3D12TextureCubeFaceRtv::D3D12TextureCubeFaceRtv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
    :D3D12RenderTargetView(context, std::static_pointer_cast<D3D12Texture>(tex_cube), D3D12CalcSubresource(mip_level, array_index * 6 + (uint32_t)face, tex_cube->NumMips()), 1)

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
D3D12Rtv* D3D12TextureCubeFaceRtv::GetD3DRtv()
{
    if (!m_pRtvHandle && m_pRtvResource.get())
    {
        m_pRtvHandle = ((D3D12Texture*)m_Param.texture.get())->GetD3DRtv(m_Param.first_array_index, m_Param.first_face, m_Param.mip_level);
    }
    return m_pRtvHandle.get();
}
/******************************************************************************
* D3D12 Dsv
*******************************************************************************/
D3D12DepthStencilView::D3D12DepthStencilView(Context* context, D3D12ResourcePtr const& res, uint32_t first_subres, uint32_t num_subres)
    :RHIDepthStencilView(context), m_pDsvResource(res)
{
}
void D3D12DepthStencilView::ClearDepth(float depth)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

    for (uint32_t i = 0; i < m_iNumSubres; ++i)
    {
        m_pDsvResource->UpdateResourceBarrier(cmd_list, m_iFirstSubres + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }
    rc.FlushResourceBarriers(cmd_list);
    cmd_list->ClearDepthStencilView(m_pDsvHandle->Handle(), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}
void D3D12DepthStencilView::ClearStencil(uint32_t stencil)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

    for (uint32_t i = 0; i < m_iNumSubres; ++i)
    {
        m_pDsvResource->UpdateResourceBarrier(cmd_list, m_iFirstSubres + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }
    rc.FlushResourceBarriers(cmd_list);
    cmd_list->ClearDepthStencilView(m_pDsvHandle->Handle(), D3D12_CLEAR_FLAG_STENCIL, 1, static_cast<uint8_t>(stencil), 0, nullptr);
}
void D3D12DepthStencilView::ClearDepthStencil(float depth, uint32_t stencil)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

    for (uint32_t i = 0; i < m_iNumSubres; ++i)
    {
        m_pDsvResource->UpdateResourceBarrier(cmd_list, m_iFirstSubres + i, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }
    rc.FlushResourceBarriers(cmd_list);
    cmd_list->ClearDepthStencilView(m_pDsvHandle->Handle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, static_cast<uint8_t>(stencil), 0, nullptr);
}

D3D12Texture2DDsv::D3D12Texture2DDsv(Context* context, RHITexturePtr const& tex_2d, uint32_t first_array_index, uint32_t array_size, uint32_t mip_level)
    :D3D12DepthStencilView(context, std::static_pointer_cast<D3D12Texture>(tex_2d), D3D12CalcSubresource(mip_level, first_array_index, tex_2d->NumMips()), array_size)
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
D3D12Dsv* D3D12Texture2DDsv::GetD3DDsv()
{
    if (!m_pDsvHandle)
    {
        m_pDsvHandle = ((D3D12Texture*)m_Param.texture.get())->GetD3DDsv(m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level);
    }
    return m_pDsvHandle.get();
}

D3D12TextureCubeFaceDsv::D3D12TextureCubeFaceDsv(Context* context, RHITexturePtr const& tex_cube, uint32_t array_index, CubeFaceType face, uint32_t mip_level)
    : D3D12DepthStencilView(context, std::static_pointer_cast<D3D12Texture>(tex_cube), D3D12CalcSubresource(mip_level, array_index, tex_cube->NumMips()), 1)
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
D3D12Dsv* D3D12TextureCubeFaceDsv::GetD3DDsv()
{
    if (!m_pDsvHandle)
    {
        m_pDsvHandle = ((D3D12Texture*)m_Param.texture.get())->GetD3DDsv(m_Param.first_array_index, m_Param.first_face, m_Param.mip_level);
    }
    return m_pDsvHandle.get();
}


/******************************************************************************
* D3D12 Srv
*******************************************************************************/
D3D12TextureSrv::D3D12TextureSrv(Context* context, RHITexturePtr const& texture, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
    :D3D12ShaderResourceView(context)
{
    m_Param.pixel_format = texture->Format();
    // for Texture
    m_Param.texture = texture; 

    m_Param.first_array_index = first_array_index;
    m_Param.num_arrays = array_size;
    m_Param.mip_level = first_level;
    m_iNumMipLevels = num_levels;

    m_pSrvSrc = texture.get();
}
D3D12Srv* D3D12TextureSrv::GetD3DSrv()
{
    if (!m_pSrvHandle && m_Param.texture)
    {
        m_pSrvHandle = ((D3D12Texture*)m_Param.texture.get())->GetD3DSrv(m_Param.first_array_index, m_Param.num_arrays, m_Param.mip_level, m_iNumMipLevels);
    }
    return m_pSrvHandle.get();
}


/******************************************************************************
* D3D12 Uav
*******************************************************************************/
D3D12UnorderedAccessView::D3D12UnorderedAccessView(Context* context, D3D12ResourcePtr const& src, uint32_t first_subres, uint32_t num_subres)
    :RHIUnorderedAccessView(context), m_pUavSrc(src), m_iFirstSubres(first_subres), m_iNumSubres(num_subres)
{
}
void D3D12UnorderedAccessView::Clear(float4 const& v)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

    m_pUavSrc->UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    rc.FlushResourceBarriers(cmd_list);

    auto cbv_srv_uav_desc_block = rc.AllocDynamicCbvSrvUavDescBlock(1);
    D3D12_CPU_DESCRIPTOR_HANDLE const cpu_handle = cbv_srv_uav_desc_block.CpuHandle();
    D3D12_GPU_DESCRIPTOR_HANDLE const gpu_handle = cbv_srv_uav_desc_block.GpuHandle();
    d3d_device_->CopyDescriptorsSimple(1, cpu_handle, m_pUavHandle->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    cmd_list->ClearUnorderedAccessViewFloat(gpu_handle, m_pUavHandle->Handle(),
        m_pUavSrc->GetD3DResource(), &v.x(), 0, nullptr);

    rc.DeallocDynamicCbvSrvUavDescBlock(std::move(cbv_srv_uav_desc_block));
}
void D3D12UnorderedAccessView::Clear(uint4 const& v)
{
    D3D12Context& rc = static_cast<D3D12Context&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.D3DRenderCmdList();

    m_pUavSrc->UpdateResourceBarrier(cmd_list, 0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    rc.FlushResourceBarriers(cmd_list);

    auto cbv_srv_uav_desc_block = rc.AllocDynamicCbvSrvUavDescBlock(1);
    D3D12_CPU_DESCRIPTOR_HANDLE const cpu_handle = cbv_srv_uav_desc_block.CpuHandle();
    D3D12_GPU_DESCRIPTOR_HANDLE const gpu_handle = cbv_srv_uav_desc_block.GpuHandle();
    d3d_device_->CopyDescriptorsSimple(1, cpu_handle, m_pUavHandle->Handle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    cmd_list->ClearUnorderedAccessViewUint(gpu_handle, m_pUavHandle->Handle(),
        m_pUavSrc->GetD3DResource(), &v.x(), 0, nullptr);

    rc.DeallocDynamicCbvSrvUavDescBlock(std::move(cbv_srv_uav_desc_block));
}


SEEK_NAMESPACE_END
