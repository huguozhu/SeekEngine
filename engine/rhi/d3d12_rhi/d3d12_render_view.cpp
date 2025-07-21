#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_render_view.h"
#include "rhi/d3d12_rhi/d3d12_rhi_context.h"
#include "rhi/d3d12_rhi/d3d12_resource.h"
#include "kernel/context.h"

SEEK_NAMESPACE_BEGIN
/******************************************************************************
* D3D12 Rtv
*******************************************************************************/
D3D12Rtv::D3D12Rtv(Context* context, D3D12Resource* res, D3D12_RENDER_TARGET_VIEW_DESC const& rtv_desc)
    :m_pContext(context), m_pD3dResource(res)
{
    D3D12RHIContext& rc = static_cast<D3D12RHIContext&>(m_pContext->RHIContextInstance());
    ID3D12Device* pDevice = rc.GetD3D12Device();
    m_Desc = rc.AllocRtvDescBlock(1);
    pDevice->CreateRenderTargetView(res->GetD3DResource(), &rtv_desc, m_Desc.GetCpuHandle());

}
D3D12Rtv::~D3D12Rtv()
{
}

D3D12RenderTargetView::D3D12RenderTargetView(Context* context, D3D12ResourcePtr const& src, uint32_t first_subres, uint32_t num_subres)
    :RHIRenderTargetView(context), m_pRtvResource(src)
{

}

void D3D12RenderTargetView::ClearColor(float4 const& color)
{
    D3D12RHIContext& rc = static_cast<D3D12RHIContext&>(m_pContext->RHIContextInstance());
    ID3D12GraphicsCommandList* cmd_list = rc.GetD3D12CommandList();
    for (uint32_t i = 0; i < m_iNumSubres; i++)
    {
        m_pRtvResource->UpdateResourceBarrier(cmd_list, m_iFirstSubres + i, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    rc.FlushResourceBarriers(cmd_list);
    //cmd_list->ClearRenderTargetView(m_pRtvHandle->)

}

/******************************************************************************
* D3D12 Dsv
*******************************************************************************/


SEEK_NAMESPACE_END
