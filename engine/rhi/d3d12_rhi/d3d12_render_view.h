#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_render_view.h"
#include "rhi/d3d12_rhi/d3d12_resource.h"
#include "rhi/d3d12_rhi/d3d12_predeclare.h"
#include "rhi/d3d12_rhi/d3d12_gpu_descriptor_allocator.h"

SEEK_NAMESPACE_BEGIN

/******************************************************************************
* D3D12 Rtv
*******************************************************************************/
class D3D12Rtv
{
public:
    D3D12Rtv(Context* context, D3D12Resource* res, D3D12_RENDER_TARGET_VIEW_DESC const& rtv_desc);
    ~D3D12Rtv();
    D3D12_CPU_DESCRIPTOR_HANDLE Handle() const { return m_Desc.GetCpuHandle(); }

private:
    Context* m_pContext = nullptr;
    D3D12Resource* m_pD3dResource = nullptr;
    D3D12GpuDescriptorBlock m_Desc;
};
using D3D12RtvPtr = std::shared_ptr<D3D12Rtv>;


class D3D12RenderTargetView : public RHIRenderTargetView
{
public:
    D3D12RenderTargetView(Context* context, D3D12ResourcePtr const& src, uint32_t first_subres, uint32_t num_subres);
    void ClearColor(float4 const& color);

    virtual D3D12Rtv* GetD3DRtv() = 0;

private:
    D3D12RtvPtr m_pRtvHandle = nullptr;
    D3D12ResourcePtr m_pRtvResource = nullptr;
    uint32_t m_iFirstSubres;
    uint32_t m_iNumSubres;
};
using D3D12RenderTargetViewPtr = std::shared_ptr<D3D12RenderTargetView>;


class D3D12UnorderedAccessView : public RHIUnorderedAccessView
{
public:
    D3D12UnorderedAccessView(Context* context, ID3D12Resource* res);

private:
    ID3D12Resource* m_pResource = nullptr;
};
using D3D12UnorderedAccessViewPtr = std::shared_ptr<D3D12UnorderedAccessView>;
SEEK_NAMESPACE_END
