#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_framebuffer.h"
#include "rhi/d3d12/d3d12_resource.h"

SEEK_NAMESPACE_BEGIN

class D3D12FrameBuffer : public RHIFrameBuffer
{
public:
    D3D12FrameBuffer(Context* context);

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult Resolve();

    void Clear(uint32_t flags = CBM_ALL, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0), float depth = 1.0, int32_t stencil = 0);
    void ClearRenderTarget(Attachment att, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0));

    void BindBarrier(ID3D12GraphicsCommandList* cmd_list);
    void SetRenderTargets(ID3D12GraphicsCommandList* cmd_list);

private:
    void UpdateAllViews();

protected: 
    /*Buffer-first_subres-num_subres*/
    std::vector<std::tuple<D3D12Resource*, uint32_t, uint32_t> > m_vD3dRtvResources; 
    std::tuple<D3D12Resource*, uint32_t, uint32_t> m_DsvResource;

    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_vD3dRtvCpuHandles;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3dSdvHandle;
    D3D12_VIEWPORT m_stD3dViewport;
};

SEEK_NAMESPACE_END
