#pragma once

#include "kernel/kernel.h"
#include "rhi/base/rhi_framebuffer.h"

SEEK_NAMESPACE_BEGIN

class D3D12FrameBuffer : public RHIFrameBuffer
{
public:
    D3D12FrameBuffer(Context* context);
    virtual ~D3D12FrameBuffer() override;

    SResult OnBind() override;
    SResult OnUnbind() override;
    SResult Resolve();

    void Clear(uint32_t flags = CBM_ALL, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0), float depth = 1.0, int32_t stencil = 0);
    void ClearRenderTarget(Attachment att, float4 const& clr = float4(0.0, 0.0, 0.0, 0.0));

    ID3D12Resource* GetRenderTargetView() const { return m_vD3dRednerTargets[0]; }

protected:
    std::vector<ID3D12Resource*> m_vD3dRednerTargets;
    ID3D12Resource* m_pD3dDepthStencil = nullptr;
    D3D12_VIEWPORT m_stD3dViewport;
};

SEEK_NAMESPACE_END
