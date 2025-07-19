#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D12 Rtv
*******************************************************************************/
struct D3D12Rtv
{
    ID3D12ResourcePtr d3d_resource;
    uint32_t  offset;
    std::vector<D3D12_RESOURCE_STATES> curr_states;

};
class D3D12RenderTargetView : public RHIRenderTargetView
{
public:
    D3D12RenderTargetView(Context* context, ID3D12Resource* src, uint32_t first_subres, uint32_t num_subres);
    void ClearColor(float4 const& color);

    virtual D3D12Rtv* GetD3DRtv() = 0;

private:
    ID3D12ResourcePtr m_pResource = nullptr;
    uint32_t m_iFirstSubres;
    uint32_t m_iNumSubres;
};
typedef std::shared_ptr<D3D12RenderTargetView> D3D12RenderTargetViewPtr;


class D3D12UnorderedAccessView : public RHIUnorderedAccessView
{
public:
    D3D12UnorderedAccessView(Context* context, ID3D12Resource* res);

private:
    ID3D12Resource* m_pResource = nullptr;
};
SEEK_NAMESPACE_END
